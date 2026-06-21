#include <emscripten.h>
#include <emscripten/html5.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <wasi/api.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
    UYA_GUI_WEB_EVT_NONE = 0,
    UYA_GUI_WEB_EVT_POINTER_MOVE = 1,
    UYA_GUI_WEB_EVT_POINTER_DOWN = 2,
    UYA_GUI_WEB_EVT_POINTER_UP = 3,
    UYA_GUI_WEB_EVT_WHEEL = 4,
    UYA_GUI_WEB_EVT_KEY_DOWN = 5,
    UYA_GUI_WEB_EVT_KEY_UP = 6,
    UYA_GUI_WEB_EVT_RESET_INPUT = 7,
    UYA_GUI_WEB_EVT_REFRESH = 8,
    UYA_GUI_WEB_EVT_TOUCH_CANCEL = 9,
};

typedef struct UyaGuiWebDisplay {
    int width;
    int height;
    int scale;
    uint8_t *rgba_pixels;
    size_t rgba_bytes;
    int refresh_requested;
    int dirty_overlay_enabled;
} UyaGuiWebDisplay;

#define web_feed_host_event gui_platform_web_web_common_web_feed_host_event
#define web_feed_host_text_event gui_platform_web_web_common_web_feed_host_text_event
#define rich_text_host_activate gui_platform_text_host_rich_text_host_activate
#define rich_text_host_deactivate gui_platform_text_host_rich_text_host_deactivate
#define rich_text_host_active_session gui_platform_text_host_rich_text_host_active_session
#define rich_text_host_caret_rect_x gui_platform_text_host_rich_text_host_caret_rect_x
#define rich_text_host_caret_rect_y gui_platform_text_host_rich_text_host_caret_rect_y
#define rich_text_host_caret_rect_w gui_platform_text_host_rich_text_host_caret_rect_w
#define rich_text_host_caret_rect_h gui_platform_text_host_rich_text_host_caret_rect_h
#define rich_text_host_selection_text_ptr gui_platform_text_host_rich_text_host_selection_text_ptr
#define rich_text_host_selection_text_len gui_platform_text_host_rich_text_host_selection_text_len
#define rich_text_host_feed_command gui_platform_text_host_rich_text_host_feed_command
#define rich_text_host_feed_text_command gui_platform_text_host_rich_text_host_feed_text_command
#define sim_web_frame gui_sim_runner_web_sim_web_frame
#define sim_web_shutdown gui_sim_runner_web_sim_web_shutdown
#define sim_web_richtext_export_plain_text gui_sim_runner_web_sim_web_richtext_export_plain_text
#define sim_web_richtext_export_html gui_sim_runner_web_sim_web_richtext_export_html
#define sim_web_richtext_insert_text gui_sim_runner_web_sim_web_richtext_insert_text
#define sim_web_richtext_paste_plain gui_sim_runner_web_sim_web_richtext_paste_plain
#define sim_web_richtext_focus_editor gui_sim_runner_web_sim_web_richtext_focus_editor
#define sim_web_richtext_select_all gui_sim_runner_web_sim_web_richtext_select_all

extern bool web_feed_host_event(uint8_t kind, int16_t x, int16_t y, int32_t value, uint16_t key_code, uint16_t modifiers);
extern bool web_feed_host_text_event(const uint8_t *text, size_t len, uint16_t modifiers);
extern void rich_text_host_activate(uint32_t session_id, void *obj);
extern void rich_text_host_deactivate(uint32_t session_id);
extern uint32_t rich_text_host_active_session(void);
extern int16_t rich_text_host_caret_rect_x(void);
extern int16_t rich_text_host_caret_rect_y(void);
extern uint16_t rich_text_host_caret_rect_w(void);
extern uint16_t rich_text_host_caret_rect_h(void);
extern const uint8_t *rich_text_host_selection_text_ptr(void);
extern size_t rich_text_host_selection_text_len(void);
extern bool rich_text_host_feed_command(uint8_t kind);
extern bool rich_text_host_feed_text_command(uint8_t kind, const uint8_t *text, size_t len);
extern int32_t sim_web_frame(int32_t now_ms);
extern void sim_web_shutdown(void);
extern size_t sim_web_richtext_export_plain_text(uint8_t *out, size_t cap);
extern size_t sim_web_richtext_export_html(uint8_t *out, size_t cap);
extern bool sim_web_richtext_insert_text(const uint8_t *text, size_t len);
extern bool sim_web_richtext_paste_plain(const uint8_t *text, size_t len);
extern bool sim_web_richtext_focus_editor(void);
extern bool sim_web_richtext_select_all(void);

static UyaGuiWebDisplay *g_web_display = NULL;
static int g_web_loop_active = 0;
static char g_web_last_error[256] = {0};

static int uya_unknown_host_wasi_err_i32(__wasi_errno_t err) {
    if (err != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

static ssize_t uya_unknown_host_wasi_err_ssize(__wasi_errno_t err, __wasi_size_t count) {
    if (err != 0) {
        errno = err;
        return -1;
    }
    return (ssize_t)count;
}

ssize_t uya_unknown_host_write(int32_t fd, const uint8_t *buf, size_t count) {
    __wasi_ciovec_t iov = {
        .buf = buf,
        .buf_len = count,
    };
    __wasi_size_t written = 0;
    return uya_unknown_host_wasi_err_ssize(__wasi_fd_write(fd, &iov, 1u, &written), written);
}

ssize_t uya_unknown_host_read(int32_t fd, uint8_t *buf, size_t count) {
    __wasi_iovec_t iov = {
        .buf = buf,
        .buf_len = count,
    };
    __wasi_size_t read_count = 0;
    return uya_unknown_host_wasi_err_ssize(__wasi_fd_read(fd, &iov, 1u, &read_count), read_count);
}

ssize_t uya_unknown_host_writev(int32_t fd, const struct iovec *iov, int32_t iovcnt) {
    if (iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }
    __wasi_size_t written = 0;
    return uya_unknown_host_wasi_err_ssize(__wasi_fd_write(fd, (const __wasi_ciovec_t *)iov, (size_t)iovcnt, &written), written);
}

int32_t uya_unknown_host_open(const uint8_t *pathname, int32_t flags, int32_t mode) {
    return open((const char *)pathname, flags, mode);
}

int32_t uya_unknown_host_close(int32_t fd) {
    return uya_unknown_host_wasi_err_i32(__wasi_fd_close(fd));
}

int32_t uya_unknown_host_access(const uint8_t *pathname, int32_t mode) {
    (void)mode;
    int32_t fd = open((const char *)pathname, O_RDONLY, 0);
    if (fd < 0) {
        return -1;
    }
    return uya_unknown_host_wasi_err_i32(__wasi_fd_close(fd));
}

int64_t uya_unknown_host_lseek(int32_t fd, int64_t offset, int32_t whence) {
    __wasi_whence_t wasi_whence = __WASI_WHENCE_SET;
    if (whence == SEEK_SET) {
        wasi_whence = __WASI_WHENCE_SET;
    } else if (whence == SEEK_CUR) {
        wasi_whence = __WASI_WHENCE_CUR;
    } else if (whence == SEEK_END) {
        wasi_whence = __WASI_WHENCE_END;
    } else {
        errno = EINVAL;
        return -1;
    }
    __wasi_filesize_t new_offset = 0;
    if (__wasi_fd_seek(fd, (__wasi_filedelta_t)offset, wasi_whence, &new_offset) != 0) {
        errno = EINVAL;
        return -1;
    }
    return (int64_t)new_offset;
}

void *uya_unknown_host_mmap(void *addr, size_t length, int32_t prot, int32_t flags, int32_t fd, int64_t offset) {
    return mmap(addr, length, prot, flags, fd, (off_t)offset);
}

int32_t uya_unknown_host_munmap(void *addr, size_t length) {
    return munmap(addr, length);
}

int32_t uya_unknown_host_mprotect(void *addr, size_t length, int32_t prot) {
    return mprotect(addr, length, prot);
}

int32_t uya_unknown_host_fcntl(int32_t fd, int32_t cmd, int32_t arg) {
    return fcntl(fd, cmd, arg);
}

int32_t uya_unknown_host_mkdir(const uint8_t *pathname, int32_t mode) {
    (void)pathname;
    (void)mode;
    errno = ENOSYS;
    return -1;
}

int32_t uya_unknown_host_rmdir(const uint8_t *pathname) {
    (void)pathname;
    errno = ENOSYS;
    return -1;
}

int32_t uya_unknown_host_chdir(const uint8_t *path) {
    if (path != NULL && strcmp((const char *)path, "/") == 0) {
        return 0;
    }
    errno = ENOSYS;
    return -1;
}

uint8_t *uya_unknown_host_getcwd(uint8_t *buf, size_t size) {
    if (buf == NULL || size < 2u) {
        errno = ERANGE;
        return NULL;
    }
    buf[0] = '/';
    buf[1] = '\0';
    return buf;
}

int32_t uya_unknown_host_dup(int32_t fd) {
    (void)fd;
    errno = ENOSYS;
    return -1;
}

int32_t uya_unknown_host_dup2(int32_t oldfd, int32_t newfd) {
    (void)oldfd;
    (void)newfd;
    errno = ENOSYS;
    return -1;
}

void uya_unknown_host_exit(int32_t code) {
    __wasi_proc_exit((__wasi_exitcode_t)code);
}

int32_t uya_gui_web_host_now_ms(void) {
    return (int32_t)emscripten_get_now();
}

int32_t uya_gui_web_host_clock_gettime(int32_t clock_id, int64_t *tv_sec, int64_t *tv_nsec) {
    double now_ms = emscripten_get_now();
    int64_t whole_ms = (int64_t)now_ms;
    (void)clock_id;
    if (tv_sec != NULL) {
        *tv_sec = whole_ms / 1000;
    }
    if (tv_nsec != NULL) {
        *tv_nsec = (whole_ms % 1000) * 1000000;
    }
    return 0;
}

int32_t uya_gui_web_host_gettimeofday(int64_t *tv_sec, int64_t *tv_usec) {
    double now_ms = emscripten_get_now();
    int64_t whole_ms = (int64_t)now_ms;
    if (tv_sec != NULL) {
        *tv_sec = whole_ms / 1000;
    }
    if (tv_usec != NULL) {
        *tv_usec = (whole_ms % 1000) * 1000;
    }
    return 0;
}

int32_t uya_gui_web_host_nanosleep(int64_t req_sec, int64_t req_nsec, int64_t *rem_sec, int64_t *rem_nsec) {
    (void)req_sec;
    (void)req_nsec;
    if (rem_sec != NULL) {
        *rem_sec = 0;
    }
    if (rem_nsec != NULL) {
        *rem_nsec = 0;
    }
    return 0;
}

int32_t uya_gui_web_host_fstat_size(int32_t fd, int64_t *out_size) {
    __wasi_filesize_t cur = 0;
    __wasi_filesize_t end = 0;
    if (__wasi_fd_seek(fd, 0, __WASI_WHENCE_CUR, &cur) != 0) {
        return -1;
    }
    if (__wasi_fd_seek(fd, 0, __WASI_WHENCE_END, &end) != 0) {
        return -1;
    }
    if (__wasi_fd_seek(fd, (__wasi_filedelta_t)cur, __WASI_WHENCE_SET, &cur) != 0) {
        return -1;
    }
    if (out_size != NULL) {
        *out_size = (int64_t)end;
    }
    return 0;
}

static void uya_gui_web_set_error(const char *msg) {
    if (msg == NULL) {
        g_web_last_error[0] = '\0';
        return;
    }
    snprintf(g_web_last_error, sizeof(g_web_last_error), "%s", msg);
}

EM_JS(int, uya_gui_web_js_setup_canvas, (int width, int height, int scale, const char *title_ptr), {
    if (!Module.uyaGuiSetupCanvas) {
        return 0;
    }
    return Module.uyaGuiSetupCanvas(width, height, scale, UTF8ToString(title_ptr || 0)) | 0;
})

EM_JS(void, uya_gui_web_js_present, (const uint8_t *rgba_ptr, int width, int height), {
    if (Module.uyaGuiPresent) {
        Module.uyaGuiPresent(rgba_ptr, width, height);
    }
})

EM_JS(void, uya_gui_web_js_present_region, (const uint8_t *rgba_ptr, int width, int height, int x, int y, int rect_w, int rect_h), {
    if (Module.uyaGuiPresentRegion) {
        Module.uyaGuiPresentRegion(rgba_ptr, width, height, x, y, rect_w, rect_h);
    }
})

EM_JS(void, uya_gui_web_js_set_title, (const char *title_ptr), {
    if (Module.uyaGuiSetTitle) {
        Module.uyaGuiSetTitle(UTF8ToString(title_ptr || 0));
    }
})

EM_JS(int, uya_gui_web_js_request_fullscreen, (void), {
    if (!Module.uyaGuiRequestFullscreen) {
        return 0;
    }
    return Module.uyaGuiRequestFullscreen() | 0;
})

EM_JS(void, uya_gui_web_js_clear_dirty_overlay, (void), {
    if (Module.uyaGuiClearDirtyOverlay) {
        Module.uyaGuiClearDirtyOverlay();
    }
})

EM_JS(void, uya_gui_web_js_draw_dirty_rect, (int x, int y, int w, int h), {
    if (Module.uyaGuiDrawDirtyRect) {
        Module.uyaGuiDrawDirtyRect(x, y, w, h);
    }
})

EM_JS(void, uya_gui_web_js_set_dirty_overlay_enabled, (int enabled), {
    if (Module.uyaGuiSetDirtyOverlayEnabled) {
        Module.uyaGuiSetDirtyOverlayEnabled(enabled);
    }
})

EM_JS(void, uya_gui_web_js_shutdown, (void), {
    if (Module.uyaGuiShutdown) {
        Module.uyaGuiShutdown();
    }
})

EM_JS(void, uya_gui_web_js_ensure_runtime_dirs, (void), {
    try { FS.mkdir('/tmp'); } catch (e) {}
    try { FS.mkdir('/app'); } catch (e) {}
})

EM_JS(int, uya_gui_web_js_request_bitmap_font, (int pixel_height), {
    if (!Module.uyaGuiRequestBitmapFont) {
        return 0;
    }
    return Module.uyaGuiRequestBitmapFont(pixel_height | 0) | 0;
})

EM_JS(int, uya_gui_web_js_bitmap_font_state, (int pixel_height), {
    if (!Module.uyaGuiBitmapFontState) {
        return 0;
    }
    return Module.uyaGuiBitmapFontState(pixel_height | 0) | 0;
})

int32_t uya_gui_web_host_request_bitmap_font(uint16_t pixel_height) {
    return uya_gui_web_js_request_bitmap_font((int)pixel_height);
}

int32_t uya_gui_web_host_bitmap_font_state(uint16_t pixel_height) {
    return uya_gui_web_js_bitmap_font_state((int)pixel_height);
}

EMSCRIPTEN_KEEPALIVE void uya_gui_web_host_feed_event(uint8_t kind, int16_t x, int16_t y, int32_t value, uint16_t key_code, uint16_t modifiers) {
    (void)web_feed_host_event(kind, x, y, value, key_code, modifiers);
    if (kind == UYA_GUI_WEB_EVT_REFRESH && g_web_display != NULL) {
        g_web_display->refresh_requested = 1;
    }
}

EMSCRIPTEN_KEEPALIVE void uya_gui_web_host_feed_text_input(const uint8_t *text, int32_t len, uint16_t modifiers) {
    if (text == NULL || len <= 0) {
        return;
    }
    (void)web_feed_host_text_event(text, (size_t)len, modifiers);
}

EMSCRIPTEN_KEEPALIVE uint32_t uya_gui_web_host_richtext_active_session(void) {
    return rich_text_host_active_session();
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_caret_x(void) {
    return (int32_t)rich_text_host_caret_rect_x();
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_caret_y(void) {
    return (int32_t)rich_text_host_caret_rect_y();
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_caret_w(void) {
    return (int32_t)rich_text_host_caret_rect_w();
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_caret_h(void) {
    return (int32_t)rich_text_host_caret_rect_h();
}

EMSCRIPTEN_KEEPALIVE const uint8_t *uya_gui_web_host_richtext_selection_text_ptr(void) {
    return rich_text_host_selection_text_ptr();
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_selection_text_len(void) {
    return (int32_t)rich_text_host_selection_text_len();
}

EMSCRIPTEN_KEEPALIVE void uya_gui_web_host_richtext_feed_command(uint8_t kind) {
    (void)rich_text_host_feed_command(kind);
}

EMSCRIPTEN_KEEPALIVE void uya_gui_web_host_richtext_feed_text(uint8_t kind, const uint8_t *text, int32_t len) {
    if (text == NULL || len < 0) {
        return;
    }
    (void)rich_text_host_feed_text_command(kind, text, (size_t)len);
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_plain_text_copy(uint8_t *out, int32_t cap) {
    if (out == NULL || cap <= 0) {
        return 0;
    }
    return (int32_t)sim_web_richtext_export_plain_text(out, (size_t)cap);
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_html_copy(uint8_t *out, int32_t cap) {
    if (out == NULL || cap <= 0) {
        return 0;
    }
    return (int32_t)sim_web_richtext_export_html(out, (size_t)cap);
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_insert_text(const uint8_t *text, int32_t len) {
    if (text == NULL || len <= 0) {
        return 0;
    }
    return sim_web_richtext_insert_text(text, (size_t)len) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_paste_plain(const uint8_t *text, int32_t len) {
    if (text == NULL || len <= 0) {
        return 0;
    }
    return sim_web_richtext_paste_plain(text, (size_t)len) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_focus_editor(void) {
    return sim_web_richtext_focus_editor() ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE int32_t uya_gui_web_host_richtext_select_all(void) {
    return sim_web_richtext_select_all() ? 1 : 0;
}

void *uya_gui_web_display_open(int32_t width, int32_t height, int32_t scale, const uint8_t *title) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)calloc(1u, sizeof(UyaGuiWebDisplay));
    if (display == NULL) {
        uya_gui_web_set_error("web display alloc failed");
        return NULL;
    }
    display->width = width;
    display->height = height;
    display->scale = scale > 0 ? scale : 1;
    display->rgba_bytes = (size_t)width * (size_t)height * 4u;
    display->rgba_pixels = (uint8_t *)malloc(display->rgba_bytes);
    if (display->rgba_pixels == NULL) {
        free(display);
        uya_gui_web_set_error("web rgba buffer alloc failed");
        return NULL;
    }
    if (!uya_gui_web_js_setup_canvas(width, height, display->scale, (const char *)title)) {
        free(display->rgba_pixels);
        free(display);
        uya_gui_web_set_error("canvas init failed");
        return NULL;
    }
    uya_gui_web_js_ensure_runtime_dirs();
    g_web_display = display;
    uya_gui_web_set_error(NULL);
    return display;
}

void uya_gui_web_display_close(void *handle) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)handle;
    if (display == NULL) {
        return;
    }
    if (g_web_display == display) {
        g_web_display = NULL;
    }
    free(display->rgba_pixels);
    free(display);
}

static void uya_gui_web_swizzle_argb_to_rgba(UyaGuiWebDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height) {
    for (int32_t y = 0; y < height; ++y) {
        const uint8_t *src = pixels + (size_t)y * (size_t)pitch;
        uint8_t *dst = display->rgba_pixels + (size_t)y * (size_t)width * 4u;
        for (int32_t x = 0; x < width; ++x) {
            dst[0] = src[1];
            dst[1] = src[2];
            dst[2] = src[3];
            dst[3] = src[0];
            src += 4;
            dst += 4;
        }
    }
}

static void uya_gui_web_swizzle_argb_to_rgba_region(UyaGuiWebDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t x, int32_t y, int32_t rect_w, int32_t rect_h) {
    for (int32_t row = 0; row < rect_h; ++row) {
        const uint8_t *src = pixels + (size_t)(y + row) * (size_t)pitch + (size_t)x * 4u;
        uint8_t *dst = display->rgba_pixels + ((size_t)(y + row) * (size_t)width + (size_t)x) * 4u;
        for (int32_t col = 0; col < rect_w; ++col) {
            dst[0] = src[1];
            dst[1] = src[2];
            dst[2] = src[3];
            dst[3] = src[0];
            src += 4;
            dst += 4;
        }
    }
}

static int uya_gui_web_display_validate_present(UyaGuiWebDisplay *display, const uint8_t *pixels, int32_t width, int32_t height, int32_t format_tag) {
    if (display == NULL || pixels == NULL) {
        uya_gui_web_set_error("invalid present arguments");
        return 0;
    }
    if (width != display->width || height != display->height) {
        uya_gui_web_set_error("present geometry mismatch");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_web_set_error("web backend only supports ARGB8888 source");
        return 0;
    }
    return 1;
}

int32_t uya_gui_web_display_present(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)handle;
    if (!uya_gui_web_display_validate_present(display, pixels, width, height, format_tag)) {
        return 0;
    }
    uya_gui_web_js_clear_dirty_overlay();
    uya_gui_web_swizzle_argb_to_rgba(display, pixels, pitch, width, height);
    uya_gui_web_js_present(display->rgba_pixels, width, height);
    if (display->dirty_overlay_enabled) {
        uya_gui_web_js_draw_dirty_rect(0, 0, width, height);
    }
    return 1;
}

int32_t uya_gui_web_display_present_begin(void *handle, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)handle;
    if (!uya_gui_web_display_validate_present(display, display != NULL ? display->rgba_pixels : NULL, width, height, format_tag)) {
        return 0;
    }
    uya_gui_web_js_clear_dirty_overlay();
    return 1;
}

int32_t uya_gui_web_display_present_region(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag, int32_t x, int32_t y, int32_t rect_w, int32_t rect_h) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)handle;
    if (!uya_gui_web_display_validate_present(display, pixels, width, height, format_tag)) {
        return 0;
    }
    if (rect_w <= 0 || rect_h <= 0) {
        return 1;
    }
    if (x < 0 || y < 0 || x + rect_w > width || y + rect_h > height) {
        uya_gui_web_set_error("dirty rect out of bounds");
        return 0;
    }
    uya_gui_web_swizzle_argb_to_rgba_region(display, pixels, pitch, width, x, y, rect_w, rect_h);
    uya_gui_web_js_present_region(display->rgba_pixels, width, height, x, y, rect_w, rect_h);
    if (display->dirty_overlay_enabled) {
        uya_gui_web_js_draw_dirty_rect(x, y, rect_w, rect_h);
    }
    return 1;
}

int32_t uya_gui_web_display_present_end(void *handle) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)handle;
    if (display == NULL) {
        uya_gui_web_set_error("invalid present handle");
        return 0;
    }
    return 1;
}

int32_t uya_gui_web_display_consume_refresh_request(void *handle) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)handle;
    if (display == NULL || !display->refresh_requested) {
        return 0;
    }
    display->refresh_requested = 0;
    return 1;
}

void uya_gui_web_display_set_title(void *handle, const uint8_t *title) {
    (void)handle;
    uya_gui_web_js_set_title((const char *)title);
}

int32_t uya_gui_web_display_request_fullscreen(void *handle) {
    (void)handle;
    return uya_gui_web_js_request_fullscreen();
}

int32_t uya_gui_web_display_set_dirty_overlay(void *handle, int32_t enabled) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)handle;
    if (display == NULL) {
        return 0;
    }
    display->dirty_overlay_enabled = enabled != 0;
    uya_gui_web_js_set_dirty_overlay_enabled(enabled);
    return 1;
}

const uint8_t *uya_gui_web_last_error(void) {
    return (const uint8_t *)g_web_last_error;
}

static EM_BOOL uya_gui_web_loop(double time_ms, void *user_data) {
    (void)user_data;
    if (!g_web_loop_active) {
        return EM_FALSE;
    }
    if (sim_web_frame((int32_t)time_ms) == 0) {
        g_web_loop_active = 0;
        sim_web_shutdown();
        uya_gui_web_js_shutdown();
        return EM_FALSE;
    }
    return EM_TRUE;
}

int32_t uya_gui_web_host_start_loop(void) {
    if (g_web_loop_active) {
        return 1;
    }
    g_web_loop_active = 1;
    emscripten_request_animation_frame_loop(uya_gui_web_loop, NULL);
    return 1;
}
