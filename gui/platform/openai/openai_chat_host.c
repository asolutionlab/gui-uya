#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern int kill(pid_t pid, int sig);

enum {
    UYA_OPENAI_CHAT_READY = 0,
    UYA_OPENAI_CHAT_PENDING = 1,
    UYA_OPENAI_CHAT_INVALID_ARG = -1,
    UYA_OPENAI_CHAT_DISABLED = -2,
    UYA_OPENAI_CHAT_BUSY = -3,
    UYA_OPENAI_CHAT_INVALID_HANDLE = -4,
    UYA_OPENAI_CHAT_CANCELLED = -5,
    UYA_OPENAI_CHAT_TRANSPORT_ERROR = -6,
    UYA_OPENAI_CHAT_HTTP_ERROR = -7,
    UYA_OPENAI_CHAT_RESPONSE_TOO_LARGE = -8,
    UYA_OPENAI_CHAT_PARSE_ERROR = -9,
    UYA_OPENAI_CHAT_NO_CONTENT = -10,
};

#define UYA_OPENAI_CHAT_MAX_HTTP_BODY (64u * 1024u)

typedef struct UyaOpenAiResolvedConfig {
    int enabled;
    char api_key[256];
    char model[128];
    char base_url[256];
    char api_path[128];
    char config_source[64];
} UyaOpenAiResolvedConfig;

typedef struct UyaOpenAiChatRequest {
    int in_use;
    int handle;
    int done;
    int cancel_requested;
    int result_code;
    pid_t child_pid;
    int pipe_fd;
    char *request_body;
    size_t request_len;
    char *assistant_content;
    size_t assistant_len;
    char *http_body;
    size_t http_len;
    long http_status;
    char base_url[256];
    char api_path[128];
    char api_key[256];
} UyaOpenAiChatRequest;

typedef struct UyaOpenAiChatReplyHeader {
    int32_t result_code;
    int32_t http_status;
    int32_t assistant_len;
} UyaOpenAiChatReplyHeader;

static UyaOpenAiChatRequest g_openai_req = {0};
static pthread_mutex_t g_openai_mutex = PTHREAD_MUTEX_INITIALIZER;

static int uya_openai_chat_env_truthy(const char *value);

static int uya_openai_chat_debug_enabled(void) {
    const char *value = getenv("UYA_OPENAI_DEBUG");
    return uya_openai_chat_env_truthy(value);
}

static void uya_openai_chat_debugf(const char *fmt, ...) {
    va_list ap;
    if (!uya_openai_chat_debug_enabled()) {
        return;
    }
    (void)fprintf(stderr, "[uya-openai] ");
    va_start(ap, fmt);
    (void)vfprintf(stderr, fmt, ap);
    va_end(ap);
    (void)fputc('\n', stderr);
    (void)fflush(stderr);
}

static void uya_openai_chat_preview(const char *src, char *dst, size_t dst_size) {
    size_t si = 0u;
    size_t di = 0u;
    if (dst == NULL || dst_size == 0u) {
        return;
    }
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    while (src[si] != '\0' && di + 1u < dst_size) {
        unsigned char ch = (unsigned char)src[si++];
        if (ch == '\n' || ch == '\r' || ch == '\t') {
            ch = ' ';
        }
        if (ch < 32u || ch > 126u) {
            ch = '?';
        }
        dst[di++] = (char)ch;
    }
    if (src[si] != '\0' && di + 4u < dst_size) {
        dst[di++] = '.';
        dst[di++] = '.';
        dst[di++] = '.';
    }
    dst[di] = '\0';
}

static const char *uya_openai_chat_status_name(int code) {
    switch (code) {
        case UYA_OPENAI_CHAT_READY: return "ready";
        case UYA_OPENAI_CHAT_PENDING: return "pending";
        case UYA_OPENAI_CHAT_INVALID_ARG: return "invalid_arg";
        case UYA_OPENAI_CHAT_DISABLED: return "disabled";
        case UYA_OPENAI_CHAT_BUSY: return "busy";
        case UYA_OPENAI_CHAT_INVALID_HANDLE: return "invalid_handle";
        case UYA_OPENAI_CHAT_CANCELLED: return "cancelled";
        case UYA_OPENAI_CHAT_TRANSPORT_ERROR: return "transport_error";
        case UYA_OPENAI_CHAT_HTTP_ERROR: return "http_error";
        case UYA_OPENAI_CHAT_RESPONSE_TOO_LARGE: return "response_too_large";
        case UYA_OPENAI_CHAT_PARSE_ERROR: return "parse_error";
        case UYA_OPENAI_CHAT_NO_CONTENT: return "no_content";
        default: return "unknown";
    }
}

static int uya_openai_chat_env_truthy(const char *value) {
    if (value == NULL || value[0] == '\0') {
        return 0;
    }
    if (strcmp(value, "0") == 0) {
        return 0;
    }
    if (strcmp(value, "false") == 0 || strcmp(value, "FALSE") == 0) {
        return 0;
    }
    if (strcmp(value, "off") == 0 || strcmp(value, "OFF") == 0) {
        return 0;
    }
    return 1;
}

static void uya_openai_chat_trim_in_place(char *text) {
    size_t len;
    size_t start = 0u;
    if (text == NULL) {
        return;
    }
    len = strlen(text);
    while (text[start] == ' ' || text[start] == '\t' || text[start] == '\r' || text[start] == '\n') {
        ++start;
    }
    if (start > 0u) {
        memmove(text, text + start, len - start + 1u);
        len -= start;
    }
    while (len > 0u) {
        char ch = text[len - 1u];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') {
            break;
        }
        text[len - 1u] = '\0';
        --len;
    }
}

static void uya_openai_chat_unquote_in_place(char *text) {
    size_t len;
    if (text == NULL) {
        return;
    }
    len = strlen(text);
    if (len >= 2u && ((text[0] == '"' && text[len - 1u] == '"') || (text[0] == '\'' && text[len - 1u] == '\''))) {
        memmove(text, text + 1, len - 2u);
        text[len - 2u] = '\0';
    }
}

static void uya_openai_chat_copy_value(char *dst, size_t dst_size, const char *value) {
    if (dst == NULL || dst_size == 0u) {
        return;
    }
    if (value == NULL) {
        dst[0] = '\0';
        return;
    }
    (void)snprintf(dst, dst_size, "%s", value);
}

static void uya_openai_chat_join_url(char *dst, size_t dst_size, const char *base_url, const char *path) {
    size_t base_len;
    size_t path_offset = 0u;
    if (dst == NULL || dst_size == 0u) {
        return;
    }
    dst[0] = '\0';
    if (base_url == NULL || base_url[0] == '\0') {
        return;
    }
    if (path == NULL || path[0] == '\0') {
        uya_openai_chat_copy_value(dst, dst_size, base_url);
        return;
    }
    base_len = strlen(base_url);
    if (base_len > 0u && base_url[base_len - 1u] == '/' && path[0] == '/') {
        path_offset = 1u;
    } else if ((base_len == 0u || base_url[base_len - 1u] != '/') && path[0] != '/') {
        (void)snprintf(dst, dst_size, "%s/%s", base_url, path);
        return;
    }
    (void)snprintf(dst, dst_size, "%s%s", base_url, path + path_offset);
}

static void uya_openai_chat_apply_kv(UyaOpenAiResolvedConfig *cfg, const char *key, const char *value) {
    if (cfg == NULL || key == NULL || value == NULL) {
        return;
    }
    if (strcmp(key, "OPENAI_API_KEY") == 0) {
        uya_openai_chat_copy_value(cfg->api_key, sizeof(cfg->api_key), value);
        return;
    }
    if (strcmp(key, "OPENAI_MODEL") == 0) {
        uya_openai_chat_copy_value(cfg->model, sizeof(cfg->model), value);
        return;
    }
    if (strcmp(key, "OPENAI_BASE_URL") == 0) {
        uya_openai_chat_copy_value(cfg->base_url, sizeof(cfg->base_url), value);
        return;
    }
    if (strcmp(key, "OPENAI_API_PATH") == 0) {
        uya_openai_chat_copy_value(cfg->api_path, sizeof(cfg->api_path), value);
        return;
    }
    if (strcmp(key, "UYA_DDZ_USE_OPENAI") == 0) {
        cfg->enabled = uya_openai_chat_env_truthy(value);
    }
}

static int uya_openai_chat_try_load_config_file(const char *path, UyaOpenAiResolvedConfig *cfg) {
    FILE *fp;
    char line[768];
    if (path == NULL || path[0] == '\0' || cfg == NULL) {
        return 0;
    }
    fp = fopen(path, "rb");
    if (fp == NULL) {
        return 0;
    }
    while (fgets(line, (int)sizeof(line), fp) != NULL) {
        char *eq;
        char *key = line;
        char *value;
        uya_openai_chat_trim_in_place(key);
        if (key[0] == '\0' || key[0] == '#') {
            continue;
        }
        eq = strchr(key, '=');
        if (eq == NULL) {
            continue;
        }
        *eq = '\0';
        value = eq + 1;
        uya_openai_chat_trim_in_place(key);
        uya_openai_chat_trim_in_place(value);
        uya_openai_chat_unquote_in_place(value);
        uya_openai_chat_apply_kv(cfg, key, value);
    }
    (void)fclose(fp);
    uya_openai_chat_copy_value(cfg->config_source, sizeof(cfg->config_source), path);
    return 1;
}

static int uya_openai_chat_try_load_config_upwards(const char *filename, UyaOpenAiResolvedConfig *cfg) {
    char cwd[4096];
    char path[4608];
    size_t dir_len;
    if (filename == NULL || filename[0] == '\0' || cfg == NULL) {
        return 0;
    }
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return 0;
    }
    for (;;) {
        dir_len = strlen(cwd);
        if (dir_len == 0u) {
            return 0;
        }
        if (strcmp(cwd, "/") == 0) {
            (void)snprintf(path, sizeof(path), "/%s", filename);
        } else {
            (void)snprintf(path, sizeof(path), "%s/%s", cwd, filename);
        }
        if (uya_openai_chat_try_load_config_file(path, cfg)) {
            return 1;
        }
        while (dir_len > 0u && cwd[dir_len - 1u] != '/') {
            --dir_len;
        }
        if (dir_len == 0u) {
            return 0;
        }
        if (dir_len == 1u) {
            cwd[1] = '\0';
        } else {
            cwd[dir_len - 1u] = '\0';
        }
    }
}

static void uya_openai_chat_load_config_file(UyaOpenAiResolvedConfig *cfg) {
    const char *explicit_path;
    if (cfg == NULL) {
        return;
    }
    explicit_path = getenv("UYA_OPENAI_CONFIG_FILE");
    if (explicit_path != NULL && explicit_path[0] != '\0') {
        (void)uya_openai_chat_try_load_config_file(explicit_path, cfg);
        return;
    }
    if (uya_openai_chat_try_load_config_upwards(".uya_openai.env", cfg)) {
        return;
    }
    (void)uya_openai_chat_try_load_config_upwards("openai.env", cfg);
}

static UyaOpenAiResolvedConfig uya_openai_chat_resolve_config(void) {
    UyaOpenAiResolvedConfig cfg;
    const char *env_value;
    memset(&cfg, 0, sizeof(cfg));
    cfg.enabled = 0;
    uya_openai_chat_copy_value(cfg.model, sizeof(cfg.model), "gpt-5.4-mini");
    uya_openai_chat_copy_value(cfg.base_url, sizeof(cfg.base_url), "https://api.openai.com/v1");
    uya_openai_chat_copy_value(cfg.api_path, sizeof(cfg.api_path), "/ddz/decision");
    uya_openai_chat_copy_value(cfg.config_source, sizeof(cfg.config_source), "defaults");

    uya_openai_chat_load_config_file(&cfg);

    env_value = getenv("UYA_DDZ_USE_OPENAI");
    if (env_value != NULL && env_value[0] != '\0') {
        cfg.enabled = uya_openai_chat_env_truthy(env_value);
        uya_openai_chat_copy_value(cfg.config_source, sizeof(cfg.config_source), "env");
    }
    env_value = getenv("OPENAI_API_KEY");
    if (env_value != NULL && env_value[0] != '\0') {
        uya_openai_chat_copy_value(cfg.api_key, sizeof(cfg.api_key), env_value);
        uya_openai_chat_copy_value(cfg.config_source, sizeof(cfg.config_source), "env");
    }
    env_value = getenv("OPENAI_MODEL");
    if (env_value != NULL && env_value[0] != '\0') {
        uya_openai_chat_copy_value(cfg.model, sizeof(cfg.model), env_value);
        uya_openai_chat_copy_value(cfg.config_source, sizeof(cfg.config_source), "env");
    }
    env_value = getenv("OPENAI_BASE_URL");
    if (env_value != NULL && env_value[0] != '\0') {
        uya_openai_chat_copy_value(cfg.base_url, sizeof(cfg.base_url), env_value);
        uya_openai_chat_copy_value(cfg.config_source, sizeof(cfg.config_source), "env");
    }
    env_value = getenv("OPENAI_API_PATH");
    if (env_value != NULL && env_value[0] != '\0') {
        uya_openai_chat_copy_value(cfg.api_path, sizeof(cfg.api_path), env_value);
        uya_openai_chat_copy_value(cfg.config_source, sizeof(cfg.config_source), "env");
    }
    return cfg;
}

static int uya_openai_chat_enabled(void) {
    UyaOpenAiResolvedConfig cfg = uya_openai_chat_resolve_config();
    if (!cfg.enabled) {
        return 0;
    }
    if (cfg.api_key[0] == '\0') {
        return 0;
    }
    return 1;
}

static void uya_openai_chat_free_request(UyaOpenAiChatRequest *req) {
    if (req == NULL) {
        return;
    }
    free(req->request_body);
    req->request_body = NULL;
    free(req->assistant_content);
    req->assistant_content = NULL;
    free(req->http_body);
    req->http_body = NULL;
    req->request_len = 0u;
    req->assistant_len = 0u;
    req->http_len = 0u;
    req->http_status = 0L;
    req->cancel_requested = 0;
    req->done = 0;
    req->child_pid = 0;
    if (req->pipe_fd >= 0) {
        (void)close(req->pipe_fd);
        req->pipe_fd = -1;
    }
    req->result_code = UYA_OPENAI_CHAT_INVALID_HANDLE;
    req->base_url[0] = '\0';
    req->api_path[0] = '\0';
    req->api_key[0] = '\0';
}

static void uya_openai_chat_reset_slot(UyaOpenAiChatRequest *req) {
    if (req == NULL) {
        return;
    }
    uya_openai_chat_free_request(req);
    req->in_use = 0;
    req->handle = 0;
}

static int uya_openai_chat_write_all(int fd, const void *buf, size_t len) {
    const char *p = (const char *)buf;
    while (len > 0u) {
        ssize_t written = write(fd, p, len);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }
        p += (size_t)written;
        len -= (size_t)written;
    }
    return 1;
}

static int uya_openai_chat_read_all(int fd, char **out_buf, size_t *out_len) {
    size_t cap = 512u;
    size_t len = 0u;
    char *buf;
    if (out_buf == NULL || out_len == NULL) {
        return 0;
    }
    *out_buf = NULL;
    *out_len = 0u;
    buf = (char *)malloc(cap);
    if (buf == NULL) {
        return 0;
    }
    for (;;) {
        ssize_t n = read(fd, buf + len, cap - len);
        if (n == 0) {
            break;
        }
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            free(buf);
            return 0;
        }
        len += (size_t)n;
        if (len == cap) {
            char *next;
            if (cap >= UYA_OPENAI_CHAT_MAX_HTTP_BODY) {
                free(buf);
                return 0;
            }
            cap *= 2u;
            if (cap > UYA_OPENAI_CHAT_MAX_HTTP_BODY) {
                cap = UYA_OPENAI_CHAT_MAX_HTTP_BODY;
            }
            next = (char *)realloc(buf, cap);
            if (next == NULL) {
                free(buf);
                return 0;
            }
            buf = next;
        }
    }
    *out_buf = buf;
    *out_len = len;
    return 1;
}

static size_t uya_openai_chat_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    UyaOpenAiChatRequest *req = (UyaOpenAiChatRequest *)userdata;
    size_t chunk = size * nmemb;
    size_t needed;
    char *next;
    if (req == NULL || ptr == NULL || chunk == 0u) {
        return 0u;
    }
    if (req->http_len + chunk + 1u > UYA_OPENAI_CHAT_MAX_HTTP_BODY) {
        req->result_code = UYA_OPENAI_CHAT_RESPONSE_TOO_LARGE;
        return 0u;
    }
    needed = req->http_len + chunk + 1u;
    next = (char *)realloc(req->http_body, needed);
    if (next == NULL) {
        req->result_code = UYA_OPENAI_CHAT_TRANSPORT_ERROR;
        return 0u;
    }
    req->http_body = next;
    (void)memcpy(req->http_body + req->http_len, ptr, chunk);
    req->http_len += chunk;
    req->http_body[req->http_len] = '\0';
    return chunk;
}

static int uya_openai_chat_xferinfo_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    UyaOpenAiChatRequest *req = (UyaOpenAiChatRequest *)clientp;
    (void)dltotal;
    (void)dlnow;
    (void)ultotal;
    (void)ulnow;
    if (req != NULL && req->cancel_requested) {
        return 1;
    }
    return 0;
}

static const char *uya_openai_chat_find_key(const char *text, const char *key) {
    size_t key_len;
    const char *p;
    if (text == NULL || key == NULL) {
        return NULL;
    }
    key_len = strlen(key);
    p = text;
    while ((p = strstr(p, key)) != NULL) {
        if (p == text || p[-1] != '\\') {
            return p + key_len;
        }
        p += key_len;
    }
    return NULL;
}

static const char *uya_openai_chat_skip_ws(const char *p) {
    while (p != NULL && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')) {
        ++p;
    }
    return p;
}

static int uya_openai_chat_json_decode_string(const char *src, char **out_text, size_t *out_len) {
    size_t cap = 64u;
    size_t len = 0u;
    char *buf;
    if (src == NULL || out_text == NULL || out_len == NULL || *src != '"') {
        return 0;
    }
    buf = (char *)malloc(cap);
    if (buf == NULL) {
        return 0;
    }
    ++src;
    while (*src != '\0' && *src != '"') {
        unsigned ch = (unsigned char)*src;
        if (ch == '\\') {
            ++src;
            if (*src == '\0') {
                free(buf);
                return 0;
            }
            switch (*src) {
                case '"': ch = '"'; break;
                case '\\': ch = '\\'; break;
                case '/': ch = '/'; break;
                case 'b': ch = '\b'; break;
                case 'f': ch = '\f'; break;
                case 'n': ch = '\n'; break;
                case 'r': ch = '\r'; break;
                case 't': ch = '\t'; break;
                case 'u':
                    ch = '?';
                    src += 4;
                    break;
                default:
                    free(buf);
                    return 0;
            }
        }
        if (len + 2u > cap) {
            cap *= 2u;
            {
                char *next = (char *)realloc(buf, cap);
                if (next == NULL) {
                    free(buf);
                    return 0;
                }
                buf = next;
            }
        }
        buf[len++] = (char)ch;
        ++src;
    }
    if (*src != '"') {
        free(buf);
        return 0;
    }
    buf[len] = '\0';
    *out_text = buf;
    *out_len = len;
    return 1;
}

static int uya_openai_chat_set_assistant_copy(UyaOpenAiChatRequest *req, const char *src, size_t len) {
    char *copy;
    if (req == NULL || src == NULL) {
        return UYA_OPENAI_CHAT_PARSE_ERROR;
    }
    copy = (char *)malloc(len + 1u);
    if (copy == NULL) {
        return UYA_OPENAI_CHAT_TRANSPORT_ERROR;
    }
    if (len > 0u) {
        memcpy(copy, src, len);
    }
    copy[len] = '\0';
    req->assistant_content = copy;
    req->assistant_len = len;
    return UYA_OPENAI_CHAT_READY;
}

static int uya_openai_chat_extract_content(UyaOpenAiChatRequest *req) {
    const char *p;
    const char *message_key;
    const char *content_key;
    char *decoded = NULL;
    size_t decoded_len = 0u;
    if (req == NULL || req->http_body == NULL) {
        return UYA_OPENAI_CHAT_PARSE_ERROR;
    }
    p = uya_openai_chat_find_key(req->http_body, "\"action_id\"");
    if (p != NULL) {
        return uya_openai_chat_set_assistant_copy(req, req->http_body, req->http_len);
    }
    p = uya_openai_chat_find_key(req->http_body, "\"refusal\"");
    if (p != NULL) {
        p = uya_openai_chat_skip_ws(p);
        if (p != NULL && *p == ':') {
            p = uya_openai_chat_skip_ws(p + 1);
            if (p != NULL && strncmp(p, "null", 4u) != 0) {
                return UYA_OPENAI_CHAT_NO_CONTENT;
            }
        }
    }
    message_key = uya_openai_chat_find_key(req->http_body, "\"message\"");
    if (message_key == NULL) {
        return UYA_OPENAI_CHAT_PARSE_ERROR;
    }
    content_key = uya_openai_chat_find_key(message_key, "\"content\"");
    if (content_key == NULL) {
        return UYA_OPENAI_CHAT_NO_CONTENT;
    }
    p = uya_openai_chat_skip_ws(content_key);
    if (p == NULL || *p != ':') {
        return UYA_OPENAI_CHAT_PARSE_ERROR;
    }
    p = uya_openai_chat_skip_ws(p + 1);
    if (p == NULL || *p != '"') {
        return UYA_OPENAI_CHAT_NO_CONTENT;
    }
    if (!uya_openai_chat_json_decode_string(p, &decoded, &decoded_len)) {
        return UYA_OPENAI_CHAT_PARSE_ERROR;
    }
    req->assistant_content = decoded;
    req->assistant_len = decoded_len;
    return UYA_OPENAI_CHAT_READY;
}

static int uya_openai_chat_exec_request(UyaOpenAiChatRequest *req) {
    CURL *curl = NULL;
    CURLcode rc;
    struct curl_slist *headers = NULL;
    char endpoint[320];
    char auth_header[320];
    struct timeval tv_begin;
    struct timeval tv_end;
    long elapsed_ms = 0L;
    char request_preview[192];
    char response_preview[192];
    if (req == NULL) {
        return UYA_OPENAI_CHAT_INVALID_ARG;
    }
    request_preview[0] = '\0';
    response_preview[0] = '\0';
    uya_openai_chat_preview(req->request_body, request_preview, sizeof(request_preview));
    uya_openai_chat_debugf(
        "child request start base_url=%s api_path=%s bytes=%lu preview=%s",
        req->base_url,
        req->api_path,
        (unsigned long)req->request_len,
        request_preview
    );
    (void)gettimeofday(&tv_begin, NULL);
    (void)curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl == NULL) {
        uya_openai_chat_debugf("child request init failed");
        return UYA_OPENAI_CHAT_TRANSPORT_ERROR;
    }

    uya_openai_chat_join_url(endpoint, sizeof(endpoint), req->base_url, req->api_path);
    (void)snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", req->api_key);

    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, endpoint);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->request_body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)req->request_len);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1500L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 3500L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, uya_openai_chat_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, uya_openai_chat_xferinfo_cb);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, req);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "uya-gui-doudizhu/1.0");
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    rc = curl_easy_perform(curl);
    (void)gettimeofday(&tv_end, NULL);
    elapsed_ms = (tv_end.tv_sec - tv_begin.tv_sec) * 1000L + (tv_end.tv_usec - tv_begin.tv_usec) / 1000L;
    if (req->cancel_requested) {
        req->result_code = UYA_OPENAI_CHAT_CANCELLED;
    } else if (req->result_code == UYA_OPENAI_CHAT_RESPONSE_TOO_LARGE) {
        /* keep oversized marker */
    } else if (rc != CURLE_OK) {
        req->result_code = UYA_OPENAI_CHAT_TRANSPORT_ERROR;
    } else if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &req->http_status) != CURLE_OK) {
        req->result_code = UYA_OPENAI_CHAT_TRANSPORT_ERROR;
    } else if (req->http_status < 200L || req->http_status >= 300L) {
        req->result_code = UYA_OPENAI_CHAT_HTTP_ERROR;
    } else {
        req->result_code = uya_openai_chat_extract_content(req);
    }
    uya_openai_chat_preview(req->http_body, response_preview, sizeof(response_preview));
    uya_openai_chat_debugf(
        "child request done status=%s curl=%d http=%ld elapsed_ms=%ld body_bytes=%lu response=%s",
        uya_openai_chat_status_name(req->result_code),
        (int)rc,
        req->http_status,
        elapsed_ms,
        (unsigned long)req->http_len,
        response_preview
    );
    if (rc != CURLE_OK) {
        uya_openai_chat_debugf("curl error=%s", curl_easy_strerror(rc));
    }
    if (req->assistant_content != NULL) {
        char assistant_preview[192];
        uya_openai_chat_preview(req->assistant_content, assistant_preview, sizeof(assistant_preview));
        uya_openai_chat_debugf("assistant content bytes=%lu preview=%s", (unsigned long)req->assistant_len, assistant_preview);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return req->result_code;
}

static void uya_openai_chat_child_main(UyaOpenAiChatRequest *req, int write_fd) {
    UyaOpenAiChatReplyHeader header;
    memset(&header, 0, sizeof(header));
    header.result_code = uya_openai_chat_exec_request(req);
    header.http_status = (int32_t)req->http_status;
    header.assistant_len = (int32_t)req->assistant_len;
    if (!uya_openai_chat_write_all(write_fd, &header, sizeof(header))) {
        _exit(1);
    }
    if (header.result_code == UYA_OPENAI_CHAT_READY && req->assistant_content != NULL && req->assistant_len > 0u) {
        if (!uya_openai_chat_write_all(write_fd, req->assistant_content, req->assistant_len)) {
            _exit(1);
        }
    }
    _exit(0);
}

int32_t uya_openai_chat_available(void) {
    return uya_openai_chat_enabled() ? 1 : 0;
}

int32_t uya_openai_chat_get_model(uint8_t *out_model, int32_t out_cap) {
    UyaOpenAiResolvedConfig cfg;
    size_t len;
    if (out_model == NULL || out_cap <= 0) {
        return 0;
    }
    cfg = uya_openai_chat_resolve_config();
    len = strlen(cfg.model);
    if (len == 0u) {
        out_model[0] = 0u;
        return 0;
    }
    if ((size_t)out_cap <= len) {
        len = (size_t)(out_cap - 1);
    }
    memcpy(out_model, cfg.model, len);
    out_model[len] = 0u;
    return 1;
}

int32_t uya_openai_chat_start(const uint8_t *request_body, size_t request_len) {
    UyaOpenAiChatRequest *req = &g_openai_req;
    UyaOpenAiResolvedConfig cfg;
    char *body_copy;
    int pipefd[2];
    pid_t pid;
    if (request_body == NULL || request_len == 0u) {
        return UYA_OPENAI_CHAT_INVALID_ARG;
    }
    cfg = uya_openai_chat_resolve_config();
    uya_openai_chat_debugf(
        "start request enabled=%d source=%s model=%s base_url=%s api_path=%s api_key_present=%d bytes=%lu",
        cfg.enabled,
        cfg.config_source,
        cfg.model,
        cfg.base_url,
        cfg.api_path,
        cfg.api_key[0] != '\0',
        (unsigned long)request_len
    );
    if (!cfg.enabled) {
        uya_openai_chat_debugf("start request rejected: disabled");
        return UYA_OPENAI_CHAT_DISABLED;
    }
    if (cfg.api_key[0] == '\0') {
        uya_openai_chat_debugf("start request rejected: missing api key");
        return UYA_OPENAI_CHAT_DISABLED;
    }

    body_copy = (char *)malloc(request_len + 1u);
    if (body_copy == NULL) {
        return UYA_OPENAI_CHAT_TRANSPORT_ERROR;
    }
    (void)memcpy(body_copy, request_body, request_len);
    body_copy[request_len] = '\0';

    pthread_mutex_lock(&g_openai_mutex);
    if (req->in_use) {
        pthread_mutex_unlock(&g_openai_mutex);
        free(body_copy);
        uya_openai_chat_debugf("start request rejected: busy");
        return UYA_OPENAI_CHAT_BUSY;
    }
    memset(req, 0, sizeof(*req));
    req->in_use = 1;
    req->handle = 1;
    req->pipe_fd = -1;
    req->request_body = body_copy;
    req->request_len = request_len;
    req->result_code = UYA_OPENAI_CHAT_PENDING;
    uya_openai_chat_copy_value(req->base_url, sizeof(req->base_url), cfg.base_url);
    uya_openai_chat_copy_value(req->api_path, sizeof(req->api_path), cfg.api_path);
    uya_openai_chat_copy_value(req->api_key, sizeof(req->api_key), cfg.api_key);
    if (pipe(pipefd) != 0) {
        uya_openai_chat_reset_slot(req);
        pthread_mutex_unlock(&g_openai_mutex);
        uya_openai_chat_debugf("start request failed: pipe errno=%d", errno);
        return UYA_OPENAI_CHAT_TRANSPORT_ERROR;
    }
    pid = fork();
    if (pid < 0) {
        (void)close(pipefd[0]);
        (void)close(pipefd[1]);
        uya_openai_chat_reset_slot(req);
        pthread_mutex_unlock(&g_openai_mutex);
        uya_openai_chat_debugf("start request failed: fork errno=%d", errno);
        return UYA_OPENAI_CHAT_TRANSPORT_ERROR;
    }
    if (pid == 0) {
        (void)close(pipefd[0]);
        uya_openai_chat_child_main(req, pipefd[1]);
    }
    (void)close(pipefd[1]);
    req->child_pid = pid;
    req->pipe_fd = pipefd[0];
    pthread_mutex_unlock(&g_openai_mutex);
    uya_openai_chat_debugf("start request accepted handle=%d child_pid=%ld", req->handle, (long)pid);
    return req->handle;
}

int32_t uya_openai_chat_poll(int32_t handle, uint8_t *out_body, int32_t out_cap, int32_t *out_len) {
    UyaOpenAiChatRequest *req = &g_openai_req;
    int result;
    if (out_len != NULL) {
        *out_len = 0;
    }
    if (out_body != NULL && out_cap > 0) {
        out_body[0] = 0;
    }
    if (handle <= 0 || out_body == NULL || out_len == NULL || out_cap <= 0) {
        return UYA_OPENAI_CHAT_INVALID_ARG;
    }

    pthread_mutex_lock(&g_openai_mutex);
    if (!req->in_use || req->handle != handle) {
        pthread_mutex_unlock(&g_openai_mutex);
        return UYA_OPENAI_CHAT_INVALID_HANDLE;
    }
    if (!req->done) {
        int status = 0;
        pid_t waited = waitpid(req->child_pid, &status, WNOHANG);
        if (waited == 0) {
            pthread_mutex_unlock(&g_openai_mutex);
            return UYA_OPENAI_CHAT_PENDING;
        }
        req->done = 1;
        if (waited < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            req->result_code = req->cancel_requested ? UYA_OPENAI_CHAT_CANCELLED : UYA_OPENAI_CHAT_TRANSPORT_ERROR;
        } else {
            char *reply = NULL;
            size_t reply_len = 0u;
            if (req->pipe_fd < 0 || !uya_openai_chat_read_all(req->pipe_fd, &reply, &reply_len) ||
                reply_len < sizeof(UyaOpenAiChatReplyHeader)) {
                free(reply);
                req->result_code = req->cancel_requested ? UYA_OPENAI_CHAT_CANCELLED : UYA_OPENAI_CHAT_TRANSPORT_ERROR;
            } else {
                UyaOpenAiChatReplyHeader header;
                (void)memcpy(&header, reply, sizeof(header));
                req->http_status = (long)header.http_status;
                req->result_code = header.result_code;
                if (header.assistant_len > 0) {
                    size_t body_len = (size_t)header.assistant_len;
                    if (reply_len != sizeof(header) + body_len) {
                        req->result_code = UYA_OPENAI_CHAT_TRANSPORT_ERROR;
                    } else {
                        req->assistant_content = (char *)malloc(body_len + 1u);
                        if (req->assistant_content == NULL) {
                            req->result_code = UYA_OPENAI_CHAT_TRANSPORT_ERROR;
                        } else {
                            (void)memcpy(req->assistant_content, reply + sizeof(header), body_len);
                            req->assistant_content[body_len] = '\0';
                            req->assistant_len = body_len;
                        }
                    }
                }
                free(reply);
            }
        }
        uya_openai_chat_debugf(
            "poll completed handle=%d child_pid=%ld waitpid=%ld status=%s http=%ld assistant_bytes=%lu",
            handle,
            (long)req->child_pid,
            (long)waited,
            uya_openai_chat_status_name(req->result_code),
            req->http_status,
            (unsigned long)req->assistant_len
        );
    }
    result = req->result_code;
    if (result == UYA_OPENAI_CHAT_READY) {
        if ((size_t)out_cap <= req->assistant_len) {
            result = UYA_OPENAI_CHAT_RESPONSE_TOO_LARGE;
        } else {
            (void)memcpy(out_body, req->assistant_content, req->assistant_len);
            out_body[req->assistant_len] = '\0';
            *out_len = (int32_t)req->assistant_len;
        }
    }
    uya_openai_chat_reset_slot(req);
    pthread_mutex_unlock(&g_openai_mutex);
    return result;
}

void uya_openai_chat_cancel(int32_t handle) {
    UyaOpenAiChatRequest *req = &g_openai_req;
    if (handle <= 0) {
        return;
    }
    pthread_mutex_lock(&g_openai_mutex);
    if (!req->in_use || req->handle != handle) {
        pthread_mutex_unlock(&g_openai_mutex);
        return;
    }
    req->cancel_requested = 1;
    if (req->child_pid > 0) {
        uya_openai_chat_debugf("cancel request handle=%d child_pid=%ld", handle, (long)req->child_pid);
        (void)kill(req->child_pid, SIGKILL);
        (void)waitpid(req->child_pid, NULL, 0);
        req->done = 1;
        req->result_code = UYA_OPENAI_CHAT_CANCELLED;
    }
    pthread_mutex_unlock(&g_openai_mutex);

    pthread_mutex_lock(&g_openai_mutex);
    if (req->in_use && req->handle == handle) {
        uya_openai_chat_reset_slot(req);
    }
    pthread_mutex_unlock(&g_openai_mutex);
}
