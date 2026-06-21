#include <stddef.h>
#include <stdint.h>

__attribute__((weak)) int32_t uya_openai_chat_available(void) {
    return 0;
}

__attribute__((weak)) int32_t uya_openai_chat_start(const uint8_t *request_body, size_t request_len) {
    (void)request_body;
    (void)request_len;
    return -2;
}

__attribute__((weak)) int32_t uya_openai_chat_poll(int32_t handle, uint8_t *out_body, int32_t out_cap, int32_t *out_len) {
    (void)handle;
    if (out_body != NULL && out_cap > 0) {
        out_body[0] = 0;
    }
    if (out_len != NULL) {
        *out_len = 0;
    }
    return -4;
}

__attribute__((weak)) void uya_openai_chat_cancel(int32_t handle) {
    (void)handle;
}

__attribute__((weak)) int32_t uya_openai_chat_get_model(uint8_t *out_model, int32_t out_cap) {
    static const char *kDefaultModel = "gpt-5.4-mini";
    size_t i = 0u;
    if (out_model == NULL || out_cap <= 0) {
        return 0;
    }
    while (kDefaultModel[i] != '\0' && (int32_t)(i + 1u) < out_cap) {
        out_model[i] = (uint8_t)kDefaultModel[i];
        ++i;
    }
    if ((int32_t)i >= out_cap) {
        i = (size_t)(out_cap - 1);
    }
    out_model[i] = 0u;
    return i > 0u ? 1 : 0;
}
