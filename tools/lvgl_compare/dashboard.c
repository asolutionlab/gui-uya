#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>

#include "lvgl.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/libs/tiny_ttf/lv_tiny_ttf.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define LVGL_DASHBOARD_OUT_REL "build/dashboard_compare/lvgl_dashboard.bmp"
#define LVGL_DASHBOARD_W 640
#define LVGL_DASHBOARD_H 480
#define LVGL_DASHBOARD_FONT_PX 14
#define LVGL_DASHBOARD_DEFAULT_FRAMES 120u

#define DASH_PAGE_BG 0x1B2430u
#define DASH_SHELL_BG 0x1B2430u
#define DASH_TITLE_BG 0x101418u
#define DASH_GRID_BG 0x0F172Au
#define DASH_BORDER 0x334155u
#define DASH_TEXT 0xF8FAFCu
#define DASH_BUTTON_BG 0x2563EBu
#define DASH_CELL_BORDER 0xEAEFF5u
#define DASH_AXIS 0xE4E8F0u

typedef struct {
    uint8_t * data;
    size_t size;
    lv_font_t * font;
} font_asset_t;

typedef struct {
    double avg_us;
    double max_us;
} render_profile_t;

static const char * dashboard_font_paths[] = {
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
    "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
    "/usr/share/fonts/opentype/source-han-cjk/SourceHanSansSC-Regular.otf",
    "src/gui/render/DejaVuSans.ttf",
};

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

static bool load_dashboard_font(font_asset_t * asset, const char ** used_path)
{
    size_t i;

    for(i = 0; i < sizeof(dashboard_font_paths) / sizeof(dashboard_font_paths[0]); i++) {
        if(load_tiny_ttf_font(asset, dashboard_font_paths[i], LVGL_DASHBOARD_FONT_PX)) {
            if(used_path) *used_path = dashboard_font_paths[i];
            return true;
        }
    }

    memset(asset, 0, sizeof(*asset));
    if(used_path) *used_path = "lv_font_montserrat_14";
    return false;
}

static void unload_tiny_ttf_font(font_asset_t * asset)
{
    if(asset == NULL) return;
    if(asset->font) lv_tiny_ttf_destroy(asset->font);
    free(asset->data);
    memset(asset, 0, sizeof(*asset));
}

static uint32_t env_u32(const char * name, uint32_t fallback, uint32_t max_value)
{
    const char * value = getenv(name);
    char * end = NULL;
    unsigned long parsed;

    if(value == NULL || value[0] == '\0') return fallback;
    parsed = strtoul(value, &end, 10);
    if(end == value) return fallback;
    if(parsed > max_value) return max_value;
    return (uint32_t)parsed;
}

static bool env_bool(const char * name, bool fallback)
{
    const char * value = getenv(name);

    if(value == NULL || value[0] == '\0') return fallback;
    if(strcmp(value, "0") == 0 || strcmp(value, "false") == 0 || strcmp(value, "off") == 0) return false;
    return true;
}

static lv_color_t color_hex(uint32_t hex)
{
    return lv_color_hex(hex);
}

static void clean_obj(lv_obj_t * obj)
{
    lv_obj_remove_style_all(obj);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void set_box_style(lv_obj_t * obj, uint32_t bg, uint32_t border, int32_t border_w, int32_t radius)
{
    lv_obj_set_style_bg_color(obj, color_hex(bg), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, color_hex(border), 0);
    lv_obj_set_style_border_width(obj, border_w, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

static const char * metric_title(uint32_t metric)
{
    if(metric == 1u) return "Ops Dashboard / NET";
    if(metric == 2u) return "Ops Dashboard / MEM";
    return "Ops Dashboard / CPU";
}

static void metric_values(uint32_t metric, int32_t values[5])
{
    if(metric == 1u) {
        values[0] = 26;
        values[1] = 32;
        values[2] = 24;
        values[3] = 38;
        values[4] = 30;
    } else if(metric == 2u) {
        values[0] = 20;
        values[1] = 22;
        values[2] = 28;
        values[3] = 31;
        values[4] = 36;
    } else {
        values[0] = 18;
        values[1] = 26;
        values[2] = 21;
        values[3] = 34;
        values[4] = 29;
    }
}

static void metric_min_max(const int32_t values[5], int32_t * min_out, int32_t * max_out)
{
    int32_t min_v = values[0];
    int32_t max_v = values[0];
    int i;

    for(i = 1; i < 5; i++) {
        if(values[i] < min_v) min_v = values[i];
        if(values[i] > max_v) max_v = values[i];
    }

    *min_out = min_v;
    *max_out = max_v;
}

static lv_obj_t * create_text_label(lv_obj_t * parent, const char * text, const lv_font_t * font, uint32_t color)
{
    lv_obj_t * label = lv_label_create(parent);
    clean_obj(label);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color_hex(color), 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    lv_obj_set_style_pad_all(label, 0, 0);
    return label;
}

static void create_title(lv_obj_t * shell, const lv_font_t * font, uint32_t metric)
{
    lv_obj_t * title_box = lv_obj_create(shell);
    lv_obj_t * title_label;

    clean_obj(title_box);
    lv_obj_set_pos(title_box, 28, 24);
    lv_obj_set_size(title_box, 240, 32);
    set_box_style(title_box, DASH_TITLE_BG, DASH_TITLE_BG, 0, 0);

    title_label = create_text_label(title_box, metric_title(metric), font, DASH_TEXT);
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 0, 0);
}

static void create_axis_line(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_t * axis = lv_obj_create(parent);
    clean_obj(axis);
    lv_obj_set_pos(axis, x, y);
    lv_obj_set_size(axis, w, h);
    set_box_style(axis, DASH_AXIS, DASH_AXIS, 0, 0);
}

static void create_chart(lv_obj_t * shell, uint32_t metric)
{
    int32_t values[5];
    int32_t min_v;
    int32_t max_v;
    lv_obj_t * chart = lv_chart_create(shell);
    lv_chart_series_t * series;

    metric_values(metric, values);
    metric_min_max(values, &min_v, &max_v);

    clean_obj(chart);
    lv_obj_set_pos(chart, 28, 88);
    lv_obj_set_size(chart, 336, 248);
    set_box_style(chart, DASH_SHELL_BG, DASH_BORDER, 1, 6);
    lv_obj_set_style_pad_all(chart, 10, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, 5);
    lv_chart_set_div_line_count(chart, 0, 0);
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, min_v, max_v);
    lv_obj_set_style_line_color(chart, color_hex(DASH_TEXT), LV_PART_ITEMS);
    lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);
    lv_obj_set_style_line_opa(chart, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);

    series = lv_chart_add_series(chart, color_hex(DASH_TEXT), LV_CHART_AXIS_PRIMARY_Y);
    if(series) {
        lv_chart_set_series_values(chart, series, values, 5);
        lv_chart_refresh(chart);
    }

    create_axis_line(shell, 38, 325, 316, 1);
    create_axis_line(shell, 38, 98, 1, 228);
}

static void create_button(lv_obj_t * cell, const char * text, const lv_font_t * font)
{
    lv_obj_t * btn = lv_button_create(cell);
    lv_obj_t * label;

    clean_obj(btn);
    lv_obj_set_pos(btn, 16, 12);
    lv_obj_set_size(btn, 96, 36);
    set_box_style(btn, DASH_BUTTON_BG, DASH_BUTTON_BG, 0, 8);
    lv_obj_set_style_pad_all(btn, 8, 0);

    label = create_text_label(btn, text, font, DASH_TEXT);
    lv_obj_center(label);
}

static void create_grid_cell(lv_obj_t * grid, int32_t y, const char * text, const lv_font_t * font)
{
    lv_obj_t * cell = lv_obj_create(grid);
    clean_obj(cell);
    lv_obj_set_pos(cell, 24, y);
    lv_obj_set_size(cell, 128, 60);
    lv_obj_set_style_bg_opa(cell, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(cell, color_hex(DASH_CELL_BORDER), 0);
    lv_obj_set_style_border_width(cell, 1, 0);
    lv_obj_set_style_radius(cell, 6, 0);
    lv_obj_set_style_pad_all(cell, 0, 0);
    create_button(cell, text, font);
}

static void create_grid(lv_obj_t * shell, const lv_font_t * font)
{
    lv_obj_t * grid = lv_obj_create(shell);

    clean_obj(grid);
    lv_obj_set_pos(grid, 392, 88);
    lv_obj_set_size(grid, 176, 232);
    set_box_style(grid, DASH_GRID_BG, DASH_BORDER, 1, 3);

    create_grid_cell(grid, 20, "CPU", font);
    create_grid_cell(grid, 86, "NET", font);
    create_grid_cell(grid, 152, "MEM", font);
}

static void draw_dashboard_screen(const lv_font_t * font, uint32_t metric)
{
    lv_obj_t * scr = lv_screen_active();
    lv_obj_t * page;
    lv_obj_t * shell;

    clean_obj(scr);
    lv_obj_set_style_bg_color(scr, color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    page = lv_obj_create(scr);
    clean_obj(page);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_size(page, LVGL_DASHBOARD_W, LVGL_DASHBOARD_H);
    set_box_style(page, DASH_PAGE_BG, DASH_BORDER, 1, 6);

    shell = lv_obj_create(page);
    clean_obj(shell);
    lv_obj_set_pos(shell, 24, 24);
    lv_obj_set_size(shell, 592, 432);
    set_box_style(shell, DASH_SHELL_BG, DASH_BORDER, 1, 6);

    create_title(shell, font, metric);
    create_chart(shell, metric);
    create_grid(shell, font);
}

static uint32_t us_to_ms_rounded(double us)
{
    if(us <= 0.0) return 0u;
    return (uint32_t)((us + 500.0) / 1000.0);
}

static render_profile_t benchmark_refresh(
    lv_display_t * disp,
    const lv_font_t * font,
    uint32_t metric,
    uint32_t frames,
    bool rebuild_each_frame
)
{
    uint64_t freq;
    uint32_t i;
    double total_us = 0.0;
    double max_us = 0.0;
    render_profile_t profile;

    profile.avg_us = 0.0;
    profile.max_us = 0.0;
    if(frames == 0u) return profile;

    lv_obj_update_layout(lv_screen_active());
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(disp);
    lv_timer_handler();

    freq = SDL_GetPerformanceFrequency();
    if(freq == 0u) return profile;

    for(i = 0; i < frames; i++) {
        uint64_t start;
        uint64_t end;
        double frame_us;

        start = SDL_GetPerformanceCounter();
        if(rebuild_each_frame) {
            lv_obj_clean(lv_screen_active());
            draw_dashboard_screen(font, metric);
        } else {
            lv_obj_invalidate(lv_screen_active());
        }
        lv_refr_now(disp);
        lv_timer_handler();
        end = SDL_GetPerformanceCounter();

        frame_us = ((double)(end - start) * 1000000.0) / (double)freq;
        total_us += frame_us;
        if(frame_us > max_us) max_us = frame_us;
    }

    profile.avg_us = total_us / (double)frames;
    profile.max_us = max_us;
    return profile;
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

static bool capture_display(lv_display_t * disp, const char * out_path)
{
    SDL_Renderer * renderer;
    uint8_t * rgba;
    bool write_ok;

    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(disp);
    lv_timer_handler();

    renderer = lv_sdl_window_get_renderer(disp);
    if(renderer == NULL) {
        fprintf(stderr, "failed to get sdl renderer\n");
        return false;
    }

    rgba = malloc(LVGL_DASHBOARD_W * LVGL_DASHBOARD_H * 4u);
    if(rgba == NULL) {
        fprintf(stderr, "failed to allocate capture buffer\n");
        return false;
    }

    if(SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, rgba, LVGL_DASHBOARD_W * 4) != 0) {
        fprintf(stderr, "failed to read sdl pixels: %s\n", SDL_GetError());
        free(rgba);
        return false;
    }

    write_ok = write_bmp_from_rgba(out_path, rgba, LVGL_DASHBOARD_W, LVGL_DASHBOARD_H);
    free(rgba);
    return write_ok;
}

int main(void)
{
    char cwd[4096];
    char default_out[4608];
    const char * out_path = getenv("LVGL_DASHBOARD_OUT");
    const char * font_path = NULL;
    const lv_font_t * dashboard_font;
    font_asset_t font_asset;
    lv_display_t * disp;
    uint32_t metric = env_u32("LVGL_DASHBOARD_METRIC", 0u, 2u);
    uint32_t frames = env_u32("LVGL_DASHBOARD_FRAMES", LVGL_DASHBOARD_DEFAULT_FRAMES, 1000000u);
    bool rebuild_each_frame = env_bool("LVGL_DASHBOARD_REBUILD", false);
    bool capture = env_bool("LVGL_DASHBOARD_CAPTURE", true);
    render_profile_t profile;
    uint64_t startup_begin;
    uint64_t startup_end;
    uint64_t startup_freq;
    uint64_t startup_us = 0u;

    if(getenv("SDL_VIDEODRIVER") == NULL) {
        setenv("SDL_VIDEODRIVER", "dummy", 0);
    }

    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "failed to get cwd\n");
        return 1;
    }
    snprintf(default_out, sizeof(default_out), "%s/%s", cwd, LVGL_DASHBOARD_OUT_REL);
    if(out_path == NULL || out_path[0] == '\0') {
        out_path = default_out;
    }

    if(!ensure_dir("build") || !ensure_dir("build/dashboard_compare")) {
        fprintf(stderr, "failed to create output directory\n");
        return 1;
    }

    startup_begin = SDL_GetPerformanceCounter();
    lv_init();
    disp = lv_sdl_window_create(LVGL_DASHBOARD_W, LVGL_DASHBOARD_H);
    if(disp == NULL) {
        fprintf(stderr, "failed to create lvgl sdl display\n");
        return 2;
    }
    lv_sdl_window_set_title(disp, "LVGL dashboard compare");

    if(load_dashboard_font(&font_asset, &font_path)) {
        dashboard_font = font_asset.font;
    } else {
        dashboard_font = &lv_font_montserrat_14;
        fprintf(stderr, "warning: using LVGL built-in dashboard font\n");
    }

    draw_dashboard_screen(dashboard_font, metric);
    lv_obj_update_layout(lv_screen_active());
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(disp);
    lv_timer_handler();
    startup_end = SDL_GetPerformanceCounter();
    startup_freq = SDL_GetPerformanceFrequency();
    if(startup_freq != 0u) {
        startup_us = ((startup_end - startup_begin) * 1000000u) / startup_freq;
    }

    profile = benchmark_refresh(disp, dashboard_font, metric, frames, rebuild_each_frame);

    if(capture) {
        if(!capture_display(disp, out_path)) {
            unload_tiny_ttf_font(&font_asset);
            fprintf(stderr, "failed to write bmp: %s\n", SDL_GetError());
            return 3;
        }
        printf("[lvgl-dashboard-compare] wrote %s\n", out_path);
    }

    printf(
        "[lvgl-dashboard-compare] display=%dx%d font=%s px=%d metric=%u mode=%s capture=%s\n"
        "[lvgl-dashboard-compare] startup=%llu us\n"
        "[lvgl profiler] frames=%u frame(avg/max)=%u/%u ms update=0 ms render=%u ms present=0 ms\n",
        LVGL_DASHBOARD_W,
        LVGL_DASHBOARD_H,
        font_path ? font_path : "lv_font_montserrat_14",
        LVGL_DASHBOARD_FONT_PX,
        metric,
        rebuild_each_frame ? "rebuild" : "retained",
        capture ? "on" : "off",
        (unsigned long long)startup_us,
        frames,
        us_to_ms_rounded(profile.avg_us),
        us_to_ms_rounded(profile.max_us),
        us_to_ms_rounded(profile.avg_us)
    );

    unload_tiny_ttf_font(&font_asset);
    return 0;
}
