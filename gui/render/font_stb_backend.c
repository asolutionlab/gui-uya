#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_rect_pack.h"
#include "stb_truetype_htcw.h"

typedef struct UyaStbFontBackend {
    stbtt_fontinfo info;
    const unsigned char * data;
    size_t data_len;
    uint32_t sfnt_offset;
    float scale;
    int pixel_height;
    int ascent;
    int descent;
    int line_gap;
} UyaStbFontBackend;

static int uya_stb_round_to_i32(float value)
{
    if(value >= 0.0f) return (int)(value + 0.5f);
    return (int)(value - 0.5f);
}

void * uya_c_stb_font_create(const unsigned char * data, size_t data_len, uint32_t sfnt_offset, uint16_t pixel_height)
{
    UyaStbFontBackend * backend;

    if(data == NULL || data_len < 12u || pixel_height == 0u) return NULL;
    if((size_t)sfnt_offset >= data_len) return NULL;

    backend = (UyaStbFontBackend *)malloc(sizeof(UyaStbFontBackend));
    if(backend == NULL) return NULL;
    memset(backend, 0, sizeof(*backend));

    if(!stbtt_InitFont(&backend->info, data, (int)sfnt_offset)) {
        free(backend);
        return NULL;
    }

    backend->data = data;
    backend->data_len = data_len;
    backend->sfnt_offset = sfnt_offset;
    backend->pixel_height = (int)pixel_height;
    backend->scale = stbtt_ScaleForPixelHeight(&backend->info, (float)pixel_height);
    stbtt_GetFontVMetrics(&backend->info, &backend->ascent, &backend->descent, &backend->line_gap);
    return backend;
}

int uya_c_stb_find_matching_font_offset(const unsigned char * data, size_t data_len, const char * name_utf8)
{
    int offset;

    if(data == NULL || data_len < 12u || name_utf8 == NULL || name_utf8[0] == '\0') return -1;
    offset = stbtt_FindMatchingFont(data, name_utf8, STBTT_MACSTYLE_DONTCARE);
    if(offset < 0) return -1;
    if((size_t)offset + 12u > data_len) return -1;
    return offset;
}

void uya_c_stb_font_destroy(void * handle)
{
    if(handle != NULL) free(handle);
}

int uya_c_stb_font_find_glyph_index(void * handle, uint32_t codepoint)
{
    UyaStbFontBackend * backend = (UyaStbFontBackend *)handle;
    if(backend == NULL) return 0;
    return stbtt_FindGlyphIndex(&backend->info, (int)codepoint);
}

int uya_c_stb_font_get_glyph_metrics(void * handle, uint32_t codepoint, uint16_t * glyph_id_out, int * advance_px_out,
                                     int * x1_out, int * y1_out, int * x2_out, int * y2_out)
{
    UyaStbFontBackend * backend = (UyaStbFontBackend *)handle;
    int glyph_id;
    int advance_units;
    int lsb;
    int x1;
    int y1;
    int x2;
    int y2;

    if(backend == NULL) return 0;

    glyph_id = stbtt_FindGlyphIndex(&backend->info, (int)codepoint);
    if(glyph_id == 0) return 0;

    stbtt_GetGlyphHMetrics(&backend->info, glyph_id, &advance_units, &lsb);
    stbtt_GetGlyphBitmapBox(&backend->info, glyph_id, backend->scale, backend->scale, &x1, &y1, &x2, &y2);
    (void)lsb;

    if(glyph_id_out) *glyph_id_out = (uint16_t)glyph_id;
    if(advance_px_out) *advance_px_out = uya_stb_round_to_i32(backend->scale * (float)advance_units);
    if(x1_out) *x1_out = x1;
    if(y1_out) *y1_out = y1;
    if(x2_out) *x2_out = x2;
    if(y2_out) *y2_out = y2;
    return 1;
}

int uya_c_stb_font_get_kerning_adjust(void * handle, uint32_t left_codepoint, uint32_t right_codepoint)
{
    UyaStbFontBackend * backend = (UyaStbFontBackend *)handle;
    int left_glyph;
    int right_glyph;
    int advance_units;
    int lsb;
    int kerning_units;
    int base_px;
    int kerned_px;

    if(backend == NULL) return 0;

    left_glyph = stbtt_FindGlyphIndex(&backend->info, (int)left_codepoint);
    right_glyph = stbtt_FindGlyphIndex(&backend->info, (int)right_codepoint);
    if(left_glyph == 0 || right_glyph == 0) return 0;

    stbtt_GetGlyphHMetrics(&backend->info, left_glyph, &advance_units, &lsb);
    kerning_units = stbtt_GetGlyphKernAdvance(&backend->info, left_glyph, right_glyph);
    base_px = uya_stb_round_to_i32(backend->scale * (float)advance_units);
    kerned_px = uya_stb_round_to_i32(backend->scale * (float)(advance_units + kerning_units));
    (void)lsb;
    return kerned_px - base_px;
}

int uya_c_stb_font_render_glyph(void * handle, uint16_t glyph_id, unsigned char * dst, size_t dst_stride,
                                int width, int height, int hint_enabled)
{
    UyaStbFontBackend * backend = (UyaStbFontBackend *)handle;
    int row;

    if(backend == NULL || dst == NULL || dst_stride == 0u) return 0;
    if(width <= 0 || height <= 0) return 1;

    for(row = 0; row < height; row++) {
        memset(dst + (size_t)row * dst_stride, 0, (size_t)width);
    }

    if(hint_enabled != 0) {
        stbtt_MakeGlyphBitmap(&backend->info, dst, width, height, (int)dst_stride, backend->scale, backend->scale, glyph_id);
    }
    else {
        stbtt_MakeGlyphBitmapSubpixel(&backend->info, dst, width, height, (int)dst_stride,
                                      backend->scale, backend->scale, 0.35f, 0.15f, glyph_id);
    }
    return 1;
}
