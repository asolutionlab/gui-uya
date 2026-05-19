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
    var title = UTF8ToString(title_ptr || 0);
    if (typeof document === "undefined") {
        return 0;
    }
    var canvas = Module.uyaGuiCanvas || document.getElementById("uya-gui-canvas");
    if (!canvas) {
        canvas = document.createElement("canvas");
        canvas.id = "uya-gui-canvas";
        document.body.style.margin = "0";
        document.body.style.background = "#111";
        document.body.style.display = "flex";
        document.body.style.alignItems = "center";
        document.body.style.justifyContent = "center";
        document.body.appendChild(canvas);
    }
    Module.uyaGuiCanvas = canvas;
    canvas.width = width;
    canvas.height = height;
    canvas.style.width = (width * Math.max(scale, 1)) + "px";
    canvas.style.height = (height * Math.max(scale, 1)) + "px";
    canvas.style.outline = "none";
    canvas.style.touchAction = "none";
    canvas.style.overscrollBehavior = "contain";
    canvas.style.imageRendering = "pixelated";
    canvas.tabIndex = 0;
    if (title.length > 0) {
        document.title = title;
    }
    Module.uyaGuiCtx2d = canvas.getContext("2d", { alpha: false, desynchronized: true });
    if (!Module.uyaGuiCtx2d) {
        return 0;
    }
    if (!Module.uyaGuiImageData || Module.uyaGuiImageData.width !== width || Module.uyaGuiImageData.height !== height) {
        Module.uyaGuiImageData = Module.uyaGuiCtx2d.createImageData(width, height);
    }
    if (canvas.__uyaGuiBound) {
        return 1;
    }

    var mapKey = function(key) {
        switch (key) {
            case "Escape": return 27;
            case "Enter": return 13;
            case " ": return 32;
            case "Spacebar": return 32;
            case "ArrowLeft": return 1000;
            case "ArrowRight": return 1001;
            case "ArrowUp": return 1002;
            case "ArrowDown": return 1003;
            case "F11": return 1011;
            default: return 0;
        }
    };
    var feed = function(kind, x, y, value, keyCode, modifiers) {
        Module._uya_gui_web_host_feed_event(kind, x | 0, y | 0, value | 0, keyCode | 0, modifiers | 0);
    };
    var pointFromEvent = function(clientX, clientY) {
        var rect = canvas.getBoundingClientRect();
        var scaleX = rect.width > 0 ? canvas.width / rect.width : 1;
        var scaleY = rect.height > 0 ? canvas.height / rect.height : 1;
        var x = Math.round((clientX - rect.left) * scaleX);
        var y = Math.round((clientY - rect.top) * scaleY);
        x = Math.max(0, Math.min(canvas.width - 1, x));
        y = Math.max(0, Math.min(canvas.height - 1, y));
        return { x: x, y: y };
    };
    var modifierBits = function(evt) {
        return (evt.altKey ? 1 : 0) | (evt.ctrlKey ? 2 : 0) | (evt.metaKey ? 4 : 0) | (evt.shiftKey ? 8 : 0);
    };

    canvas.addEventListener("mousedown", function(evt) {
        var p = pointFromEvent(evt.clientX, evt.clientY);
        canvas.focus();
        feed(2, p.x, p.y, 0, 0, modifierBits(evt));
        evt.preventDefault();
    });
    window.addEventListener("mousemove", function(evt) {
        var p = pointFromEvent(evt.clientX, evt.clientY);
        feed(1, p.x, p.y, 0, 0, modifierBits(evt));
    });
    window.addEventListener("mouseup", function(evt) {
        var p = pointFromEvent(evt.clientX, evt.clientY);
        feed(3, p.x, p.y, 0, 0, modifierBits(evt));
        evt.preventDefault();
    });
    canvas.addEventListener("touchstart", function(evt) {
        if (evt.changedTouches.length < 1) {
            return;
        }
        var touch = evt.changedTouches[0];
        var p = pointFromEvent(touch.clientX, touch.clientY);
        canvas.focus();
        feed(2, p.x, p.y, 0, 0, 0);
        evt.preventDefault();
    }, { passive: false });
    canvas.addEventListener("touchmove", function(evt) {
        if (evt.changedTouches.length < 1) {
            return;
        }
        var touch = evt.changedTouches[0];
        var p = pointFromEvent(touch.clientX, touch.clientY);
        feed(1, p.x, p.y, 0, 0, 0);
        evt.preventDefault();
    }, { passive: false });
    canvas.addEventListener("touchend", function(evt) {
        if (evt.changedTouches.length < 1) {
            return;
        }
        var touch = evt.changedTouches[0];
        var p = pointFromEvent(touch.clientX, touch.clientY);
        feed(3, p.x, p.y, 0, 0, 0);
        evt.preventDefault();
    }, { passive: false });
    canvas.addEventListener("touchcancel", function(evt) {
        feed(9, 0, 0, 0, 0, 0);
        evt.preventDefault();
    }, { passive: false });
    canvas.addEventListener("wheel", function(evt) {
        var p = pointFromEvent(evt.clientX, evt.clientY);
        var delta = evt.deltaY > 0 ? 1 : (evt.deltaY < 0 ? -1 : 0);
        feed(4, p.x, p.y, delta, 0, modifierBits(evt));
        evt.preventDefault();
    }, { passive: false });
    canvas.addEventListener("keydown", function(evt) {
        var mapped = mapKey(evt.key);
        if (!mapped) {
            return;
        }
        feed(5, 0, 0, 0, mapped, modifierBits(evt));
        evt.preventDefault();
    });
    canvas.addEventListener("keyup", function(evt) {
        var mapped = mapKey(evt.key);
        if (!mapped) {
            return;
        }
        feed(6, 0, 0, 0, mapped, modifierBits(evt));
        evt.preventDefault();
    });
    window.addEventListener("blur", function() {
        feed(7, 0, 0, 0, 0, 0);
    });
    document.addEventListener("visibilitychange", function() {
        if (document.hidden) {
            feed(7, 0, 0, 0, 0, 0);
        } else {
            feed(8, 0, 0, 0, 0, 0);
        }
    });
    window.addEventListener("resize", function() {
        feed(8, 0, 0, 0, 0, 0);
    });
    canvas.__uyaGuiBound = true;
    return 1;
});

EM_JS(void, uya_gui_web_js_present, (const uint8_t *rgba_ptr, int width, int height), {
    var ctx = Module.uyaGuiCtx2d;
    var imageData = Module.uyaGuiImageData;
    if (!ctx || !imageData) {
        return;
    }
    var src = Module.HEAPU8.subarray(rgba_ptr, rgba_ptr + (width * height * 4));
    imageData.data.set(src);
    ctx.putImageData(imageData, 0, 0);
});

EM_JS(void, uya_gui_web_js_set_title, (const char *title_ptr), {
    if (typeof document === "undefined") {
        return;
    }
    document.title = UTF8ToString(title_ptr || 0);
});

EM_JS(int, uya_gui_web_js_request_fullscreen, (), {
    var canvas = Module.uyaGuiCanvas;
    if (!canvas || !canvas.requestFullscreen) {
        return 0;
    }
    canvas.requestFullscreen().catch(function() {});
    return 1;
});

EM_JS(void, uya_gui_web_js_shutdown, (), {
    Module.uyaGuiLoopActive = 0;
});

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
    EM_ASM({
        try { FS.mkdir('/tmp'); } catch (e) {}
        try { FS.mkdir('/app'); } catch (e) {}
    });
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

int32_t uya_gui_web_display_present(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiWebDisplay *display = (UyaGuiWebDisplay *)handle;
    if (display == NULL || pixels == NULL) {
        uya_gui_web_set_error("invalid present arguments");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_web_set_error("web backend only supports ARGB8888 source");
        return 0;
    }
    uya_gui_web_swizzle_argb_to_rgba(display, pixels, pitch, width, height);
    uya_gui_web_js_present(display->rgba_pixels, width, height);
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
