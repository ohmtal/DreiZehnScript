//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// Copyright (c) https://github.com/zserge
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// PixelBuffer
// - draw functions
// - rotate functions
// - blit
// - tga (uncompressed) import / export
// - bmp export
//-----------------------------------------------------------------------------
// draw functions borrowed from drawing-c:
// https://github.com/zserge/fenster/blob/main/examples/drawing-c/main.c)
// * - Line: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
// * - Circle: https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
// * - Rectangle: well, it's obvious
// * ** replaced ** - Flood fill: very inefficient, using recursion
// * - Text: small 5x3 bitmap font is used to render glyps
//-----------------------------------------------------------------------------
#pragma once


namespace DreiZehn::PixelBuffer {


struct pixel_buffer {
    uint32_t width   = 0;
    uint32_t height  = 0;
    uint32_t* buffer = nullptr;


    uint32_t get(uint32_t x, uint32_t y);
    bool set(uint32_t x, uint32_t y, uint32_t color);
    bool setAt(uint32_t index, uint32_t color);
    void blit(uint32_t dest_x, uint32_t dest_y, const pixel_buffer& src);

}; // pixel buffer
// -------------------------------------------------------------------------
// Scale
// -------------------------------------------------------------------------
// ------------------------------------------------------------------------
void draw_scaled_pixel(pixel_buffer *f, int x, int y, int scale, uint32_t color);
void scale_buffer_to_window(pixel_buffer *f, uint32_t *src_buf, uint32_t src_w, uint32_t src_h);
void scale_buffer_to_window(pixel_buffer *f,pixel_buffer *pb);
// ------------------------------------------------------------------------

// -------------------------------------------------------------------------
// Rotation
// -------------------------------------------------------------------------
 void rotate90_clockwise(const pixel_buffer& src, pixel_buffer& dest);

 void rotate_arbitrary(const pixel_buffer& src, pixel_buffer& dest, float angle_rad);

// -------------------------------------------------------------------------
// TGA load / save
// -------------------------------------------------------------------------
 bool save_to_tga(const char* filename, const pixel_buffer& src);

// -------------------------------------------------------------------------
 bool load_from_tga(const char* filename, pixel_buffer& dest);

// -------------------------------------------------------------------------
void pixel(pixel_buffer* f, int x, int y, uint32_t c);
// -------------------------------------------------------------------------
// from drawing-c
// -------------------------------------------------------------------------
void line(pixel_buffer *f, int x0, int y0, int x1, int y1,
                        uint32_t c);
// -------------------------------------------------------------------------
void rect(pixel_buffer *f, int x, int y, int w, int h,
                        uint32_t c);

void circle(pixel_buffer *f, int x, int y, int r, uint32_t c);

// -------------------------------------------------------------------------
// fast save fill replacement using scanlines
// -------------------------------------------------------------------------
typedef struct {
    int x, y;
} ScanlinePoint;

// static  void fill_scanline(pixel_buffer *f, int start_x, int start_y, uint32_t old, uint32_t c) {
void fill(pixel_buffer *f, int start_x, int start_y, uint32_t old, uint32_t c);

// -------------------------------------------------------------------------
constexpr uint16_t font5x3[] = {0x0000,0x2092,0x002d,0x5f7d,0x279e,0x52a5,0x7ad6,0x0012,0x4494,0x1491,0x017a,0x05d0,0x1400,0x01c0,0x0400,0x12a4,0x2b6a,0x749a,0x752a,0x38a3,0x4f4a,0x38cf,0x3bce,0x12a7,0x3aae,0x49ae,0x0410,0x1410,0x4454,0x0e38,0x1511,0x10e3,0x73ee,0x5f7a,0x3beb,0x624e,0x3b6b,0x73cf,0x13cf,0x6b4e,0x5bed,0x7497,0x2b27,0x5add,0x7249,0x5b7d,0x5b6b,0x3b6e,0x12eb,0x4f6b,0x5aeb,0x388e,0x2497,0x6b6d,0x256d,0x5f6d,0x5aad,0x24ad,0x72a7,0x6496,0x4889,0x3493,0x002a,0xf000,0x0011,0x6b98,0x3b79,0x7270,0x7b74,0x6750,0x95d6,0xb9ee,0x5b59,0x6410,0xb482,0x56e8,0x6492,0x5be8,0x5b58,0x3b70,0x976a,0xcd6a,0x1370,0x38f0,0x64ba,0x3b68,0x2568,0x5f68,0x54a8,0xb9ad,0x73b8,0x64d6,0x2492,0x3593,0x03e0};
// -------------------------------------------------------------------------
void text(pixel_buffer *f, int x, int y, const char *s, int scale, uint32_t c);
// -------------------------------------------------------------------------
int save_to_bmp(const char *filename, pixel_buffer *f );
// ------------------------------------------------------------------------
} // namespace DreiZehn
