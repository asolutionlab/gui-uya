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
