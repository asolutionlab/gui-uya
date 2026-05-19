#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdbool.h>
#include <stdint.h>
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

extern bool web_feed_host_event(uint8_t kind, int16_t x, int16_t y, int32_t value, uint16_t key_code, uint16_t modifiers);
extern int32_t sim_web_frame(int32_t now_ms);
extern void sim_web_shutdown(void);

static UyaGuiWebDisplay *g_web_display = NULL;
static int g_web_loop_active = 0;
static char g_web_last_error[256] = {0};

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
    off_t cur = lseek(fd, 0, SEEK_CUR);
    off_t end = lseek(fd, 0, SEEK_END);
    if (end < 0) {
        return -1;
    }
    if (cur >= 0) {
        (void)lseek(fd, cur, SEEK_SET);
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

EMSCRIPTEN_KEEPALIVE void uya_gui_web_host_feed_event(uint8_t kind, int16_t x, int16_t y, int32_t value, uint16_t key_code, uint16_t modifiers) {
    (void)web_feed_host_event(kind, x, y, value, key_code, modifiers);
    if (kind == UYA_GUI_WEB_EVT_REFRESH && g_web_display != NULL) {
        g_web_display->refresh_requested = 1;
    }
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
