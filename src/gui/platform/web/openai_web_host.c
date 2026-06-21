#include <emscripten.h>
#include <stddef.h>
#include <stdint.h>

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

EM_JS(int32_t, uya_gui_web_openai_js_available, (void), {
    if (!Module.uyaGuiOpenAiIsAvailable) {
        return 0;
    }
    return Module.uyaGuiOpenAiIsAvailable() ? 1 : 0;
})

EM_JS(int32_t, uya_gui_web_openai_js_get_model, (uint8_t *out_model, int32_t out_cap), {
    var model = 'gpt-5.4-mini';
    if (Module.uyaGuiOpenAiModelName) {
        model = Module.uyaGuiOpenAiModelName() || model;
    }
    if (!out_model || out_cap <= 0) {
        return 0;
    }
    stringToUTF8(model, out_model, out_cap);
    return model && model.length > 0 ? 1 : 0;
})

EM_JS(int32_t, uya_gui_web_openai_js_start, (const uint8_t *request_body, int32_t request_len), {
    if (!Module.uyaGuiOpenAiStartRequest) {
        return -2;
    }
    return Module.uyaGuiOpenAiStartRequest(request_body, request_len) | 0;
})

EM_JS(int32_t, uya_gui_web_openai_js_poll, (int32_t handle, uint8_t *out_body, int32_t out_cap, int32_t *out_len), {
    if (!Module.uyaGuiOpenAiPollRequest) {
        if (out_len) {
            HEAP32[out_len >> 2] = 0;
        }
        return -2;
    }
    return Module.uyaGuiOpenAiPollRequest(handle, out_body, out_cap, out_len) | 0;
})

EM_JS(void, uya_gui_web_openai_js_cancel, (int32_t handle), {
    if (Module.uyaGuiOpenAiCancelRequest) {
        Module.uyaGuiOpenAiCancelRequest(handle | 0);
    }
})

int32_t uya_openai_chat_available(void) {
    return uya_gui_web_openai_js_available();
}

int32_t uya_openai_chat_start(const uint8_t *request_body, size_t request_len) {
    if (request_body == NULL || request_len == 0u) {
        return UYA_OPENAI_CHAT_INVALID_ARG;
    }
    return uya_gui_web_openai_js_start(request_body, (int32_t)request_len);
}

int32_t uya_openai_chat_poll(int32_t handle, uint8_t *out_body, int32_t out_cap, int32_t *out_len) {
    if (out_len != NULL) {
        *out_len = 0;
    }
    if (out_body != NULL && out_cap > 0) {
        out_body[0] = 0u;
    }
    if (handle <= 0 || out_body == NULL || out_len == NULL || out_cap <= 0) {
        return UYA_OPENAI_CHAT_INVALID_ARG;
    }
    return uya_gui_web_openai_js_poll(handle, out_body, out_cap, out_len);
}

void uya_openai_chat_cancel(int32_t handle) {
    if (handle <= 0) {
        return;
    }
    uya_gui_web_openai_js_cancel(handle);
}

int32_t uya_openai_chat_get_model(uint8_t *out_model, int32_t out_cap) {
    if (out_model == NULL || out_cap <= 0) {
        return 0;
    }
    out_model[0] = 0u;
    return uya_gui_web_openai_js_get_model(out_model, out_cap);
}
