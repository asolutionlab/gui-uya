#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <stdint.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct UyaGuiSimGles2Fns {
    void (*ActiveTexture)(GLenum texture);
    void (*AttachShader)(GLuint program, GLuint shader);
    void (*BindBuffer)(GLenum target, GLuint buffer);
    void (*BindTexture)(GLenum target, GLuint texture);
    void (*BlendFunc)(GLenum sfactor, GLenum dfactor);
    void (*BufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
    void (*Clear)(GLbitfield mask);
    void (*ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (*CompileShader)(GLuint shader);
    GLuint (*CreateProgram)(void);
    GLuint (*CreateShader)(GLenum type);
    void (*Disable)(GLenum cap);
    void (*DeleteBuffers)(GLsizei n, const GLuint *buffers);
    void (*DeleteProgram)(GLuint program);
    void (*DeleteShader)(GLuint shader);
    void (*DeleteTextures)(GLsizei n, const GLuint *textures);
    void (*DisableVertexAttribArray)(GLuint index);
    void (*DrawArrays)(GLenum mode, GLint first, GLsizei count);
    void (*Enable)(GLenum cap);
    void (*EnableVertexAttribArray)(GLuint index);
    void (*GenBuffers)(GLsizei n, GLuint *buffers);
    void (*GenTextures)(GLsizei n, GLuint *textures);
    GLint (*GetAttribLocation)(GLuint program, const GLchar *name);
    void (*GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void (*GetProgramiv)(GLuint program, GLenum pname, GLint *params);
    void (*GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void (*GetShaderiv)(GLuint shader, GLenum pname, GLint *params);
    GLint (*GetUniformLocation)(GLuint program, const GLchar *name);
    void (*LinkProgram)(GLuint program);
    void (*PixelStorei)(GLenum pname, GLint param);
    void (*ShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
    void (*TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
    void (*TexParameteri)(GLenum target, GLenum pname, GLint param);
    void (*TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void (*Uniform1i)(GLint location, GLint v0);
    void (*Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (*UseProgram)(GLuint program);
    void (*VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    void (*Viewport)(GLint x, GLint y, GLsizei width, GLsizei height);
    void (*Scissor)(GLint x, GLint y, GLsizei width, GLsizei height);
} UyaGuiSimGles2Fns;

enum {
    UYA_GUI_SIM_GPU_AUTO = 0,
    UYA_GUI_SIM_GPU_SOFTWARE = 1,
    UYA_GUI_SIM_GPU_GLES2 = 2,
};

enum {
    UYA_GUI_SIM_PRESENT_SOFTWARE = 1,
    UYA_GUI_SIM_PRESENT_GLES2 = 2,
};

#define UYA_GUI_SIM_DIRTY_OVERLAY_MAX_RECTS 32

typedef struct UyaGuiSimDisplay {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_GLContext gl_context;
    UyaGuiSimGles2Fns gl;
    GLuint gl_program;
    GLuint gl_shape_program;
    GLuint gl_texture;
    GLuint gl_white_texture;
    GLuint gl_vbo;
    GLint gl_attr_pos;
    GLint gl_attr_uv;
    GLint gl_uniform_tex;
    GLint gl_uniform_color;
    GLint gl_shape_attr_pos;
    GLint gl_shape_attr_uv;
    GLint gl_shape_uniform_color0;
    GLint gl_shape_uniform_color1;
    GLint gl_shape_uniform_params0;
    GLint gl_shape_uniform_params1;
    uint8_t *gl_rgba_pixels;
    size_t gl_rgba_bytes;
    int width;
    int height;
    int scale;
    int fullscreen;
    int present_kind;
    const uint8_t *pending_pixels;
    int pending_pitch;
    int pending_width;
    int pending_height;
    int pending_format_tag;
    int pending_present;
    int dirty_overlay_enabled;
    int dirty_overlay_full;
    SDL_Rect dirty_overlay_rects[UYA_GUI_SIM_DIRTY_OVERLAY_MAX_RECTS];
    int dirty_overlay_rect_count;
    uint8_t *overlay_pixels;
    size_t overlay_bytes;
    int needs_refresh;
    int direct_frame_active;
} UyaGuiSimDisplay;

typedef struct SdlHostEvent {
    uint8_t kind;
    int16_t x;
    int16_t y;
    int32_t value;
    uint16_t key_code;
    uint16_t reserved;
} SdlHostEvent;

typedef struct UyaGuiRectCmd {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
} UyaGuiRectCmd;

typedef struct UyaGuiColorCmd {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} UyaGuiColorCmd;

typedef struct UyaGuiGpuFillRectCmd {
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
} UyaGuiGpuFillRectCmd;

typedef struct UyaGuiGpuStrokeRectCmd {
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
    int32_t width;
} UyaGuiGpuStrokeRectCmd;

typedef struct UyaGuiGpuLineCmd {
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
    UyaGuiColorCmd color;
    int32_t width;
} UyaGuiGpuLineCmd;

typedef struct UyaGuiGpuImageCmd {
    UyaGuiRectCmd dst_rect;
    UyaGuiRectCmd src_rect;
    const uint8_t *pixels;
    int32_t image_w;
    int32_t image_h;
    int32_t stride;
    int32_t format_tag;
    int32_t has_src;
} UyaGuiGpuImageCmd;

typedef struct UyaGuiGpuGlyphCmd {
    UyaGuiRectCmd dst_rect;
    const uint8_t *pixels;
    int32_t glyph_w;
    int32_t glyph_h;
    int32_t stride;
    int32_t format_tag;
    UyaGuiColorCmd color;
} UyaGuiGpuGlyphCmd;

enum {
    SDL_EVT_NONE = 0,
    SDL_EVT_QUIT = 1,
    SDL_EVT_POINTER_MOVE = 2,
    SDL_EVT_POINTER_DOWN = 3,
    SDL_EVT_POINTER_UP = 4,
    SDL_EVT_KEY_DOWN = 5,
    SDL_EVT_KEY_UP = 6,
    SDL_EVT_WHEEL = 7,
};

static int g_sdl_refcount = 0;
static UyaGuiSimDisplay *g_active_display = NULL;
static char g_last_error[256] = {0};
static SDL_Event g_pending_event;
static int g_has_pending_event = 0;
static void *(*g_host_malloc_fn)(size_t) = NULL;
static void *(*g_host_calloc_fn)(size_t, size_t) = NULL;
static void *(*g_host_realloc_fn)(void *, size_t) = NULL;
static void (*g_host_free_fn)(void *) = NULL;

static void uya_gui_sim_init_host_allocators(void) {
    if (g_host_malloc_fn != NULL && g_host_calloc_fn != NULL && g_host_realloc_fn != NULL && g_host_free_fn != NULL) {
        return;
    }

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    g_host_malloc_fn = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
    g_host_calloc_fn = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
    g_host_realloc_fn = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
    g_host_free_fn = (void (*)(void *))dlsym(RTLD_NEXT, "free");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}

static void *uya_gui_sim_host_malloc(size_t size) {
    uya_gui_sim_init_host_allocators();
    return g_host_malloc_fn != NULL ? g_host_malloc_fn(size) : NULL;
}

static void *uya_gui_sim_host_calloc(size_t nmemb, size_t size) {
    uya_gui_sim_init_host_allocators();
    return g_host_calloc_fn != NULL ? g_host_calloc_fn(nmemb, size) : NULL;
}

static void *uya_gui_sim_host_realloc(void *ptr, size_t size) {
    uya_gui_sim_init_host_allocators();
    return g_host_realloc_fn != NULL ? g_host_realloc_fn(ptr, size) : NULL;
}

static void uya_gui_sim_host_free(void *ptr) {
    uya_gui_sim_init_host_allocators();
    if (g_host_free_fn != NULL) {
        g_host_free_fn(ptr);
    }
}

static void uya_gui_sim_set_error(const char *message) {
    if (message == NULL || message[0] == '\0') {
        message = "unknown SDL2 error";
    }
    (void)snprintf(g_last_error, sizeof(g_last_error), "%s", message);
}

static const char *uya_gui_sim_present_name(const UyaGuiSimDisplay *display) {
    if (display != NULL && display->present_kind == UYA_GUI_SIM_PRESENT_GLES2) {
        return "gles2";
    }
    return "software";
}

static SDL_Window *uya_gui_sim_create_window(int width, int height, int scale, int fullscreen, const uint8_t *title, uint32_t extra_flags) {
    uint32_t window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | extra_flags;
    if (fullscreen != 0) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    return SDL_CreateWindow(
        title != NULL ? (const char *)title : "UyaGUI Linux Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width * (scale > 0 ? scale : 1),
        height * (scale > 0 ? scale : 1),
        window_flags
    );
}

static void uya_gui_sim_destroy_renderer_resources(UyaGuiSimDisplay *display) {
    if (display->texture != NULL) {
        SDL_DestroyTexture(display->texture);
        display->texture = NULL;
    }
    if (display->renderer != NULL) {
        SDL_DestroyRenderer(display->renderer);
        display->renderer = NULL;
    }
}

static void uya_gui_sim_destroy_overlay_resources(UyaGuiSimDisplay *display) {
    if (display == NULL) {
        return;
    }
    if (display->overlay_pixels != NULL) {
        uya_gui_sim_host_free(display->overlay_pixels);
        display->overlay_pixels = NULL;
    }
    display->overlay_bytes = 0u;
    display->dirty_overlay_rect_count = 0;
    display->dirty_overlay_full = 0;
}

static void uya_gui_sim_destroy_gles2_resources(UyaGuiSimDisplay *display) {
    if (display == NULL) {
        return;
    }
    if (display->gl_context != NULL) {
        if (display->window != NULL) {
            (void)SDL_GL_MakeCurrent(display->window, display->gl_context);
        }
        if (display->gl.DeleteBuffers != NULL && display->gl_vbo != 0u) {
            display->gl.DeleteBuffers(1, &display->gl_vbo);
        }
        if (display->gl.DeleteTextures != NULL && display->gl_texture != 0u) {
            display->gl.DeleteTextures(1, &display->gl_texture);
        }
        if (display->gl.DeleteTextures != NULL && display->gl_white_texture != 0u) {
            display->gl.DeleteTextures(1, &display->gl_white_texture);
        }
        if (display->gl.DeleteProgram != NULL && display->gl_program != 0u) {
            display->gl.DeleteProgram(display->gl_program);
        }
        if (display->gl.DeleteProgram != NULL && display->gl_shape_program != 0u) {
            display->gl.DeleteProgram(display->gl_shape_program);
        }
        SDL_GL_DeleteContext(display->gl_context);
    }
    if (display->gl_rgba_pixels != NULL) {
        uya_gui_sim_host_free(display->gl_rgba_pixels);
        display->gl_rgba_pixels = NULL;
    }
    display->gl_rgba_bytes = 0u;
    display->gl_context = NULL;
    display->gl_program = 0u;
    display->gl_shape_program = 0u;
    display->gl_texture = 0u;
    display->gl_white_texture = 0u;
    display->gl_vbo = 0u;
    display->gl_attr_pos = -1;
    display->gl_attr_uv = -1;
    display->gl_uniform_tex = -1;
    display->gl_uniform_color = -1;
    display->gl_shape_attr_pos = -1;
    display->gl_shape_attr_uv = -1;
    display->gl_shape_uniform_color0 = -1;
    display->gl_shape_uniform_color1 = -1;
    display->gl_shape_uniform_params0 = -1;
    display->gl_shape_uniform_params1 = -1;
    (void)memset(&display->gl, 0, sizeof(display->gl));
}

static void uya_gui_sim_destroy_display_resources(UyaGuiSimDisplay *display) {
    if (display == NULL) {
        return;
    }
    uya_gui_sim_destroy_overlay_resources(display);
    uya_gui_sim_destroy_renderer_resources(display);
    uya_gui_sim_destroy_gles2_resources(display);
    if (display->window != NULL) {
        SDL_DestroyWindow(display->window);
        display->window = NULL;
    }
    display->present_kind = UYA_GUI_SIM_PRESENT_SOFTWARE;
    display->direct_frame_active = 0;
}

static int uya_gui_sim_overlay_ensure_capacity(UyaGuiSimDisplay *display, int32_t width, int32_t height) {
    const size_t needed = (size_t)width * (size_t)height * 4u;
    if (display->overlay_bytes >= needed && display->overlay_pixels != NULL) {
        return 1;
    }
    uint8_t *resized = (uint8_t *)uya_gui_sim_host_realloc(display->overlay_pixels, needed);
    if (resized == NULL) {
        uya_gui_sim_set_error("overlay realloc failed");
        return 0;
    }
    display->overlay_pixels = resized;
    display->overlay_bytes = needed;
    return 1;
}

static void uya_gui_sim_overlay_copy_argb8888(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height) {
    int32_t y = 0;
    while (y < height) {
        const uint8_t *src_row = pixels + (size_t)y * (size_t)pitch;
        uint8_t *dst_row = display->overlay_pixels + (size_t)y * (size_t)width * 4u;
        (void)memcpy(dst_row, src_row, (size_t)width * 4u);
        y += 1;
    }
}

static void uya_gui_sim_overlay_draw_pixel_argb8888(uint8_t *pixels, int32_t width, int32_t height, int32_t x, int32_t y) {
    if (pixels == NULL || x < 0 || y < 0 || x >= width || y >= height) {
        return;
    }
    uint8_t *dst = pixels + ((size_t)y * (size_t)width + (size_t)x) * 4u;
    dst[0] = 255u;
    dst[1] = 255u;
    dst[2] = 0u;
    dst[3] = 0u;
}

static void uya_gui_sim_overlay_draw_rect_argb8888(uint8_t *pixels, int32_t width, int32_t height, SDL_Rect rect) {
    int32_t x0 = rect.x;
    int32_t y0 = rect.y;
    int32_t x1 = rect.x + rect.w - 1;
    int32_t y1 = rect.y + rect.h - 1;
    int32_t x = 0;
    int32_t y = 0;

    if (rect.w <= 0 || rect.h <= 0) {
        return;
    }

    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 >= width) {
        x1 = width - 1;
    }
    if (y1 >= height) {
        y1 = height - 1;
    }
    if (x0 > x1 || y0 > y1) {
        return;
    }

    x = x0;
    while (x <= x1) {
        uya_gui_sim_overlay_draw_pixel_argb8888(pixels, width, height, x, y0);
        uya_gui_sim_overlay_draw_pixel_argb8888(pixels, width, height, x, y1);
        x += 1;
    }

    y = y0;
    while (y <= y1) {
        uya_gui_sim_overlay_draw_pixel_argb8888(pixels, width, height, x0, y);
        uya_gui_sim_overlay_draw_pixel_argb8888(pixels, width, height, x1, y);
        y += 1;
    }
}

static const uint8_t *uya_gui_sim_overlay_prepare(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height,
                                                  int32_t *out_pitch) {
    int32_t i = 0;
    if (out_pitch != NULL) {
        *out_pitch = pitch;
    }
    if (display == NULL || pixels == NULL || !display->dirty_overlay_enabled) {
        return pixels;
    }
    if (!uya_gui_sim_overlay_ensure_capacity(display, width, height)) {
        return pixels;
    }

    uya_gui_sim_overlay_copy_argb8888(display, pixels, pitch, width, height);
    if (display->dirty_overlay_full || display->dirty_overlay_rect_count <= 0) {
        SDL_Rect full = { 0, 0, width, height };
        uya_gui_sim_overlay_draw_rect_argb8888(display->overlay_pixels, width, height, full);
    } else {
        i = 0;
        while (i < display->dirty_overlay_rect_count) {
            uya_gui_sim_overlay_draw_rect_argb8888(display->overlay_pixels, width, height, display->dirty_overlay_rects[i]);
            i += 1;
        }
    }

    if (out_pitch != NULL) {
        *out_pitch = width * 4;
    }
    return display->overlay_pixels;
}

static int32_t uya_gui_sim_present_software(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch) {
    if (SDL_UpdateTexture(display->texture, NULL, pixels, pitch) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    if (SDL_RenderClear(display->renderer) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    if (SDL_RenderCopy(display->renderer, display->texture, NULL, NULL) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    SDL_RenderPresent(display->renderer);
    display->needs_refresh = 0;
    return 1;
}

static int uya_gui_sim_update_texture_region(UyaGuiSimDisplay *display,
                                             const SDL_Rect *rect,
                                             const uint8_t *pixels,
                                             int32_t pitch) {
    if (display == NULL || display->texture == NULL || rect == NULL || pixels == NULL) {
        uya_gui_sim_set_error("texture region update unavailable");
        return 0;
    }
    if (rect->w <= 0 || rect->h <= 0) {
        return 1;
    }
    if (SDL_UpdateTexture(display->texture, rect, pixels, pitch) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    return 1;
}

static void uya_gui_sim_convert_argb_to_rgba(uint8_t *dst, const uint8_t *src, int pitch, int width, int height) {
    int y = 0;
    while (y < height) {
        const uint8_t *src_row = src + (size_t)y * (size_t)pitch;
        uint8_t *dst_row = dst + (size_t)y * (size_t)width * 4u;
        int x = 0;
        while (x < width) {
            const uint8_t a = src_row[x * 4 + 0];
            const uint8_t r = src_row[x * 4 + 1];
            const uint8_t g = src_row[x * 4 + 2];
            const uint8_t b = src_row[x * 4 + 3];
            dst_row[x * 4 + 0] = r;
            dst_row[x * 4 + 1] = g;
            dst_row[x * 4 + 2] = b;
            dst_row[x * 4 + 3] = a;
            x += 1;
        }
        y += 1;
    }
}

static int uya_gui_sim_gles2_ensure_rgba_capacity(UyaGuiSimDisplay *display, size_t needed) {
    if (display->gl_rgba_bytes >= needed && display->gl_rgba_pixels != NULL) {
        return 1;
    }
    uint8_t *resized = (uint8_t *)uya_gui_sim_host_realloc(display->gl_rgba_pixels, needed);
    if (resized == NULL) {
        uya_gui_sim_set_error("gles2 scratch realloc failed");
        return 0;
    }
    display->gl_rgba_pixels = resized;
    display->gl_rgba_bytes = needed;
    return 1;
}

static void uya_gui_sim_argb_u32_to_rgba_f32(uint32_t argb, GLfloat *out_rgba) {
    out_rgba[0] = (GLfloat)((argb >> 16) & 0xFFu) / 255.0f;
    out_rgba[1] = (GLfloat)((argb >> 8) & 0xFFu) / 255.0f;
    out_rgba[2] = (GLfloat)(argb & 0xFFu) / 255.0f;
    out_rgba[3] = (GLfloat)((argb >> 24) & 0xFFu) / 255.0f;
}

static void uya_gui_sim_gles2_rect_vertices(const UyaGuiSimDisplay *display,
                                            float x0, float y0, float x1, float y1,
                                            float u0, float v0, float u1, float v1,
                                            GLfloat *out_vertices) {
    const float width = (float)(display->width > 0 ? display->width : 1);
    const float height = (float)(display->height > 0 ? display->height : 1);
    const float left = (x0 * 2.0f / width) - 1.0f;
    const float right = (x1 * 2.0f / width) - 1.0f;
    const float top = 1.0f - (y0 * 2.0f / height);
    const float bottom = 1.0f - (y1 * 2.0f / height);

    out_vertices[0] = left;   out_vertices[1] = top;    out_vertices[2] = u0; out_vertices[3] = v0;
    out_vertices[4] = left;   out_vertices[5] = bottom; out_vertices[6] = u0; out_vertices[7] = v1;
    out_vertices[8] = right;  out_vertices[9] = top;    out_vertices[10] = u1; out_vertices[11] = v0;
    out_vertices[12] = right; out_vertices[13] = bottom;out_vertices[14] = u1; out_vertices[15] = v1;
}

static int uya_gui_sim_gles2_draw_quad(UyaGuiSimDisplay *display, GLuint texture, const GLfloat *vertices, const GLfloat *rgba) {
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }

    display->gl.UseProgram(display->gl_program);
    display->gl.ActiveTexture(GL_TEXTURE0);
    display->gl.BindTexture(GL_TEXTURE_2D, texture);
    display->gl.Uniform1i(display->gl_uniform_tex, 0);
    display->gl.Uniform4f(display->gl_uniform_color, rgba[0], rgba[1], rgba[2], rgba[3]);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, display->gl_vbo);
    display->gl.BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(GLfloat) * 16), vertices, GL_DYNAMIC_DRAW);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_pos, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)0);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_uv, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)(sizeof(GLfloat) * 2));
    display->gl.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, 0u);
    return 1;
}

static int uya_gui_sim_gles2_draw_shape_quad(UyaGuiSimDisplay *display,
                                             const GLfloat *vertices,
                                             const GLfloat *color0,
                                             const GLfloat *color1,
                                             float size_x,
                                             float size_y,
                                             float radius,
                                             float stroke_width,
                                             float mode,
                                             float extra0,
                                             float extra1,
                                             float extra2) {
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }

    display->gl.UseProgram(display->gl_shape_program);
    display->gl.Uniform4f(display->gl_shape_uniform_color0, color0[0], color0[1], color0[2], color0[3]);
    display->gl.Uniform4f(display->gl_shape_uniform_color1, color1[0], color1[1], color1[2], color1[3]);
    display->gl.Uniform4f(display->gl_shape_uniform_params0, size_x, size_y, radius, stroke_width);
    display->gl.Uniform4f(display->gl_shape_uniform_params1, mode, extra0, extra1, extra2);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, display->gl_vbo);
    display->gl.BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(GLfloat) * 16), vertices, GL_DYNAMIC_DRAW);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_shape_attr_pos);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_shape_attr_uv);
    display->gl.VertexAttribPointer((GLuint)display->gl_shape_attr_pos, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)0);
    display->gl.VertexAttribPointer((GLuint)display->gl_shape_attr_uv, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)(sizeof(GLfloat) * 2));
    display->gl.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_shape_attr_pos);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_shape_attr_uv);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, 0u);
    return 1;
}

static void uya_gui_sim_gles2_drawable_size(const UyaGuiSimDisplay *display, int *out_w, int *out_h) {
    int drawable_w = 0;
    int drawable_h = 0;
    if (display != NULL && display->window != NULL) {
        SDL_GL_GetDrawableSize(display->window, &drawable_w, &drawable_h);
    }
    if (drawable_w <= 0) {
        drawable_w = display != NULL ? display->width * display->scale : 0;
    }
    if (drawable_h <= 0) {
        drawable_h = display != NULL ? display->height * display->scale : 0;
    }
    if (out_w != NULL) {
        *out_w = drawable_w;
    }
    if (out_h != NULL) {
        *out_h = drawable_h;
    }
}

static void uya_gui_sim_gles2_apply_scissor(UyaGuiSimDisplay *display, int32_t x, int32_t y, int32_t w, int32_t h) {
    int drawable_w = 0;
    int drawable_h = 0;
    int scissor_x = 0;
    int scissor_y = 0;
    int scissor_w = 0;
    int scissor_h = 0;
    if (display == NULL || display->gl_context == NULL) {
        return;
    }
    uya_gui_sim_gles2_drawable_size(display, &drawable_w, &drawable_h);
    if (drawable_w <= 0 || drawable_h <= 0 || display->width <= 0 || display->height <= 0) {
        display->gl.Scissor(0, 0, 0, 0);
        return;
    }
    scissor_x = (x * drawable_w) / display->width;
    scissor_y = ((display->height - (y + h)) * drawable_h) / display->height;
    scissor_w = ((x + w) * drawable_w) / display->width - scissor_x;
    scissor_h = ((y + h) * drawable_h) / display->height - (y * drawable_h) / display->height;
    if (scissor_w < 0) {
        scissor_w = 0;
    }
    if (scissor_h < 0) {
        scissor_h = 0;
    }
    display->gl.Scissor(scissor_x, scissor_y, scissor_w, scissor_h);
}

static void uya_gui_sim_gles2_reset_full_clip(UyaGuiSimDisplay *display) {
    if (display == NULL || display->gl_context == NULL) {
        return;
    }
    uya_gui_sim_gles2_apply_scissor(display, 0, 0, display->width, display->height);
}

static int uya_gui_sim_unpack_image_pixel_rgba(uint8_t *dst_rgba, const uint8_t *src_row, int format_tag, int x) {
    switch (format_tag) {
        case 0: {
            const uint16_t bits = (uint16_t)(src_row[x * 2 + 0] | ((uint16_t)src_row[x * 2 + 1] << 8));
            const uint8_t r5 = (uint8_t)((bits >> 11) & 0x1Fu);
            const uint8_t g6 = (uint8_t)((bits >> 5) & 0x3Fu);
            const uint8_t b5 = (uint8_t)(bits & 0x1Fu);
            dst_rgba[0] = (uint8_t)((r5 << 3) | (r5 >> 2));
            dst_rgba[1] = (uint8_t)((g6 << 2) | (g6 >> 4));
            dst_rgba[2] = (uint8_t)((b5 << 3) | (b5 >> 2));
            dst_rgba[3] = 255u;
            return 1;
        }
        case 1:
            dst_rgba[0] = src_row[x * 3 + 0];
            dst_rgba[1] = src_row[x * 3 + 1];
            dst_rgba[2] = src_row[x * 3 + 2];
            dst_rgba[3] = 255u;
            return 1;
        case 2:
            dst_rgba[0] = src_row[x * 4 + 1];
            dst_rgba[1] = src_row[x * 4 + 2];
            dst_rgba[2] = src_row[x * 4 + 3];
            dst_rgba[3] = src_row[x * 4 + 0];
            return 1;
        case 3: {
            const uint16_t bits = (uint16_t)(src_row[x * 2 + 0] | ((uint16_t)src_row[x * 2 + 1] << 8));
            const uint8_t a4 = (uint8_t)((bits >> 12) & 0x0Fu);
            const uint8_t r4 = (uint8_t)((bits >> 8) & 0x0Fu);
            const uint8_t g4 = (uint8_t)((bits >> 4) & 0x0Fu);
            const uint8_t b4 = (uint8_t)(bits & 0x0Fu);
            dst_rgba[0] = (uint8_t)(r4 * 17u);
            dst_rgba[1] = (uint8_t)(g4 * 17u);
            dst_rgba[2] = (uint8_t)(b4 * 17u);
            dst_rgba[3] = (uint8_t)(a4 * 17u);
            return 1;
        }
        case 4:
            dst_rgba[0] = src_row[x];
            dst_rgba[1] = src_row[x];
            dst_rgba[2] = src_row[x];
            dst_rgba[3] = 255u;
            return 1;
        case 5:
            dst_rgba[0] = 255u;
            dst_rgba[1] = 255u;
            dst_rgba[2] = 255u;
            dst_rgba[3] = src_row[x];
            return 1;
        case 6: {
            const uint8_t byte = src_row[x / 8];
            const uint8_t mask = (uint8_t)(0x80u >> (x % 8));
            const uint8_t gray = (byte & mask) != 0 ? 255u : 0u;
            dst_rgba[0] = gray;
            dst_rgba[1] = gray;
            dst_rgba[2] = gray;
            dst_rgba[3] = 255u;
            return 1;
        }
        case 7: {
            const uint8_t packed = src_row[x / 2];
            const uint8_t gray4 = (x % 2) == 0 ? (uint8_t)(packed >> 4) : (uint8_t)(packed & 0x0Fu);
            const uint8_t gray = (uint8_t)(gray4 * 17u);
            dst_rgba[0] = gray;
            dst_rgba[1] = gray;
            dst_rgba[2] = gray;
            dst_rgba[3] = 255u;
            return 1;
        }
        default:
            return 0;
    }
}

static int uya_gui_sim_convert_image_region_to_rgba(UyaGuiSimDisplay *display,
                                                    const uint8_t *pixels,
                                                    int32_t stride,
                                                    int32_t format_tag,
                                                    int32_t src_x,
                                                    int32_t src_y,
                                                    int32_t width,
                                                    int32_t height) {
    size_t needed = (size_t)width * (size_t)height * 4u;
    int32_t y = 0;
    if (!uya_gui_sim_gles2_ensure_rgba_capacity(display, needed)) {
        return 0;
    }
    while (y < height) {
        const uint8_t *src_row = pixels + (size_t)(src_y + y) * (size_t)stride;
        uint8_t *dst_row = display->gl_rgba_pixels + (size_t)y * (size_t)width * 4u;
        int32_t x = 0;
        while (x < width) {
            if (!uya_gui_sim_unpack_image_pixel_rgba(&dst_row[x * 4], src_row, format_tag, src_x + x)) {
                uya_gui_sim_set_error("unsupported image pixel format for gles2");
                return 0;
            }
            x += 1;
        }
        y += 1;
    }
    return 1;
}

static void uya_gui_sim_color_cmd_to_rgba_f32(const UyaGuiColorCmd *color, GLfloat *out_rgba) {
    out_rgba[0] = (GLfloat)color->r / 255.0f;
    out_rgba[1] = (GLfloat)color->g / 255.0f;
    out_rgba[2] = (GLfloat)color->b / 255.0f;
    out_rgba[3] = (GLfloat)color->a / 255.0f;
}

static float uya_gui_sim_gles2_ndc_x(const UyaGuiSimDisplay *display, float x) {
    const float width = (float)(display->width > 0 ? display->width : 1);
    return (x * 2.0f / width) - 1.0f;
}

static float uya_gui_sim_gles2_ndc_y(const UyaGuiSimDisplay *display, float y) {
    const float height = (float)(display->height > 0 ? display->height : 1);
    return 1.0f - (y * 2.0f / height);
}

static int uya_gui_sim_gles2_draw_solid_rect(UyaGuiSimDisplay *display, const UyaGuiRectCmd *rect, const UyaGuiColorCmd *color) {
    GLfloat vertices[16];
    GLfloat rgba[4];
    uya_gui_sim_color_cmd_to_rgba_f32(color, rgba);
    uya_gui_sim_gles2_rect_vertices(
        display,
        (float)rect->x,
        (float)rect->y,
        (float)rect->x + (float)rect->w,
        (float)rect->y + (float)rect->h,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        vertices
    );
    return uya_gui_sim_gles2_draw_quad(display, display->gl_white_texture, vertices, rgba);
}

static int uya_gui_sim_gles2_draw_shape_rect(UyaGuiSimDisplay *display,
                                             const UyaGuiRectCmd *rect,
                                             const UyaGuiColorCmd *color0,
                                             const UyaGuiColorCmd *color1,
                                             float radius,
                                             float stroke_width,
                                             float mode,
                                             float extra0,
                                             float extra1,
                                             float extra2) {
    GLfloat vertices[16];
    GLfloat rgba0[4];
    GLfloat rgba1[4];
    uya_gui_sim_color_cmd_to_rgba_f32(color0, rgba0);
    uya_gui_sim_color_cmd_to_rgba_f32(color1, rgba1);
    uya_gui_sim_gles2_rect_vertices(
        display,
        (float)rect->x,
        (float)rect->y,
        (float)rect->x + (float)rect->w,
        (float)rect->y + (float)rect->h,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        vertices
    );
    return uya_gui_sim_gles2_draw_shape_quad(
        display,
        vertices,
        rgba0,
        rgba1,
        (float)rect->w,
        (float)rect->h,
        radius,
        stroke_width,
        mode,
        extra0,
        extra1,
        extra2
    );
}

static int uya_gui_sim_gles2_draw_line(UyaGuiSimDisplay *display, const UyaGuiGpuLineCmd *cmd) {
    GLfloat vertices[16];
    GLfloat rgba[4];
    const float x1 = (float)cmd->x1;
    const float y1 = (float)cmd->y1;
    const float x2 = (float)cmd->x2;
    const float y2 = (float)cmd->y2;
    const float width = (float)(cmd->width > 0 ? cmd->width : 1);
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    float nx = 0.0f;
    float ny = 0.0f;
    float len = SDL_sqrtf(dx * dx + dy * dy);
    uya_gui_sim_color_cmd_to_rgba_f32(&cmd->color, rgba);

    if (len <= 0.001f) {
        UyaGuiRectCmd rect;
        rect.x = (int16_t)(cmd->x1 - (cmd->width / 2));
        rect.y = (int16_t)(cmd->y1 - (cmd->width / 2));
        rect.w = (uint16_t)(cmd->width > 0 ? cmd->width : 1);
        rect.h = (uint16_t)(cmd->width > 0 ? cmd->width : 1);
        return uya_gui_sim_gles2_draw_solid_rect(display, &rect, &cmd->color);
    }

    nx = -dy / len * (width * 0.5f);
    ny = dx / len * (width * 0.5f);

    vertices[0] = uya_gui_sim_gles2_ndc_x(display, x1 + nx);
    vertices[1] = uya_gui_sim_gles2_ndc_y(display, y1 + ny);
    vertices[2] = 0.0f;
    vertices[3] = 0.0f;
    vertices[4] = uya_gui_sim_gles2_ndc_x(display, x1 - nx);
    vertices[5] = uya_gui_sim_gles2_ndc_y(display, y1 - ny);
    vertices[6] = 0.0f;
    vertices[7] = 1.0f;
    vertices[8] = uya_gui_sim_gles2_ndc_x(display, x2 + nx);
    vertices[9] = uya_gui_sim_gles2_ndc_y(display, y2 + ny);
    vertices[10] = 1.0f;
    vertices[11] = 0.0f;
    vertices[12] = uya_gui_sim_gles2_ndc_x(display, x2 - nx);
    vertices[13] = uya_gui_sim_gles2_ndc_y(display, y2 - ny);
    vertices[14] = 1.0f;
    vertices[15] = 1.0f;
    return uya_gui_sim_gles2_draw_quad(display, display->gl_white_texture, vertices, rgba);
}

static int uya_gui_sim_gles2_draw_image_region(UyaGuiSimDisplay *display,
                                               const uint8_t *pixels,
                                               int32_t stride,
                                               int32_t format_tag,
                                               int32_t src_x,
                                               int32_t src_y,
                                               int32_t src_w,
                                               int32_t src_h,
                                               const UyaGuiRectCmd *dst_rect) {
    GLfloat vertices[16];
    static const GLfloat rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    if (src_w <= 0 || src_h <= 0) {
        return 1;
    }
    if (src_w > display->width || src_h > display->height) {
        uya_gui_sim_set_error("gles2 image region exceeds scratch texture");
        return 0;
    }
    if (!uya_gui_sim_convert_image_region_to_rgba(display, pixels, stride, format_tag, src_x, src_y, src_w, src_h)) {
        return 0;
    }
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    display->gl.BindTexture(GL_TEXTURE_2D, display->gl_texture);
    display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    display->gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src_w, src_h, GL_RGBA, GL_UNSIGNED_BYTE, display->gl_rgba_pixels);
    uya_gui_sim_gles2_rect_vertices(
        display,
        (float)dst_rect->x,
        (float)dst_rect->y,
        (float)dst_rect->x + (float)dst_rect->w,
        (float)dst_rect->y + (float)dst_rect->h,
        0.0f,
        0.0f,
        (float)src_w / (float)display->width,
        (float)src_h / (float)display->height,
        vertices
    );
    return uya_gui_sim_gles2_draw_quad(display, display->gl_texture, vertices, rgba);
}

static uint8_t *g_glyph_atlas_pixels = NULL;
static size_t g_glyph_atlas_bytes = 0;

static int uya_gui_sim_ensure_glyph_atlas_capacity(size_t needed) {
    if (g_glyph_atlas_bytes >= needed && g_glyph_atlas_pixels != NULL) {
        return 1;
    }
    uint8_t *resized = (uint8_t *)uya_gui_sim_host_realloc(g_glyph_atlas_pixels, needed);
    if (resized == NULL) {
        uya_gui_sim_set_error("glyph atlas realloc failed");
        return 0;
    }
    g_glyph_atlas_pixels = resized;
    g_glyph_atlas_bytes = needed;
    return 1;
}

typedef struct {
    int32_t atlas_x;
    int32_t atlas_y;
    int32_t glyph_w;
    int32_t glyph_h;
} GlyphAtlasEntry;

static int uya_gui_sim_gles2_draw_glyph_batch(UyaGuiSimDisplay *display, const UyaGuiGpuGlyphCmd *cmds, int32_t count) {
    GlyphAtlasEntry *entries = NULL;
    GLfloat *vertices = NULL;
    int32_t atlas_x = 0;
    int32_t atlas_y = 0;
    int32_t row_h = 0;
    int32_t atlas_w = 0;
    int32_t atlas_h = 0;
    int32_t valid_count = 0;
    int32_t i = 0;
    GLfloat color_rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    if (count <= 0) {
        return 1;
    }

    entries = (GlyphAtlasEntry *)malloc(sizeof(GlyphAtlasEntry) * count);
    if (entries == NULL) {
        uya_gui_sim_set_error("glyph atlas entries alloc failed");
        return 0;
    }

    // 计算 atlas 排列
    for (i = 0; i < count; i++) {
        const UyaGuiGpuGlyphCmd *cmd = &cmds[i];
        if (cmd->pixels == NULL || cmd->glyph_w <= 0 || cmd->glyph_h <= 0 || cmd->stride <= 0) {
            entries[i].atlas_x = -1;
            entries[i].atlas_y = -1;
            entries[i].glyph_w = 0;
            entries[i].glyph_h = 0;
            continue;
        }
        if (cmd->glyph_w > display->width || cmd->glyph_h > display->height) {
            free(entries);
            uya_gui_sim_set_error("gles2 glyph region exceeds scratch texture");
            return 0;
        }
        if (cmd->format_tag != 5) {
            free(entries);
            uya_gui_sim_set_error("unsupported glyph pixel format for gles2");
            return 0;
        }
        if (atlas_x + cmd->glyph_w > display->width) {
            atlas_y += row_h;
            atlas_x = 0;
            row_h = 0;
        }
        if (atlas_y + cmd->glyph_h > display->height) {
            // atlas 满了，绘制已累积的 batch 后重新开始
            free(entries);
            uya_gui_sim_set_error("gles2 glyph atlas overflow");
            return 0;
        }
        entries[i].atlas_x = atlas_x;
        entries[i].atlas_y = atlas_y;
        entries[i].glyph_w = cmd->glyph_w;
        entries[i].glyph_h = cmd->glyph_h;
        atlas_x += cmd->glyph_w;
        if (row_h < cmd->glyph_h) row_h = cmd->glyph_h;
        if (atlas_w < atlas_x) atlas_w = atlas_x;
        valid_count++;
    }

    if (valid_count == 0) {
        free(entries);
        return 1;
    }

    atlas_h = atlas_y + row_h;

    // 分配 atlas buffer 并清零
    size_t atlas_pixels_needed = (size_t)atlas_w * (size_t)atlas_h * 4u;
    if (!uya_gui_sim_ensure_glyph_atlas_capacity(atlas_pixels_needed)) {
        free(entries);
        return 0;
    }
    memset(g_glyph_atlas_pixels, 0, atlas_pixels_needed);

    // 拷贝 A8→RGBA 数据到 atlas buffer
    for (i = 0; i < count; i++) {
        const UyaGuiGpuGlyphCmd *cmd = &cmds[i];
        if (entries[i].atlas_x < 0) continue;

        int32_t gw = cmd->glyph_w;
        int32_t gh = cmd->glyph_h;
        int32_t gx = entries[i].atlas_x;
        int32_t gy = entries[i].atlas_y;
        int32_t y;

        for (y = 0; y < gh; y++) {
            const uint8_t *src_row = cmd->pixels + (size_t)y * (size_t)cmd->stride;
            uint8_t *dst_row = g_glyph_atlas_pixels + ((size_t)(gy + y) * (size_t)atlas_w + (size_t)gx) * 4u;
            int32_t x;
            for (x = 0; x < gw; x++) {
                uint8_t alpha = src_row[x];
                dst_row[x * 4 + 0] = 255u;
                dst_row[x * 4 + 1] = 255u;
                dst_row[x * 4 + 2] = 255u;
                dst_row[x * 4 + 3] = alpha;
            }
        }

        // 记录第一个有效 glyph 的颜色作为统一 tint
        if (color_rgba[3] == 1.0f && i == 0) {
            uya_gui_sim_color_cmd_to_rgba_f32(&cmd->color, color_rgba);
        }
    }

    // 准备顶点数组：每个 glyph 6 个顶点（2 triangles）
    vertices = (GLfloat *)malloc(sizeof(GLfloat) * 4 * 6 * valid_count);
    if (vertices == NULL) {
        free(entries);
        uya_gui_sim_set_error("glyph vertices alloc failed");
        return 0;
    }

    {
        GLfloat *v = vertices;
        for (i = 0; i < count; i++) {
            const UyaGuiGpuGlyphCmd *cmd = &cmds[i];
            if (entries[i].atlas_x < 0) continue;

            float x0 = (float)cmd->dst_rect.x;
            float y0 = (float)cmd->dst_rect.y;
            float x1 = x0 + (float)cmd->dst_rect.w;
            float y1 = y0 + (float)cmd->dst_rect.h;

            float u0 = (float)entries[i].atlas_x / (float)display->width;
            float v0 = (float)entries[i].atlas_y / (float)display->height;
            float u1 = (float)(entries[i].atlas_x + cmd->glyph_w) / (float)display->width;
            float v1 = (float)(entries[i].atlas_y + cmd->glyph_h) / (float)display->height;

            float left = (x0 * 2.0f / (float)display->width) - 1.0f;
            float right = (x1 * 2.0f / (float)display->width) - 1.0f;
            float top = 1.0f - (y0 * 2.0f / (float)display->height);
            float bottom = 1.0f - (y1 * 2.0f / (float)display->height);

            // triangle 1
            v[0]  = left;  v[1]  = top;    v[2]  = u0; v[3]  = v0;
            v[4]  = left;  v[5]  = bottom; v[6]  = u0; v[7]  = v1;
            v[8]  = right; v[9]  = top;    v[10] = u1; v[11] = v0;
            // triangle 2
            v[12] = right; v[13] = top;    v[14] = u1; v[15] = v0;
            v[16] = left;  v[17] = bottom; v[18] = u0; v[19] = v1;
            v[20] = right; v[21] = bottom; v[22] = u1; v[23] = v1;
            v += 24;
        }
    }

    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        free(entries);
        free(vertices);
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }

    display->gl.BindTexture(GL_TEXTURE_2D, display->gl_texture);
    display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    display->gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas_w, atlas_h, GL_RGBA, GL_UNSIGNED_BYTE, g_glyph_atlas_pixels);

    display->gl.UseProgram(display->gl_program);
    display->gl.ActiveTexture(GL_TEXTURE0);
    display->gl.Uniform1i(display->gl_uniform_tex, 0);
    display->gl.Uniform4f(display->gl_uniform_color, color_rgba[0], color_rgba[1], color_rgba[2], color_rgba[3]);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, display->gl_vbo);
    display->gl.BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(GLfloat) * 4 * 6 * valid_count), vertices, GL_DYNAMIC_DRAW);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_pos, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)0);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_uv, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)(sizeof(GLfloat) * 2));
    display->gl.DrawArrays(GL_TRIANGLES, 0, valid_count * 6);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, 0u);

    free(entries);
    free(vertices);
    return 1;
}

static int uya_gui_sim_gles2_present_region(UyaGuiSimDisplay *display,
                                            const uint8_t *pixels,
                                            int32_t stride,
                                            int32_t format_tag,
                                            int32_t src_x,
                                            int32_t src_y,
                                            int32_t src_w,
                                            int32_t src_h,
                                            const UyaGuiRectCmd *dst_rect) {
    GLfloat vertices[16];
    static const GLfloat rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    if (src_w <= 0 || src_h <= 0) {
        return 1;
    }
    if (src_x < 0 || src_y < 0 || src_x + src_w > display->width || src_y + src_h > display->height) {
        uya_gui_sim_set_error("gles2 present region exceeds texture bounds");
        return 0;
    }
    if (!uya_gui_sim_convert_image_region_to_rgba(display, pixels, stride, format_tag, src_x, src_y, src_w, src_h)) {
        return 0;
    }
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    display->gl.BindTexture(GL_TEXTURE_2D, display->gl_texture);
    display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    display->gl.TexSubImage2D(GL_TEXTURE_2D, 0, src_x, src_y, src_w, src_h, GL_RGBA, GL_UNSIGNED_BYTE, display->gl_rgba_pixels);
    uya_gui_sim_gles2_rect_vertices(
        display,
        (float)dst_rect->x,
        (float)dst_rect->y,
        (float)dst_rect->x + (float)dst_rect->w,
        (float)dst_rect->y + (float)dst_rect->h,
        (float)src_x / (float)display->width,
        (float)src_y / (float)display->height,
        (float)(src_x + src_w) / (float)display->width,
        (float)(src_y + src_h) / (float)display->height,
        vertices
    );
    return uya_gui_sim_gles2_draw_quad(display, display->gl_texture, vertices, rgba);
}

static int uya_gui_sim_load_gles2_proc(void **slot, const char *name) {
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    *slot = SDL_GL_GetProcAddress(name);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    if (*slot == NULL) {
        uya_gui_sim_set_error(name);
        return 0;
    }
    return 1;
}

static int uya_gui_sim_load_gles2_functions(UyaGuiSimDisplay *display) {
    if (!uya_gui_sim_load_gles2_proc((void **)&display->gl.ActiveTexture, "glActiveTexture")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.AttachShader, "glAttachShader")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.BindBuffer, "glBindBuffer")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.BindTexture, "glBindTexture")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.BlendFunc, "glBlendFunc")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.BufferData, "glBufferData")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Clear, "glClear")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.ClearColor, "glClearColor")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.CompileShader, "glCompileShader")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.CreateProgram, "glCreateProgram")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.CreateShader, "glCreateShader")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Disable, "glDisable")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DeleteBuffers, "glDeleteBuffers")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DeleteProgram, "glDeleteProgram")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DeleteShader, "glDeleteShader")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DeleteTextures, "glDeleteTextures")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DisableVertexAttribArray, "glDisableVertexAttribArray")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DrawArrays, "glDrawArrays")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Enable, "glEnable")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.EnableVertexAttribArray, "glEnableVertexAttribArray")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GenBuffers, "glGenBuffers")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GenTextures, "glGenTextures")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetAttribLocation, "glGetAttribLocation")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetProgramInfoLog, "glGetProgramInfoLog")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetProgramiv, "glGetProgramiv")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetShaderInfoLog, "glGetShaderInfoLog")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetShaderiv, "glGetShaderiv")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetUniformLocation, "glGetUniformLocation")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.LinkProgram, "glLinkProgram")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.PixelStorei, "glPixelStorei")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.ShaderSource, "glShaderSource")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.TexImage2D, "glTexImage2D")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.TexParameteri, "glTexParameteri")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.TexSubImage2D, "glTexSubImage2D")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Uniform1i, "glUniform1i")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Uniform4f, "glUniform4f")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.UseProgram, "glUseProgram")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.VertexAttribPointer, "glVertexAttribPointer")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Viewport, "glViewport")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Scissor, "glScissor")) {
        return 0;
    }
    return 1;
}

static int uya_gui_sim_compile_gles2_shader(UyaGuiSimDisplay *display, GLenum type, const char *source, GLuint *out_shader) {
    GLint ok = 0;
    GLuint shader = display->gl.CreateShader(type);
    if (shader == 0u) {
        uya_gui_sim_set_error("glCreateShader failed");
        return 0;
    }
    display->gl.ShaderSource(shader, 1, (const GLchar *const *)&source, NULL);
    display->gl.CompileShader(shader);
    display->gl.GetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == 0) {
        GLchar info[512];
        GLsizei info_len = 0;
        info[0] = '\0';
        display->gl.GetShaderInfoLog(shader, (GLsizei)sizeof(info), &info_len, info);
        display->gl.DeleteShader(shader);
        uya_gui_sim_set_error(info_len > 0 ? (const char *)info : "glCompileShader failed");
        return 0;
    }
    *out_shader = shader;
    return 1;
}

static int uya_gui_sim_init_gles2_pipeline(UyaGuiSimDisplay *display) {
    static const char *k_vertex_shader =
        "attribute vec2 a_pos;\n"
        "attribute vec2 a_uv;\n"
        "varying vec2 v_uv;\n"
        "void main() {\n"
        "  gl_Position = vec4(a_pos, 0.0, 1.0);\n"
        "  v_uv = a_uv;\n"
        "}\n";
    static const char *k_fragment_shader =
        "precision mediump float;\n"
        "varying vec2 v_uv;\n"
        "uniform sampler2D u_tex;\n"
        "uniform vec4 u_color;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(u_tex, v_uv) * u_color;\n"
        "}\n";
    static const char *k_shape_fragment_shader =
        "precision mediump float;\n"
        "varying vec2 v_uv;\n"
        "uniform vec4 u_color0;\n"
        "uniform vec4 u_color1;\n"
        "uniform vec4 u_params0;\n"
        "uniform vec4 u_params1;\n"
        "float sd_round_rect(vec2 p, vec2 b, float r) {\n"
        "  vec2 q = abs(p) - b + vec2(r, r);\n"
        "  return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;\n"
        "}\n"
        "float arc_contains(float angle, float start_deg, float end_deg) {\n"
        "  if (start_deg <= end_deg) {\n"
        "    return angle >= start_deg && angle <= end_deg ? 1.0 : 0.0;\n"
        "  }\n"
        "  return angle >= start_deg || angle <= end_deg ? 1.0 : 0.0;\n"
        "}\n"
        "void main() {\n"
        "  vec2 size = u_params0.xy;\n"
        "  float radius = max(u_params0.z, 0.0);\n"
        "  float param_w = max(u_params0.w, 0.0);\n"
        "  float mode = u_params1.x;\n"
        "  float extra0 = u_params1.y;\n"
        "  float extra1 = u_params1.z;\n"
        "  float extra2 = u_params1.w;\n"
        "  vec2 local = v_uv * size;\n"
        "  vec2 center = size * 0.5;\n"
        "  vec2 half_size = size * 0.5;\n"
        "  float max_radius = min(half_size.x, half_size.y);\n"
        "  if (radius > max_radius) radius = max_radius;\n"
        "  vec4 base = u_color0;\n"
        "  float alpha = 1.0;\n"
        "  if (mode > 0.5 && mode < 1.5) {\n"
        "    float t = extra0 > 0.5 ? v_uv.x : v_uv.y;\n"
        "    base = mix(u_color0, u_color1, clamp(t, 0.0, 1.0));\n"
        "  } else if (mode > 1.5) {\n"
        "    if (mode < 2.5) {\n"
        "      float rr = max(min(size.x, size.y) * 0.5, 1.0);\n"
        "      float t = clamp(length(local - center) / rr, 0.0, 1.0);\n"
        "      base = mix(u_color0, u_color1, t);\n"
        "    } else if (mode < 3.5) {\n"
        "      vec2 inner_half = max(half_size - vec2(param_w), vec2(0.0));\n"
        "      float softness = param_w + 0.5;\n"
        "      float dist = sd_round_rect(local - center, inner_half, radius);\n"
        "      float t = clamp((softness - dist) / softness, 0.0, 1.0);\n"
        "      alpha = t * t * (3.0 - 2.0 * t);\n"
        "    } else {\n"
        "      float dist = length(local - center) - radius;\n"
        "      if (mode < 4.5) {\n"
        "        float half_stroke = 0.5;\n"
        "        alpha = 1.0 - smoothstep(half_stroke - 1.0, half_stroke + 1.0, abs(dist));\n"
        "      } else if (mode < 5.5) {\n"
        "        alpha = 1.0 - smoothstep(0.0, 1.0, dist);\n"
        "      } else {\n"
        "        float angle = degrees(atan(local.y - center.y, local.x - center.x));\n"
        "        if (angle < 0.0) angle += 360.0;\n"
        "        float in_arc = arc_contains(angle, extra0, extra1);\n"
        "        if (mode < 6.5) {\n"
        "          float half_stroke = max(param_w * 0.5, 0.5);\n"
          "          alpha = (1.0 - smoothstep(half_stroke - 1.0, half_stroke + 1.0, abs(dist))) * in_arc;\n"
        "        } else {\n"
        "          alpha = (1.0 - smoothstep(0.0, 1.0, dist)) * in_arc;\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  } else {\n"
        "    float dist = sd_round_rect(local - center, half_size, radius);\n"
        "    if (param_w > 0.0) {\n"
        "      float half_stroke = param_w * 0.5;\n"
        "      alpha = 1.0 - smoothstep(half_stroke - 1.0, half_stroke + 1.0, abs(dist));\n"
        "    } else {\n"
        "      alpha = 1.0 - smoothstep(0.0, 1.0, dist);\n"
        "    }\n"
        "  }\n"
        "  gl_FragColor = vec4(base.rgb, base.a * alpha);\n"
        "}\n";
    static const GLfloat k_quad_vertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
    };
    GLint ok = 0;
    size_t rgba_bytes = (size_t)display->width * (size_t)display->height * 4u;
    GLuint vertex_shader = 0u;
    GLuint fragment_shader = 0u;
    GLuint shape_vertex_shader = 0u;
    GLuint shape_fragment_shader = 0u;

    if (!uya_gui_sim_load_gles2_functions(display)) {
        return 0;
    }
    if (!uya_gui_sim_compile_gles2_shader(display, GL_VERTEX_SHADER, k_vertex_shader, &vertex_shader)) {
        return 0;
    }
    if (!uya_gui_sim_compile_gles2_shader(display, GL_FRAGMENT_SHADER, k_fragment_shader, &fragment_shader)) {
        display->gl.DeleteShader(vertex_shader);
        return 0;
    }

    display->gl_program = display->gl.CreateProgram();
    if (display->gl_program == 0u) {
        display->gl.DeleteShader(vertex_shader);
        display->gl.DeleteShader(fragment_shader);
        uya_gui_sim_set_error("glCreateProgram failed");
        return 0;
    }
    display->gl.AttachShader(display->gl_program, vertex_shader);
    display->gl.AttachShader(display->gl_program, fragment_shader);
    display->gl.LinkProgram(display->gl_program);
    display->gl.GetProgramiv(display->gl_program, GL_LINK_STATUS, &ok);
    display->gl.DeleteShader(vertex_shader);
    display->gl.DeleteShader(fragment_shader);
    if (ok == 0) {
        GLchar info[512];
        GLsizei info_len = 0;
        info[0] = '\0';
        display->gl.GetProgramInfoLog(display->gl_program, (GLsizei)sizeof(info), &info_len, info);
        uya_gui_sim_set_error(info_len > 0 ? (const char *)info : "glLinkProgram failed");
        return 0;
    }

    display->gl_attr_pos = display->gl.GetAttribLocation(display->gl_program, "a_pos");
    display->gl_attr_uv = display->gl.GetAttribLocation(display->gl_program, "a_uv");
    display->gl_uniform_tex = display->gl.GetUniformLocation(display->gl_program, "u_tex");
    display->gl_uniform_color = display->gl.GetUniformLocation(display->gl_program, "u_color");
    if (display->gl_attr_pos < 0 || display->gl_attr_uv < 0 || display->gl_uniform_tex < 0 || display->gl_uniform_color < 0) {
        uya_gui_sim_set_error("gles2 shader attribute lookup failed");
        return 0;
    }

    if (!uya_gui_sim_compile_gles2_shader(display, GL_VERTEX_SHADER, k_vertex_shader, &shape_vertex_shader)) {
        return 0;
    }
    if (!uya_gui_sim_compile_gles2_shader(display, GL_FRAGMENT_SHADER, k_shape_fragment_shader, &shape_fragment_shader)) {
        display->gl.DeleteShader(shape_vertex_shader);
        return 0;
    }

    display->gl_shape_program = display->gl.CreateProgram();
    if (display->gl_shape_program == 0u) {
        display->gl.DeleteShader(shape_vertex_shader);
        display->gl.DeleteShader(shape_fragment_shader);
        uya_gui_sim_set_error("glCreateProgram shape failed");
        return 0;
    }
    display->gl.AttachShader(display->gl_shape_program, shape_vertex_shader);
    display->gl.AttachShader(display->gl_shape_program, shape_fragment_shader);
    display->gl.LinkProgram(display->gl_shape_program);
    display->gl.GetProgramiv(display->gl_shape_program, GL_LINK_STATUS, &ok);
    display->gl.DeleteShader(shape_vertex_shader);
    display->gl.DeleteShader(shape_fragment_shader);
    if (ok == 0) {
        GLchar info[512];
        GLsizei info_len = 0;
        info[0] = '\0';
        display->gl.GetProgramInfoLog(display->gl_shape_program, (GLsizei)sizeof(info), &info_len, info);
        uya_gui_sim_set_error(info_len > 0 ? (const char *)info : "glLinkProgram shape failed");
        return 0;
    }

    display->gl_shape_attr_pos = display->gl.GetAttribLocation(display->gl_shape_program, "a_pos");
    display->gl_shape_attr_uv = display->gl.GetAttribLocation(display->gl_shape_program, "a_uv");
    display->gl_shape_uniform_color0 = display->gl.GetUniformLocation(display->gl_shape_program, "u_color0");
    display->gl_shape_uniform_color1 = display->gl.GetUniformLocation(display->gl_shape_program, "u_color1");
    display->gl_shape_uniform_params0 = display->gl.GetUniformLocation(display->gl_shape_program, "u_params0");
    display->gl_shape_uniform_params1 = display->gl.GetUniformLocation(display->gl_shape_program, "u_params1");
    if (display->gl_shape_attr_pos < 0
        || display->gl_shape_attr_uv < 0
        || display->gl_shape_uniform_color0 < 0
        || display->gl_shape_uniform_color1 < 0
        || display->gl_shape_uniform_params0 < 0
        || display->gl_shape_uniform_params1 < 0) {
        uya_gui_sim_set_error("gles2 shape shader lookup failed");
        return 0;
    }

    display->gl.GenTextures(1, &display->gl_texture);
    display->gl.BindTexture(GL_TEXTURE_2D, display->gl_texture);
    display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    display->gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, display->width, display->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    {
        static const uint8_t k_white_pixel[4] = { 255u, 255u, 255u, 255u };
        display->gl.GenTextures(1, &display->gl_white_texture);
        display->gl.BindTexture(GL_TEXTURE_2D, display->gl_white_texture);
        display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
        display->gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, k_white_pixel);
    }

    display->gl.GenBuffers(1, &display->gl_vbo);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, display->gl_vbo);
    display->gl.BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(k_quad_vertices), k_quad_vertices, GL_STATIC_DRAW);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, 0u);

    display->gl_rgba_pixels = (uint8_t *)uya_gui_sim_host_malloc(rgba_bytes);
    if (display->gl_rgba_pixels == NULL) {
        uya_gui_sim_set_error("malloc gles2 scratch failed");
        return 0;
    }
    display->gl_rgba_bytes = rgba_bytes;

    display->gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    display->gl.Enable(GL_BLEND);
    display->gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    display->gl.Enable(GL_SCISSOR_TEST);
    display->gl.Scissor(0, 0, display->width, display->height);
    display->present_kind = UYA_GUI_SIM_PRESENT_GLES2;
    return 1;
}

static int uya_gui_sim_init_renderer_pipeline(UyaGuiSimDisplay *display, int vsync_enabled) {
    uint32_t renderer_flags = SDL_RENDERER_ACCELERATED;
    if (vsync_enabled != 0) {
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    }
    (void)SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    display->renderer = SDL_CreateRenderer(display->window, -1, renderer_flags);
    if (display->renderer == NULL) {
        display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (display->renderer == NULL) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    SDL_RenderSetLogicalSize(display->renderer, display->width, display->height);
    display->texture = SDL_CreateTexture(
        display->renderer,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_TEXTUREACCESS_STREAMING,
        display->width,
        display->height
    );
    if (display->texture == NULL) {
        uya_gui_sim_set_error(SDL_GetError());
        uya_gui_sim_destroy_renderer_resources(display);
        return 0;
    }
#if SDL_VERSION_ATLEAST(2, 0, 12)
    (void)SDL_SetTextureScaleMode(display->texture, SDL_ScaleModeNearest);
#endif
    display->present_kind = UYA_GUI_SIM_PRESENT_SOFTWARE;
    return 1;
}

static int uya_gui_sim_present_gles2(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height) {
    int drawable_w = 0;
    int drawable_h = 0;

    if (display->gl_context == NULL || display->window == NULL || display->gl_rgba_pixels == NULL) {
        uya_gui_sim_set_error("gles2 backend not initialized");
        return 0;
    }
    if ((size_t)width * (size_t)height * 4u > display->gl_rgba_bytes) {
        uya_gui_sim_set_error("gles2 scratch buffer too small");
        return 0;
    }

    uya_gui_sim_convert_argb_to_rgba(display->gl_rgba_pixels, pixels, pitch, width, height);
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    SDL_GL_GetDrawableSize(display->window, &drawable_w, &drawable_h);
    if (drawable_w <= 0 || drawable_h <= 0) {
        drawable_w = display->width * display->scale;
        drawable_h = display->height * display->scale;
    }

    display->gl.Viewport(0, 0, drawable_w, drawable_h);
    display->gl.Disable(GL_BLEND);
    display->gl.Enable(GL_SCISSOR_TEST);
    uya_gui_sim_gles2_reset_full_clip(display);
    display->gl.Clear(GL_COLOR_BUFFER_BIT);
    GLfloat vertices[16];
    uya_gui_sim_gles2_rect_vertices(display, 0.0f, 0.0f, (float)width, (float)height, 0.0f, 0.0f, 1.0f, 1.0f, vertices);
    display->gl.UseProgram(display->gl_program);
    display->gl.ActiveTexture(GL_TEXTURE0);
    display->gl.BindTexture(GL_TEXTURE_2D, display->gl_texture);
    display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    display->gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, display->gl_rgba_pixels);
    display->gl.Uniform1i(display->gl_uniform_tex, 0);
    display->gl.Uniform4f(display->gl_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, display->gl_vbo);
    display->gl.BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(GLfloat) * 16), vertices, GL_DYNAMIC_DRAW);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_pos, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)0);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_uv, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)(sizeof(GLfloat) * 2));
    display->gl.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, 0u);
    SDL_GL_SwapWindow(display->window);
    display->needs_refresh = 0;
    display->direct_frame_active = 0;
    return 1;
}

static int uya_gui_sim_begin_gles2_frame_with_pixels(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height) {
    int drawable_w = 0;
    int drawable_h = 0;
    if (display->gl_context == NULL || display->window == NULL || display->gl_rgba_pixels == NULL) {
        uya_gui_sim_set_error("gles2 backend not initialized");
        return 0;
    }
    if ((size_t)width * (size_t)height * 4u > display->gl_rgba_bytes) {
        uya_gui_sim_set_error("gles2 scratch buffer too small");
        return 0;
    }
    uya_gui_sim_convert_argb_to_rgba(display->gl_rgba_pixels, pixels, pitch, width, height);
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    SDL_GL_GetDrawableSize(display->window, &drawable_w, &drawable_h);
    if (drawable_w <= 0 || drawable_h <= 0) {
        drawable_w = display->width * display->scale;
        drawable_h = display->height * display->scale;
    }
    display->gl.Viewport(0, 0, drawable_w, drawable_h);
    display->gl.Disable(GL_BLEND);
    display->gl.Enable(GL_SCISSOR_TEST);
    uya_gui_sim_gles2_reset_full_clip(display);
    display->gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    display->gl.Clear(GL_COLOR_BUFFER_BIT);
    GLfloat vertices[16];
    uya_gui_sim_gles2_rect_vertices(display, 0.0f, 0.0f, (float)width, (float)height, 0.0f, 0.0f, 1.0f, 1.0f, vertices);
    display->gl.UseProgram(display->gl_program);
    display->gl.ActiveTexture(GL_TEXTURE0);
    display->gl.BindTexture(GL_TEXTURE_2D, display->gl_texture);
    display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    display->gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, display->gl_rgba_pixels);
    display->gl.Uniform1i(display->gl_uniform_tex, 0);
    display->gl.Uniform4f(display->gl_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, display->gl_vbo);
    display->gl.BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(GLfloat) * 16), vertices, GL_DYNAMIC_DRAW);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_pos, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)0);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_uv, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)(sizeof(GLfloat) * 2));
    display->gl.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, 0u);
    display->gl.Enable(GL_BLEND);
    display->gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    display->direct_frame_active = 1;
    return 1;
}

int32_t uya_gui_sim_sdl_gles2_active(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        return 0;
    }
    return display->present_kind == UYA_GUI_SIM_PRESENT_GLES2 ? 1 : 0;
}

int32_t uya_gui_sim_sdl_gles2_begin_frame(void *handle, uint32_t clear_argb) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    GLfloat rgba[4];
    int drawable_w = 0;
    int drawable_h = 0;
    if (display == NULL || display->present_kind != UYA_GUI_SIM_PRESENT_GLES2 || display->gl_context == NULL) {
        uya_gui_sim_set_error("gles2 frame begin unavailable");
        return 0;
    }
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    SDL_GL_GetDrawableSize(display->window, &drawable_w, &drawable_h);
    if (drawable_w <= 0 || drawable_h <= 0) {
        drawable_w = display->width * display->scale;
        drawable_h = display->height * display->scale;
    }
    uya_gui_sim_argb_u32_to_rgba_f32(clear_argb, rgba);
    display->gl.Viewport(0, 0, drawable_w, drawable_h);
    display->gl.Enable(GL_BLEND);
    display->gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    display->gl.Enable(GL_SCISSOR_TEST);
    uya_gui_sim_gles2_reset_full_clip(display);
    display->gl.ClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    display->gl.Clear(GL_COLOR_BUFFER_BIT);
    display->direct_frame_active = 1;
    return 1;
}

int32_t uya_gui_sim_sdl_gles2_begin_frame_from_pixels(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL || display->present_kind != UYA_GUI_SIM_PRESENT_GLES2 || display->gl_context == NULL || pixels == NULL) {
        uya_gui_sim_set_error("gles2 frame begin from pixels unavailable");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_set_error("unsupported framebuffer format for gles2 frame begin");
        return 0;
    }
    return uya_gui_sim_begin_gles2_frame_with_pixels(display, pixels, pitch, width, height);
}

int32_t uya_gui_sim_sdl_gles2_set_clip(void *handle, int32_t x, int32_t y, int32_t w, int32_t h) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    int32_t left = x;
    int32_t top = y;
    int32_t width = w;
    int32_t height = h;
    if (display == NULL || display->present_kind != UYA_GUI_SIM_PRESENT_GLES2 || display->gl_context == NULL) {
        uya_gui_sim_set_error("gles2 clip unavailable");
        return 0;
    }
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    if (left < 0) {
        width += left;
        left = 0;
    }
    if (top < 0) {
        height += top;
        top = 0;
    }
    if (left + width > display->width) {
        width = display->width - left;
    }
    if (top + height > display->height) {
        height = display->height - top;
    }
    if (width <= 0 || height <= 0) {
        display->gl.Scissor(0, 0, 0, 0);
        return 1;
    }
    uya_gui_sim_gles2_apply_scissor(display, left, top, width, height);
    return 1;
}

int32_t uya_gui_sim_sdl_gles2_fill_round_rect(void *handle, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint32_t color_argb) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 fill round rect unavailable");
        return 0;
    }
    rect.x = (int16_t)x;
    rect.y = (int16_t)y;
    rect.w = (uint16_t)w;
    rect.h = (uint16_t)h;
    color.a = (uint8_t)((color_argb >> 24) & 0xFFu);
    color.r = (uint8_t)((color_argb >> 16) & 0xFFu);
    color.g = (uint8_t)((color_argb >> 8) & 0xFFu);
    color.b = (uint8_t)(color_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &color, &color, (float)radius, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_draw_round_rect(void *handle, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint32_t color_argb, int32_t stroke_width) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 draw round rect unavailable");
        return 0;
    }
    rect.x = (int16_t)x;
    rect.y = (int16_t)y;
    rect.w = (uint16_t)w;
    rect.h = (uint16_t)h;
    color.a = (uint8_t)((color_argb >> 24) & 0xFFu);
    color.r = (uint8_t)((color_argb >> 16) & 0xFFu);
    color.g = (uint8_t)((color_argb >> 8) & 0xFFu);
    color.b = (uint8_t)(color_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &color, &color, (float)radius, (float)stroke_width, 0.0f, 0.0f, 0.0f, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_fill_linear_gradient(void *handle, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t start_argb, uint32_t end_argb, int32_t horizontal) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd start;
    UyaGuiColorCmd end;
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 linear gradient unavailable");
        return 0;
    }
    rect.x = (int16_t)x;
    rect.y = (int16_t)y;
    rect.w = (uint16_t)w;
    rect.h = (uint16_t)h;
    start.a = (uint8_t)((start_argb >> 24) & 0xFFu);
    start.r = (uint8_t)((start_argb >> 16) & 0xFFu);
    start.g = (uint8_t)((start_argb >> 8) & 0xFFu);
    start.b = (uint8_t)(start_argb & 0xFFu);
    end.a = (uint8_t)((end_argb >> 24) & 0xFFu);
    end.r = (uint8_t)((end_argb >> 16) & 0xFFu);
    end.g = (uint8_t)((end_argb >> 8) & 0xFFu);
    end.b = (uint8_t)(end_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &start, &end, 0.0f, 0.0f, 1.0f, horizontal != 0 ? 1.0f : 0.0f, 0.0f, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_fill_radial_gradient(void *handle, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t inner_argb, uint32_t outer_argb) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd inner;
    UyaGuiColorCmd outer;
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 radial gradient unavailable");
        return 0;
    }
    rect.x = (int16_t)x;
    rect.y = (int16_t)y;
    rect.w = (uint16_t)w;
    rect.h = (uint16_t)h;
    inner.a = (uint8_t)((inner_argb >> 24) & 0xFFu);
    inner.r = (uint8_t)((inner_argb >> 16) & 0xFFu);
    inner.g = (uint8_t)((inner_argb >> 8) & 0xFFu);
    inner.b = (uint8_t)(inner_argb & 0xFFu);
    outer.a = (uint8_t)((outer_argb >> 24) & 0xFFu);
    outer.r = (uint8_t)((outer_argb >> 16) & 0xFFu);
    outer.g = (uint8_t)((outer_argb >> 8) & 0xFFu);
    outer.b = (uint8_t)(outer_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &inner, &outer, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_fill_round_shadow(void *handle, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, int32_t blur, uint32_t color_argb) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
    rect.x = (int16_t)(x - blur);
    rect.y = (int16_t)(y - blur);
    rect.w = (uint16_t)(w + blur * 2);
    rect.h = (uint16_t)(h + blur * 2);
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 round shadow unavailable");
        return 0;
    }
    color.a = (uint8_t)((color_argb >> 24) & 0xFFu);
    color.r = (uint8_t)((color_argb >> 16) & 0xFFu);
    color.g = (uint8_t)((color_argb >> 8) & 0xFFu);
    color.b = (uint8_t)(color_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &color, &color, (float)radius, (float)blur, 3.0f, 0.0f, 0.0f, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_draw_circle(void *handle, int32_t cx, int32_t cy, int32_t r, uint32_t color_argb) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 draw circle unavailable");
        return 0;
    }
    rect.x = (int16_t)(cx - r);
    rect.y = (int16_t)(cy - r);
    rect.w = (uint16_t)(r * 2 + 1);
    rect.h = (uint16_t)(r * 2 + 1);
    color.a = (uint8_t)((color_argb >> 24) & 0xFFu);
    color.r = (uint8_t)((color_argb >> 16) & 0xFFu);
    color.g = (uint8_t)((color_argb >> 8) & 0xFFu);
    color.b = (uint8_t)(color_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &color, &color, (float)r, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_fill_circle(void *handle, int32_t cx, int32_t cy, int32_t r, uint32_t color_argb) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 fill circle unavailable");
        return 0;
    }
    rect.x = (int16_t)(cx - r);
    rect.y = (int16_t)(cy - r);
    rect.w = (uint16_t)(r * 2 + 1);
    rect.h = (uint16_t)(r * 2 + 1);
    color.a = (uint8_t)((color_argb >> 24) & 0xFFu);
    color.r = (uint8_t)((color_argb >> 16) & 0xFFu);
    color.g = (uint8_t)((color_argb >> 8) & 0xFFu);
    color.b = (uint8_t)(color_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &color, &color, (float)r, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_draw_arc(void *handle, int32_t cx, int32_t cy, int32_t r, int32_t start_deg, int32_t end_deg, int32_t width, uint32_t color_argb) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 draw arc unavailable");
        return 0;
    }
    rect.x = (int16_t)(cx - r - width);
    rect.y = (int16_t)(cy - r - width);
    rect.w = (uint16_t)(r * 2 + width * 2 + 1);
    rect.h = (uint16_t)(r * 2 + width * 2 + 1);
    color.a = (uint8_t)((color_argb >> 24) & 0xFFu);
    color.r = (uint8_t)((color_argb >> 16) & 0xFFu);
    color.g = (uint8_t)((color_argb >> 8) & 0xFFu);
    color.b = (uint8_t)(color_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &color, &color, (float)r, (float)width, 6.0f, (float)start_deg, (float)end_deg, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_fill_arc(void *handle, int32_t cx, int32_t cy, int32_t r, int32_t start_deg, int32_t end_deg, uint32_t color_argb) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    UyaGuiRectCmd rect;
    UyaGuiColorCmd color;
    if (display == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 fill arc unavailable");
        return 0;
    }
    rect.x = (int16_t)(cx - r);
    rect.y = (int16_t)(cy - r);
    rect.w = (uint16_t)(r * 2 + 1);
    rect.h = (uint16_t)(r * 2 + 1);
    color.a = (uint8_t)((color_argb >> 24) & 0xFFu);
    color.r = (uint8_t)((color_argb >> 16) & 0xFFu);
    color.g = (uint8_t)((color_argb >> 8) & 0xFFu);
    color.b = (uint8_t)(color_argb & 0xFFu);
    return uya_gui_sim_gles2_draw_shape_rect(display, &rect, &color, &color, (float)r, 0.0f, 7.0f, (float)start_deg, (float)end_deg, 0.0f);
}

int32_t uya_gui_sim_sdl_gles2_fill_rects(void *handle, const UyaGuiGpuFillRectCmd *cmds, int32_t count) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    int32_t i = 0;
    if (display == NULL || cmds == NULL || count < 0 || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 fill rects unavailable");
        return 0;
    }
    while (i < count) {
        if (!uya_gui_sim_gles2_draw_solid_rect(display, &cmds[i].rect, &cmds[i].color)) {
            return 0;
        }
        i += 1;
    }
    return 1;
}

int32_t uya_gui_sim_sdl_gles2_draw_rects(void *handle, const UyaGuiGpuStrokeRectCmd *cmds, int32_t count) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    int32_t i = 0;
    if (display == NULL || cmds == NULL || count < 0 || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 draw rects unavailable");
        return 0;
    }
    while (i < count) {
        const UyaGuiGpuStrokeRectCmd *cmd = &cmds[i];
        int32_t width = cmd->width > 0 ? cmd->width : 1;
        UyaGuiRectCmd top = cmd->rect;
        UyaGuiRectCmd bottom = cmd->rect;
        UyaGuiRectCmd left = cmd->rect;
        UyaGuiRectCmd right = cmd->rect;
        if ((int32_t)top.w <= 0 || (int32_t)top.h <= 0) {
            i += 1;
            continue;
        }
        top.h = (uint16_t)((width < (int32_t)cmd->rect.h) ? width : (int32_t)cmd->rect.h);
        bottom.y = (int16_t)(cmd->rect.y + (int16_t)(cmd->rect.h - top.h));
        bottom.h = top.h;
        left.w = (uint16_t)((width < (int32_t)cmd->rect.w) ? width : (int32_t)cmd->rect.w);
        right.x = (int16_t)(cmd->rect.x + (int16_t)(cmd->rect.w - left.w));
        right.w = left.w;
        if (!uya_gui_sim_gles2_draw_solid_rect(display, &top, &cmd->color)
            || !uya_gui_sim_gles2_draw_solid_rect(display, &bottom, &cmd->color)
            || !uya_gui_sim_gles2_draw_solid_rect(display, &left, &cmd->color)
            || !uya_gui_sim_gles2_draw_solid_rect(display, &right, &cmd->color)) {
            return 0;
        }
        i += 1;
    }
    return 1;
}

int32_t uya_gui_sim_sdl_gles2_draw_lines(void *handle, const UyaGuiGpuLineCmd *cmds, int32_t count) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    int32_t i = 0;
    if (display == NULL || cmds == NULL || count < 0 || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 draw lines unavailable");
        return 0;
    }
    while (i < count) {
        if (!uya_gui_sim_gles2_draw_line(display, &cmds[i])) {
            return 0;
        }
        i += 1;
    }
    return 1;
}

int32_t uya_gui_sim_sdl_gles2_draw_images(void *handle, const UyaGuiGpuImageCmd *cmds, int32_t count) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    int32_t i = 0;
    if (display == NULL || cmds == NULL || count < 0 || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 draw images unavailable");
        return 0;
    }
    while (i < count) {
        const UyaGuiGpuImageCmd *cmd = &cmds[i];
        int32_t src_x = 0;
        int32_t src_y = 0;
        int32_t src_w = cmd->image_w;
        int32_t src_h = cmd->image_h;
        if (cmd->pixels == NULL) {
            i += 1;
            continue;
        }
        if (cmd->has_src != 0) {
            src_x = cmd->src_rect.x;
            src_y = cmd->src_rect.y;
            src_w = (int32_t)cmd->src_rect.w;
            src_h = (int32_t)cmd->src_rect.h;
        }
        if (!uya_gui_sim_gles2_draw_image_region(display, cmd->pixels, cmd->stride, cmd->format_tag, src_x, src_y, src_w, src_h, &cmd->dst_rect)) {
            return 0;
        }
        i += 1;
    }
    return 1;
}

int32_t uya_gui_sim_sdl_gles2_draw_glyphs(void *handle, const UyaGuiGpuGlyphCmd *cmds, int32_t count) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL || cmds == NULL || count < 0 || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 draw glyphs unavailable");
        return 0;
    }
    return uya_gui_sim_gles2_draw_glyph_batch(display, cmds, count);
}

int32_t uya_gui_sim_sdl_gles2_present_frame(void *handle,
                                            const uint8_t *pixels,
                                            int32_t pitch,
                                            int32_t width,
                                            int32_t height,
                                            const UyaGuiRectCmd *rects,
                                            int32_t rect_count,
                                            int32_t overlay_enabled) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    int32_t i = 0;
    if (display == NULL || display->present_kind != UYA_GUI_SIM_PRESENT_GLES2 || display->gl_context == NULL || !display->direct_frame_active) {
        uya_gui_sim_set_error("gles2 direct present unavailable");
        return 0;
    }
    (void)width;
    (void)height;
    uya_gui_sim_gles2_reset_full_clip(display);
    if (pixels != NULL && rects != NULL && rect_count > 0) {
        while (i < rect_count) {
            const UyaGuiRectCmd *rect = &rects[i];
            if (rect->w > 0 && rect->h > 0) {
                if (!uya_gui_sim_gles2_present_region(display, pixels, pitch, 2, rect->x, rect->y, rect->w, rect->h, rect)) {
                    return 0;
                }
            }
            i += 1;
        }
    }
    if (overlay_enabled != 0 && rects != NULL && rect_count > 0) {
        UyaGuiColorCmd overlay = { 255u, 255u, 0u, 255u };
        UyaGuiGpuStrokeRectCmd stroke;
        stroke.color = overlay;
        stroke.width = 1;
        i = 0;
        while (i < rect_count) {
            stroke.rect = rects[i];
            if (!uya_gui_sim_sdl_gles2_draw_rects(display, &stroke, 1)) {
                return 0;
            }
            i += 1;
        }
    }
    uya_gui_sim_gles2_reset_full_clip(display);
    SDL_GL_SwapWindow(display->window);
    display->needs_refresh = 0;
    display->direct_frame_active = 0;
    return 1;
}

int32_t uya_gui_sim_sdl_display_present_begin(void *handle, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        uya_gui_sim_set_error("display null");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_set_error("unsupported framebuffer format (expected ARGB8888/BGRA texture)");
        return 0;
    }
    display->pending_pixels = NULL;
    display->pending_pitch = 0;
    display->pending_width = width;
    display->pending_height = height;
    display->pending_format_tag = format_tag;
    display->pending_present = 0;
    display->dirty_overlay_full = 0;
    display->dirty_overlay_rect_count = 0;
    return 1;
}

int32_t uya_gui_sim_sdl_display_present_region(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag,
                                               int32_t x, int32_t y, int32_t w, int32_t h) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    SDL_Rect rect;
    const uint8_t *region_pixels;
    if (display == NULL || pixels == NULL) {
        uya_gui_sim_set_error("display/pixels null");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_set_error("unsupported framebuffer format (expected ARGB8888/BGRA texture)");
        return 0;
    }
    if (w <= 0 || h <= 0) {
        return 1;
    }
    if (x < 0 || y < 0 || x + w > width || y + h > height) {
        uya_gui_sim_set_error("dirty rect out of bounds");
        return 0;
    }

    display->pending_pixels = pixels;
    display->pending_pitch = pitch;
    display->pending_width = width;
    display->pending_height = height;
    display->pending_format_tag = format_tag;

    if (display->present_kind == UYA_GUI_SIM_PRESENT_GLES2) {
        display->pending_present = 1;
        if (display->dirty_overlay_enabled) {
            if (display->dirty_overlay_rect_count < UYA_GUI_SIM_DIRTY_OVERLAY_MAX_RECTS) {
                display->dirty_overlay_rects[display->dirty_overlay_rect_count++] = (SDL_Rect){ x, y, w, h };
            } else {
                display->dirty_overlay_full = 1;
            }
        }
        return 1;
    }

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    region_pixels = pixels + (size_t)y * (size_t)pitch + (size_t)x * 4u;
    if (!uya_gui_sim_update_texture_region(display, &rect, region_pixels, pitch)) {
        return 0;
    }
    display->pending_present = 1;
    if (display->dirty_overlay_enabled) {
        if (display->dirty_overlay_rect_count < UYA_GUI_SIM_DIRTY_OVERLAY_MAX_RECTS) {
            display->dirty_overlay_rects[display->dirty_overlay_rect_count++] = rect;
        } else {
            display->dirty_overlay_full = 1;
        }
    }
    return 1;
}

int32_t uya_gui_sim_sdl_display_present_end(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    const uint8_t *pixels = NULL;
    int32_t pitch = 0;
    if (display == NULL) {
        uya_gui_sim_set_error("display null");
        return 0;
    }
    if (!display->pending_present) {
        return 1;
    }
    if (display->present_kind == UYA_GUI_SIM_PRESENT_GLES2) {
        pixels = uya_gui_sim_overlay_prepare(
            display,
            display->pending_pixels,
            display->pending_pitch,
            display->pending_width,
            display->pending_height,
            &pitch
        );
        return uya_gui_sim_present_gles2(
            display,
            pixels,
            pitch,
            display->pending_width,
            display->pending_height
        );
    }
    if (!display->dirty_overlay_enabled) {
        if (SDL_RenderClear(display->renderer) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return 0;
        }
        if (SDL_RenderCopy(display->renderer, display->texture, NULL, NULL) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return 0;
        }
        SDL_RenderPresent(display->renderer);
        return 1;
    }
    pixels = uya_gui_sim_overlay_prepare(
        display,
        display->pending_pixels,
        display->pending_pitch,
        display->pending_width,
        display->pending_height,
        &pitch
    );
    return uya_gui_sim_present_software(display, pixels, pitch);
}

static int uya_gui_sim_keycode(SDL_Keycode sym) {
    if (sym >= 32 && sym < 127) {
        return (int)sym;
    }
    switch (sym) {
        case SDLK_BACKSPACE: return 8;
        case SDLK_ESCAPE: return 27;
        case SDLK_RETURN: return 13;
        case SDLK_SPACE: return 32;
        case SDLK_LEFT: return 1000;
        case SDLK_RIGHT: return 1001;
        case SDLK_UP: return 1002;
        case SDLK_DOWN: return 1003;
        case SDLK_F11: return 1011;
        default:
            if (sym >= 0 && sym <= 0xffff) {
                return (int)sym;
            }
            return 0;
    }
}

static void uya_gui_sim_clamp_logical_point(const UyaGuiSimDisplay *display, int in_x, int in_y, int16_t *out_x, int16_t *out_y) {
    int logical_x = in_x;
    int logical_y = in_y;
    if (display->present_kind == UYA_GUI_SIM_PRESENT_GLES2 && display->window != NULL) {
        int window_w = 0;
        int window_h = 0;
        SDL_GetWindowSize(display->window, &window_w, &window_h);
        if (window_w > 0 && window_h > 0) {
            logical_x = (in_x * display->width) / window_w;
            logical_y = (in_y * display->height) / window_h;
        }
    }
    if (logical_x < 0) {
        logical_x = 0;
    }
    if (logical_y < 0) {
        logical_y = 0;
    }
    if (logical_x >= display->width) {
        logical_x = display->width - 1;
    }
    if (logical_y >= display->height) {
        logical_y = display->height - 1;
    }
    *out_x = (int16_t)logical_x;
    *out_y = (int16_t)logical_y;
}

void *uya_gui_sim_sdl_display_open(int32_t width, int32_t height, int32_t scale, int32_t fullscreen, int32_t gpu_mode, int32_t vsync_enabled, const uint8_t *title) {
    uya_gui_sim_init_host_allocators();
    if (g_sdl_refcount == 0) {
        if (g_host_malloc_fn == NULL || g_host_calloc_fn == NULL || g_host_realloc_fn == NULL || g_host_free_fn == NULL) {
            uya_gui_sim_set_error("failed to resolve host allocator symbols");
            return NULL;
        }
        if (SDL_SetMemoryFunctions(
            uya_gui_sim_host_malloc,
            uya_gui_sim_host_calloc,
            uya_gui_sim_host_realloc,
            uya_gui_sim_host_free
        ) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return NULL;
        }
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return NULL;
        }
    }
    g_sdl_refcount += 1;

    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)uya_gui_sim_host_calloc(1, sizeof(UyaGuiSimDisplay));
    if (display == NULL) {
        uya_gui_sim_set_error("calloc display failed");
        g_sdl_refcount -= 1;
        if (g_sdl_refcount == 0) {
            SDL_Quit();
        }
        return NULL;
    }

    display->width = width;
    display->height = height;
    display->scale = scale > 0 ? scale : 1;
    display->fullscreen = fullscreen != 0;
    display->gl_attr_pos = -1;
    display->gl_attr_uv = -1;
    display->gl_uniform_tex = -1;
    display->present_kind = UYA_GUI_SIM_PRESENT_SOFTWARE;
    display->dirty_overlay_enabled = 0;
    display->dirty_overlay_full = 0;
    display->dirty_overlay_rect_count = 0;

    if (gpu_mode != UYA_GUI_SIM_GPU_SOFTWARE) {
        SDL_GL_ResetAttributes();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        display->window = uya_gui_sim_create_window(width, height, display->scale, fullscreen, title, SDL_WINDOW_OPENGL);
        if (display->window != NULL) {
            display->gl_context = SDL_GL_CreateContext(display->window);
            if (display->gl_context != NULL) {
                (void)SDL_GL_SetSwapInterval(vsync_enabled != 0 ? 1 : 0);
                if (uya_gui_sim_init_gles2_pipeline(display)) {
                    g_active_display = display;
                    g_last_error[0] = '\0';
                    return display;
                }
            } else {
                uya_gui_sim_set_error(SDL_GetError());
            }
        } else {
            uya_gui_sim_set_error(SDL_GetError());
        }
        if (gpu_mode == UYA_GUI_SIM_GPU_GLES2) {
            uya_gui_sim_destroy_display_resources(display);
            uya_gui_sim_host_free(display);
            g_sdl_refcount -= 1;
            if (g_sdl_refcount == 0) {
                SDL_Quit();
            }
            return NULL;
        }
        uya_gui_sim_destroy_display_resources(display);
    }

    display->window = uya_gui_sim_create_window(width, height, display->scale, fullscreen, title, 0u);
    if (display->window == NULL || !uya_gui_sim_init_renderer_pipeline(display, vsync_enabled)) {
        if (display->window == NULL) {
            uya_gui_sim_set_error(SDL_GetError());
        }
        uya_gui_sim_destroy_display_resources(display);
        uya_gui_sim_host_free(display);
        g_sdl_refcount -= 1;
        if (g_sdl_refcount == 0) {
            SDL_Quit();
        }
        return NULL;
    }

    g_active_display = display;
    g_last_error[0] = '\0';
    return display;
}

void uya_gui_sim_sdl_display_close(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        return;
    }
    if (g_active_display == display) {
        g_active_display = NULL;
    }
    uya_gui_sim_destroy_display_resources(display);
    uya_gui_sim_host_free(display);
    if (g_sdl_refcount > 0) {
        g_sdl_refcount -= 1;
    }
    if (g_sdl_refcount == 0) {
        SDL_Quit();
    }
}

int32_t uya_gui_sim_sdl_display_present(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    const uint8_t *present_pixels = pixels;
    int32_t present_pitch = pitch;
    if (display == NULL || pixels == NULL) {
        uya_gui_sim_set_error("display/pixels null");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_set_error("unsupported framebuffer format (expected ARGB8888/BGRA texture)");
        return 0;
    }
    display->dirty_overlay_full = display->dirty_overlay_enabled ? 1 : 0;
    display->dirty_overlay_rect_count = 0;
    present_pixels = uya_gui_sim_overlay_prepare(display, pixels, pitch, width, height, &present_pitch);
    if (display->present_kind == UYA_GUI_SIM_PRESENT_GLES2) {
        return uya_gui_sim_present_gles2(display, present_pixels, present_pitch, width, height);
    }
    return uya_gui_sim_present_software(display, present_pixels, present_pitch);
}

int32_t uya_gui_sim_sdl_display_set_dirty_overlay(void *handle, int32_t enabled) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        uya_gui_sim_set_error("display null");
        return 0;
    }
    display->dirty_overlay_enabled = enabled != 0;
    display->dirty_overlay_full = 0;
    display->dirty_overlay_rect_count = 0;
    return 1;
}

int32_t uya_gui_sim_sdl_display_consume_refresh_request(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        uya_gui_sim_set_error("display null");
        return 0;
    }
    if (!display->needs_refresh) {
        return 0;
    }
    display->needs_refresh = 0;
    return 1;
}

int32_t uya_gui_sim_sdl_display_set_fullscreen(void *handle, int32_t enabled) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        return 0;
    }
    uint32_t flags = enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0u;
    if (SDL_SetWindowFullscreen(display->window, flags) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    display->fullscreen = enabled != 0;
    return 1;
}

void uya_gui_sim_sdl_display_set_title(void *handle, const uint8_t *title) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL || title == NULL) {
        return;
    }
    SDL_SetWindowTitle(display->window, (const char *)title);
}

const uint8_t *uya_gui_sim_sdl_display_gpu_name(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        return (const uint8_t *)"unknown";
    }
    return (const uint8_t *)uya_gui_sim_present_name(display);
}

int32_t uya_gui_sim_sdl_poll_event(SdlHostEvent *out_evt) {
    SDL_Event evt;
    if (out_evt == NULL) {
        return 0;
    }
    out_evt->kind = SDL_EVT_NONE;
    out_evt->x = 0;
    out_evt->y = 0;
    out_evt->value = 0;
    out_evt->key_code = 0;
    out_evt->reserved = 0;

    if (g_has_pending_event != 0) {
        evt = g_pending_event;
        g_has_pending_event = 0;
    } else if (SDL_PollEvent(&evt) == 0) {
        return 0;
    }
    if (evt.type == SDL_QUIT) {
        out_evt->kind = SDL_EVT_QUIT;
        return 1;
    }
    if (g_active_display == NULL) {
        return 0;
    }
    if (evt.type == SDL_WINDOWEVENT) {
        if (evt.window.event == SDL_WINDOWEVENT_EXPOSED
            || evt.window.event == SDL_WINDOWEVENT_RESTORED
            || evt.window.event == SDL_WINDOWEVENT_SHOWN
            || evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            g_active_display->needs_refresh = 1;
        }
        return 0;
    }

    switch (evt.type) {
        case SDL_MOUSEMOTION:
            out_evt->kind = SDL_EVT_POINTER_MOVE;
            uya_gui_sim_clamp_logical_point(g_active_display, evt.motion.x, evt.motion.y, &out_evt->x, &out_evt->y);
            return 1;
        case SDL_MOUSEBUTTONDOWN:
            if (evt.button.button == SDL_BUTTON_LEFT) {
                out_evt->kind = SDL_EVT_POINTER_DOWN;
                uya_gui_sim_clamp_logical_point(g_active_display, evt.button.x, evt.button.y, &out_evt->x, &out_evt->y);
                return 1;
            }
            return 0;
        case SDL_MOUSEBUTTONUP:
            if (evt.button.button == SDL_BUTTON_LEFT) {
                out_evt->kind = SDL_EVT_POINTER_UP;
                uya_gui_sim_clamp_logical_point(g_active_display, evt.button.x, evt.button.y, &out_evt->x, &out_evt->y);
                return 1;
            }
            return 0;
        case SDL_MOUSEWHEEL:
            out_evt->kind = SDL_EVT_WHEEL;
            out_evt->value = evt.wheel.y;
            return 1;
        case SDL_KEYDOWN:
            if (evt.key.repeat != 0) {
                return 0;
            }
            out_evt->kind = SDL_EVT_KEY_DOWN;
            out_evt->key_code = (uint16_t)uya_gui_sim_keycode(evt.key.keysym.sym);
            return 1;
        case SDL_KEYUP:
            out_evt->kind = SDL_EVT_KEY_UP;
            out_evt->key_code = (uint16_t)uya_gui_sim_keycode(evt.key.keysym.sym);
            return 1;
        default:
            return 0;
    }
}

int32_t uya_gui_sim_sdl_wait_event(int32_t timeout_ms) {
    SDL_Event evt;
    if (g_has_pending_event != 0) {
        return 1;
    }
    if (timeout_ms < 0) {
        if (SDL_WaitEvent(&evt) == 0) {
            return 0;
        }
    } else if (SDL_WaitEventTimeout(&evt, timeout_ms) == 0) {
        return 0;
    }
    g_pending_event = evt;
    g_has_pending_event = 1;
    return 1;
}

const uint8_t *uya_gui_sim_sdl_last_error(void) {
    if (g_last_error[0] != '\0') {
        return (const uint8_t *)g_last_error;
    }
    return (const uint8_t *)SDL_GetError();
}
