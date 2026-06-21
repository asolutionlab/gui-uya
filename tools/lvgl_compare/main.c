#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>

#include "lvgl.h"
#include "src/draw/snapshot/lv_snapshot.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/libs/tiny_ttf/lv_tiny_ttf.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define LVGL_COMPARE_OUT_REL "build/text_compare/lvgl_text_render_samples.bmp"
#define LVGL_COMPARE_CANVAS_W 420
#define LVGL_COMPARE_CANVAS_H 348
#define LVGL_COMPARE_PANEL_W 192
#define LVGL_COMPARE_PANEL_H 138
#define LVGL_COMPARE_SAMPLE_W 56
#define LVGL_COMPARE_SAMPLE_H 24
#define LVGL_COMPARE_SAMPLE_SCALE 3

typedef struct {
    uint8_t * data;
    size_t size;
    lv_font_t * font;
} font_asset_t;

static const char * latin_font_path = "src/gui/render/DejaVuSans.ttf";
static const char * cjk_font_path = "/usr/share/fonts/opentype/source-han-cjk/SourceHanSansSC-Regular.otf";
static const char * cjk_font_fallback_path = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";

static bool ensure_dir(const char * path)
{
    if(mkdir(path, 0755) == 0) return true;
    return errno == EEXIST;
}

static uint8_t * read_file_bytes(const char * path, size_t * out_size)
{
    FILE * fp = fopen(path, "rb");
    uint8_t * buf;
    long size;

    if(fp == NULL) return NULL;
    if(fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if(size <= 0) {
        fclose(fp);
        return NULL;
    }
    if(fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    buf = malloc((size_t)size);
    if(buf == NULL) {
        fclose(fp);
        return NULL;
    }

    if(fread(buf, 1, (size_t)size, fp) != (size_t)size) {
        free(buf);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    *out_size = (size_t)size;
    return buf;
}

static bool load_tiny_ttf_font(font_asset_t * asset, const char * path, int32_t px)
{
    if(asset == NULL || path == NULL) return false;
    memset(asset, 0, sizeof(*asset));
    asset->data = read_file_bytes(path, &asset->size);
    if(asset->data == NULL) return false;
    asset->font = lv_tiny_ttf_create_data(asset->data, asset->size, px);
    if(asset->font == NULL) {
        free(asset->data);
        memset(asset, 0, sizeof(*asset));
        return false;
    }
    return true;
}

static void unload_tiny_ttf_font(font_asset_t * asset)
{
    if(asset == NULL) return;
    if(asset->font) lv_tiny_ttf_destroy(asset->font);
    free(asset->data);
    memset(asset, 0, sizeof(*asset));
}

static lv_color_t color_hex(uint32_t hex)
{
    return lv_color_hex(hex);
}

static lv_draw_buf_t * scale_draw_buf_3x(const lv_draw_buf_t * src)
{
    uint32_t src_w;
    uint32_t src_h;
    uint32_t dst_w;
    uint32_t dst_h;
    lv_draw_buf_t * dst;
    uint32_t x;
    uint32_t y;

    if(src == NULL || src->header.cf != LV_COLOR_FORMAT_ARGB8888) return NULL;

    src_w = src->header.w;
    src_h = src->header.h;
    dst_w = src_w * LVGL_COMPARE_SAMPLE_SCALE;
    dst_h = src_h * LVGL_COMPARE_SAMPLE_SCALE;
    dst = lv_draw_buf_create(dst_w, dst_h, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    if(dst == NULL) return NULL;

    for(y = 0; y < dst_h; y++) {
        uint8_t * dst_row = (uint8_t *)dst->data + y * dst->header.stride;
        const uint8_t * src_row = (const uint8_t *)src->data + (y / LVGL_COMPARE_SAMPLE_SCALE) * src->header.stride;
        for(x = 0; x < dst_w; x++) {
            const uint8_t * src_px = src_row + (x / LVGL_COMPARE_SAMPLE_SCALE) * 4;
            uint8_t * dst_px = dst_row + x * 4;
            dst_px[0] = src_px[0];
            dst_px[1] = src_px[1];
            dst_px[2] = src_px[2];
            dst_px[3] = src_px[3];
        }
    }

    for(x = LVGL_COMPARE_SAMPLE_SCALE; x < dst_w; x += LVGL_COMPARE_SAMPLE_SCALE) {
        for(y = 0; y < dst_h; y++) {
            uint8_t * px = (uint8_t *)dst->data + y * dst->header.stride + x * 4;
            px[0] = 184;
            px[1] = 163;
            px[2] = 148;
            px[3] = 255;
        }
    }

    for(y = LVGL_COMPARE_SAMPLE_SCALE; y < dst_h; y += LVGL_COMPARE_SAMPLE_SCALE) {
        uint8_t * row = (uint8_t *)dst->data + y * dst->header.stride;
        for(x = 0; x < dst_w; x++) {
            uint8_t * px = row + x * 4;
            px[0] = 184;
            px[1] = 163;
            px[2] = 148;
            px[3] = 255;
        }
    }

    return dst;
}

static lv_draw_buf_t * snapshot_text_sample(lv_obj_t * root, const lv_font_t * font, const char * line0, const char * line1)
{
    lv_obj_t * sample = lv_obj_create(root);
    lv_obj_t * label0 = lv_label_create(sample);
    lv_obj_t * label1 = NULL;
    lv_draw_buf_t * shot;
    int32_t line_h = font ? font->line_height : 12;

    lv_obj_remove_flag(sample, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(sample, LVGL_COMPARE_SAMPLE_W, LVGL_COMPARE_SAMPLE_H);
    lv_obj_set_style_radius(sample, 0, 0);
    lv_obj_set_style_border_width(sample, 0, 0);
    lv_obj_set_style_pad_all(sample, 0, 0);
    lv_obj_set_style_bg_color(sample, color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(sample, LV_OPA_COVER, 0);
    lv_obj_set_pos(sample, 1000, 1000);

    lv_obj_set_width(label0, LVGL_COMPARE_SAMPLE_W);
    lv_label_set_text(label0, line0);
    lv_obj_set_style_text_font(label0, font, 0);
    lv_obj_set_style_text_align(label0, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label0, color_hex(0x111827), 0);
    lv_obj_set_pos(label0, 0, 1);

    if(line1 && line1[0]) {
        label1 = lv_label_create(sample);
        lv_obj_set_width(label1, LVGL_COMPARE_SAMPLE_W);
        lv_label_set_text(label1, line1);
        lv_obj_set_style_text_font(label1, font, 0);
        lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(label1, color_hex(0x111827), 0);
        lv_obj_set_pos(label1, 0, LVGL_COMPARE_SAMPLE_H - line_h - 1);
    }

    lv_obj_update_layout(sample);
    shot = lv_snapshot_take(sample, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_delete(sample);
    return shot;
}

static lv_obj_t * create_frame(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_t * frame = lv_obj_create(parent);
    lv_obj_remove_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(frame, x, y);
    lv_obj_set_size(frame, w, h);
    lv_obj_set_style_radius(frame, 0, 0);
    lv_obj_set_style_pad_all(frame, 0, 0);
    lv_obj_set_style_bg_color(frame, color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(frame, 1, 0);
    lv_obj_set_style_border_color(frame, color_hex(0xCBD5E1), 0);
    return frame;
}

static void mount_native_and_zoom(lv_obj_t * panel, lv_draw_buf_t * native_buf, lv_draw_buf_t * zoom_buf)
{
    const int32_t body_x = 8;
    const int32_t body_y = 30;
    const int32_t native_frame_x = body_x + (176 - LVGL_COMPARE_SAMPLE_W) / 2;
    const int32_t zoom_w = LVGL_COMPARE_SAMPLE_W * LVGL_COMPARE_SAMPLE_SCALE;
    const int32_t zoom_h = LVGL_COMPARE_SAMPLE_H * LVGL_COMPARE_SAMPLE_SCALE;
    const int32_t zoom_frame_x = body_x + (176 - zoom_w) / 2;
    lv_obj_t * native_frame = create_frame(panel, native_frame_x, body_y, LVGL_COMPARE_SAMPLE_W, LVGL_COMPARE_SAMPLE_H);
    lv_obj_t * zoom_frame = create_frame(panel, zoom_frame_x, body_y + LVGL_COMPARE_SAMPLE_H + 4, zoom_w, zoom_h);
    lv_obj_t * native_img = lv_image_create(native_frame);
    lv_obj_t * zoom_img = lv_image_create(zoom_frame);

    lv_image_set_src(native_img, native_buf);
    lv_obj_center(native_img);

    lv_image_set_src(zoom_img, zoom_buf);
    lv_obj_center(zoom_img);
}

static lv_obj_t * create_panel(lv_obj_t * root, int32_t x, int32_t y, const char * title, const char * detail)
{
    lv_obj_t * panel = lv_obj_create(root);
    lv_obj_t * title_label = lv_label_create(panel);
    lv_obj_t * detail_label = lv_label_create(panel);

    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, LVGL_COMPARE_PANEL_W, LVGL_COMPARE_PANEL_H);
    lv_obj_set_style_radius(panel, 8, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_set_style_bg_color(panel, color_hex(0xFDFDFF), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, color_hex(0xCBD5E1), 0);

    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title_label, color_hex(0x0F172A), 0);
    lv_obj_set_pos(title_label, 8, 7);

    lv_label_set_text(detail_label, detail);
    lv_obj_set_style_text_font(detail_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(detail_label, color_hex(0x475569), 0);
    lv_obj_set_pos(detail_label, 8, 19);

    return panel;
}

static void draw_compare_screen(const lv_font_t * latin11, const lv_font_t * latin14, const lv_font_t * cjk11)
{
    lv_obj_t * scr = lv_screen_active();
    lv_obj_t * title = lv_label_create(scr);
    lv_obj_t * subtitle = lv_label_create(scr);
    lv_obj_t * panel_builtin;
    lv_obj_t * panel_latin11;
    lv_obj_t * panel_latin14;
    lv_obj_t * panel_cjk11;
    lv_draw_buf_t * sample_builtin;
    lv_draw_buf_t * sample_latin11;
    lv_draw_buf_t * sample_latin14;
    lv_draw_buf_t * sample_cjk11;
    lv_draw_buf_t * zoom_builtin;
    lv_draw_buf_t * zoom_latin11;
    lv_draw_buf_t * zoom_latin14;
    lv_draw_buf_t * zoom_cjk11;

    lv_obj_set_style_bg_color(scr, color_hex(0xF4F6FA), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_label_set_text(title, "LVGL text render samples");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, color_hex(0x0F172A), 0);
    lv_obj_set_pos(title, 14, 8);

    lv_label_set_text(subtitle, "native 1x on top, zoom x3 with grid below");
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(subtitle, color_hex(0x475569), 0);
    lv_obj_set_pos(subtitle, 14, 24);

    panel_builtin = create_panel(scr, 12, 46, "Builtin Font", "Montserrat 12");
    panel_latin11 = create_panel(scr, 216, 46, "Tiny TTF", "DejaVuSans 11px");
    panel_latin14 = create_panel(scr, 12, 196, "Tiny TTF", "DejaVuSans 14px");
    panel_cjk11 = create_panel(scr, 216, 196, "Tiny TTF CJK", "SourceHan/Noto 11px");

    sample_builtin = snapshot_text_sample(scr, &lv_font_montserrat_12, "AVA/", "LVGL");
    sample_latin11 = snapshot_text_sample(scr, latin11, "AVA", "HAg");
    sample_latin14 = snapshot_text_sample(scr, latin14, "AVA", "HAg");
    sample_cjk11 = snapshot_text_sample(scr, cjk11, "AVA/", "中文");

    zoom_builtin = scale_draw_buf_3x(sample_builtin);
    zoom_latin11 = scale_draw_buf_3x(sample_latin11);
    zoom_latin14 = scale_draw_buf_3x(sample_latin14);
    zoom_cjk11 = scale_draw_buf_3x(sample_cjk11);

    mount_native_and_zoom(panel_builtin, sample_builtin, zoom_builtin);
    mount_native_and_zoom(panel_latin11, sample_latin11, zoom_latin11);
    mount_native_and_zoom(panel_latin14, sample_latin14, zoom_latin14);
    mount_native_and_zoom(panel_cjk11, sample_cjk11, zoom_cjk11);
}

static bool write_bmp_from_rgba(const char * path, uint8_t * rgba, uint32_t width, uint32_t height)
{
    SDL_Surface * surface = SDL_CreateRGBSurfaceWithFormatFrom(
        rgba,
        (int)width,
        (int)height,
        32,
        (int)(width * 4u),
        SDL_PIXELFORMAT_RGBA32
    );
    if(surface == NULL) return false;
    if(SDL_SaveBMP(surface, path) != 0) {
        SDL_FreeSurface(surface);
        return false;
    }
    SDL_FreeSurface(surface);
    return true;
}

int main(void)
{
    char cwd[4096];
    char out_path[4608];
    font_asset_t latin11;
    font_asset_t latin14;
    font_asset_t cjk11;
    const char * cjk_path = cjk_font_path;
    lv_display_t * disp;
    SDL_Renderer * renderer;
    uint8_t * rgba;
    bool write_ok;

    if(getenv("SDL_VIDEODRIVER") == NULL) {
        setenv("SDL_VIDEODRIVER", "dummy", 0);
    }

    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "failed to get cwd\n");
        return 1;
    }
    snprintf(out_path, sizeof(out_path), "%s/%s", cwd, LVGL_COMPARE_OUT_REL);

    if(!ensure_dir("build") || !ensure_dir("build/text_compare")) {
        fprintf(stderr, "failed to create output directory\n");
        return 1;
    }

    lv_init();
    disp = lv_sdl_window_create(LVGL_COMPARE_CANVAS_W, LVGL_COMPARE_CANVAS_H);
    if(disp == NULL) {
        fprintf(stderr, "failed to create lvgl sdl display\n");
        return 2;
    }
    lv_sdl_window_set_title(disp, "LVGL text compare");

    if(!load_tiny_ttf_font(&latin11, latin_font_path, 11)) {
        fprintf(stderr, "failed to load latin font %s\n", latin_font_path);
        return 3;
    }
    if(!load_tiny_ttf_font(&latin14, latin_font_path, 14)) {
        unload_tiny_ttf_font(&latin11);
        fprintf(stderr, "failed to load latin font 14px\n");
        return 4;
    }
    if(!load_tiny_ttf_font(&cjk11, cjk_path, 11)) {
        cjk_path = cjk_font_fallback_path;
        if(!load_tiny_ttf_font(&cjk11, cjk_path, 11)) {
            unload_tiny_ttf_font(&latin11);
            unload_tiny_ttf_font(&latin14);
            fprintf(stderr, "failed to load cjk font from %s or %s\n", cjk_font_path, cjk_font_fallback_path);
            return 5;
        }
    }

    draw_compare_screen(latin11.font, latin14.font, cjk11.font);
    lv_obj_update_layout(lv_screen_active());
    lv_refr_now(NULL);
    lv_timer_handler();

    renderer = lv_sdl_window_get_renderer(disp);
    if(renderer == NULL) {
        unload_tiny_ttf_font(&latin11);
        unload_tiny_ttf_font(&latin14);
        unload_tiny_ttf_font(&cjk11);
        fprintf(stderr, "failed to get sdl renderer\n");
        return 6;
    }

    rgba = malloc(LVGL_COMPARE_CANVAS_W * LVGL_COMPARE_CANVAS_H * 4u);
    if(rgba == NULL) {
        unload_tiny_ttf_font(&latin11);
        unload_tiny_ttf_font(&latin14);
        unload_tiny_ttf_font(&cjk11);
        fprintf(stderr, "failed to allocate capture buffer\n");
        return 6;
    }

    if(SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, rgba, LVGL_COMPARE_CANVAS_W * 4) != 0) {
        free(rgba);
        unload_tiny_ttf_font(&latin11);
        unload_tiny_ttf_font(&latin14);
        unload_tiny_ttf_font(&cjk11);
        fprintf(stderr, "failed to read sdl pixels: %s\n", SDL_GetError());
        return 6;
    }

    write_ok = write_bmp_from_rgba(out_path, rgba, LVGL_COMPARE_CANVAS_W, LVGL_COMPARE_CANVAS_H);
    free(rgba);
    unload_tiny_ttf_font(&latin11);
    unload_tiny_ttf_font(&latin14);
    unload_tiny_ttf_font(&cjk11);

    if(!write_ok) {
        fprintf(stderr, "failed to write bmp: %s\n", SDL_GetError());
        return 7;
    }

    printf("[lvgl-text-compare] wrote %s\n", out_path);
    return 0;
}
