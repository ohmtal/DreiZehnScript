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
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <cmath>

#include <stdlib.h>
#include <stdbool.h>

#include <fstream>
#include <cstdint>

#include "PixelBuffer.h"

namespace DreiZehn::PixelBuffer {



uint32_t pixel_buffer::get(uint32_t x, uint32_t y) {
    if (!buffer || x >= width || y >= height) return 0;
    return buffer[y * width + x];
}

bool pixel_buffer::set(uint32_t x, uint32_t y, uint32_t color) {
    if (!buffer || x >= width || y >= height) return false;
    buffer[y * width + x] = color;
    return true;
}

bool pixel_buffer::setAt(uint32_t index, uint32_t color) {
    if (!buffer || index >= (width * height)) return false;
    buffer[index] = color;
    return true;
}

void pixel_buffer::blit(uint32_t dest_x, uint32_t dest_y, const pixel_buffer& src) {
    if (!this->buffer || !src.buffer) return;

    uint32_t src_start_x = 0;
    uint32_t src_start_y = 0;

    uint32_t copy_width  = src.width;
    uint32_t copy_height = src.height;

    if (dest_x >= this->width || dest_y >= this->height) {
        return;
    }

    if (dest_x + copy_width > this->width) {
        copy_width = this->width - dest_x;
    }
    if (dest_y + copy_height > this->height) {
        copy_height = this->height - dest_y;
    }

    // Alpha 0xAARRGGBB
    for (uint32_t row = 0; row < copy_height; ++row) {
        uint32_t* dest_ptr = &this->buffer[(dest_y + row) * this->width + dest_x];
        const uint32_t* src_ptr = &src.buffer[(src_start_y + row) * src.width + src_start_x];

        for (uint32_t col = 0; col < copy_width; ++col) {
            uint32_t src_pixel = src_ptr[col];

            uint32_t alpha = (src_pixel >> 24) & 0xFF;

            if (alpha == 255) {
                dest_ptr[col] = src_pixel;
            }
            else if (alpha > 0) {
                uint32_t dest_pixel = dest_ptr[col];

                uint32_t rb_src  = src_pixel & 0x00FF00FF;
                uint32_t g_src   = src_pixel & 0x0000FF00;

                uint32_t rb_dest = dest_pixel & 0x00FF00FF;
                uint32_t g_dest  = dest_pixel & 0x0000FF00;

                uint32_t rb = rb_dest + (((rb_src - rb_dest) * alpha) >> 8);
                uint32_t g  = g_dest  + (((g_src  - g_dest)  * alpha) >> 8);

                dest_ptr[col] = 0xFF000000 | (rb & 0x00FF00FF) | (g & 0x0000FF00);
            }
        }
    }
}

void draw_scaled_pixel(pixel_buffer* f, int x, int y, int scale, uint32_t color) {
    for (int dy = 0; dy < scale; dy++) {
        for (int dx = 0; dx < scale; dx++) {
            int screen_x = x * scale + dx;
            int screen_y = y * scale + dy;

            if (screen_x >= 0 && screen_x < f->width && screen_y >= 0 && screen_y < f->height) {
                f->buffer[screen_y * f->width + screen_x] = color;
            }
        }
    }
}

void scale_buffer_to_window(pixel_buffer* f, uint32_t* src_buf, uint32_t src_w, uint32_t src_h) {
    if (!f || !f->buffer || !src_buf || f->width == 0 || f->height == 0) return;

    uint64_t step_x = (static_cast<uint64_t>(src_w) << 16) / f->width;
    uint64_t step_y = (static_cast<uint64_t>(src_h) << 16) / f->height;

    uint32_t* dest_ptr = f->buffer;
    uint64_t fp_y = 0;

    for (uint32_t y = 0; y < f->height; y++) {
        uint32_t src_y = fp_y >> 16;
        uint32_t* src_row_ptr = &src_buf[src_y * src_w];

        uint64_t fp_x = 0;
        for (uint32_t x = 0; x < f->width; x++) {
            uint32_t src_x = fp_x >> 16;
            *dest_ptr++ = src_row_ptr[src_x];
            fp_x += step_x;
        }
        fp_y += step_y;
    }
}

void scale_buffer_to_window(pixel_buffer *f,pixel_buffer *pb) {
    scale_buffer_to_window(f,pb->buffer, pb->width, pb->height);
}

void rotate90_clockwise(const pixel_buffer& src, pixel_buffer& dest) {
    dest.width = src.height;
    dest.height = src.width;
    const uint32_t TILE_SIZE = 32;

    for (uint32_t ty = 0; ty < src.height; ty += TILE_SIZE) {
        for (uint32_t tx = 0; tx < src.width; tx += TILE_SIZE) {
            for (uint32_t y = ty; y < std::min(ty + TILE_SIZE, src.height); ++y) {
                for (uint32_t x = tx; x < std::min(tx + TILE_SIZE, src.width); ++x) {
                    uint32_t src_idx = y * src.width + x;
                    uint32_t dest_idx = x * dest.width + (src.height - 1 - y);
                    dest.buffer[dest_idx] = src.buffer[src_idx];
                }
            }
        }
    }
}

void rotate_arbitrary(const pixel_buffer& src, pixel_buffer& dest, float angle_rad) {
    float cos_a = std::cos(angle_rad);
    float sin_a = std::sin(angle_rad);

    float cx = src.width / 2.0f;
    float cy = src.height / 2.0f;
    float dcx = dest.width / 2.0f;
    float dcy = dest.height / 2.0f;

    for (uint32_t dy = 0; dy < dest.height; ++dy) {
        for (uint32_t dx = 0; dx < dest.width; ++dx) {
            float x = dx - dcx;
            float y = dy - dcy;

            int sx = static_cast<int>(x * cos_a + y * sin_a + cx);
            int sy = static_cast<int>(-x * sin_a + y * cos_a + cy);

            if (sx >= 0 && sx < (int)src.width && sy >= 0 && sy < (int)src.height) {
                dest.buffer[dy * dest.width + dx] = src.buffer[sy * src.width + sx];
            } else {
                dest.buffer[dy * dest.width + dx] = 0;
            }
        }
    }
}

bool save_to_tga(const char* filename, const pixel_buffer& src) {
    if (!src.buffer || src.width == 0 || src.height == 0) return false;

    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    uint8_t header[18] = { 0 };

    header[2]  = 2;

    header[12] = src.width & 0xFF;
    header[13] = (src.width >> 8) & 0xFF;

    header[14] = src.height & 0xFF;
    header[15] = (src.height >> 8) & 0xFF;

    header[16] = 32;
    header[17] = 0x28;

    file.write(reinterpret_cast<char*>(header), 18);

    uint32_t num_pixels = src.width * src.height;
    file.write(reinterpret_cast<const char*>(src.buffer), num_pixels * sizeof(uint32_t));

    return file.good();
}

bool load_from_tga(const char* filename, pixel_buffer& dest) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    uint8_t header[18];
    file.read(reinterpret_cast<char*>(header), 18);
    if (!file) return false;

    if (header[2] != 2) return false;

    uint32_t w = header[12] | (header[13] << 8);
    uint32_t h = header[14] | (header[15] << 8);

    if (header[16] != 32) return false;

    if (dest.buffer) {
        delete[] dest.buffer;
    }

    dest.width = w;
    dest.height = h;
    dest.buffer = new uint32_t[w * h];

    bool flip_vertical = !(header[17] & 0x20);

    if (!flip_vertical) {
        file.read(reinterpret_cast<char*>(dest.buffer), w * h * sizeof(uint32_t));
    }
    else {
        for (int32_t row = h - 1; row >= 0; --row) {
            uint32_t* row_ptr = &dest.buffer[row * w];
            file.read(reinterpret_cast<char*>(row_ptr), w * sizeof(uint32_t));
        }
    }
    return file.good();
}

void pixel(pixel_buffer* f, int x, int y, uint32_t c) {
    if (!f || x < 0 || x >= f->width || y < 0  || y >= f->height ) return;
    f->set(x,y,c);
}

void line(pixel_buffer* f, int x0, int y0, int x1, int y1, uint32_t c){
   int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
   int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
   int err = (dx > dy ? dx : -dy) / 2, e2;
   for (;;) {
       pixel(f, x0, y0, c);
       if (x0 == x1 && y0 == y1) {
           break;
       }
       e2 = err;
       if (e2 > -dx) {
           err -= dy;
           x0 += sx;
       }
       if (e2 < dy) {
           err += dx;
           y0 += sy;
       }
   }
                       }

void rect(pixel_buffer* f, int x, int y, int w, int h, uint32_t c) {
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            pixel(f, x + col, y + row,  c);
        }
    }
}

void circle(pixel_buffer* f, int x, int y, int r, uint32_t c) {
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                pixel(f, x + dx, y + dy, c);
            }
        }
    }
}

void fill(pixel_buffer* f, int start_x, int start_y, uint32_t old, uint32_t c) {
    if (old == c) return;
    if (start_x < 0 || start_y < 0 || start_x >= f->width || start_y >= f->height) return;
    if (f->get(start_x, start_y) != old) return;

    int capacity = 256;
    int top = 0;
    ScanlinePoint *stack = (ScanlinePoint *)malloc(capacity * sizeof(ScanlinePoint));

    if (!stack) return;

    stack[top++] = {start_x, start_y};

    while (top > 0) {
        ScanlinePoint p = stack[--top];
        int x = p.x;
        int y = p.y;

        while (x >= 0 && f->get(x, y) == old) {
            x--;
        }
        x++;

        bool span_above = false;
        bool span_below = false;

        while (x < f->width && f->get(x, y) == old) {
            f->set(x, y, c);

            if (y > 0) {
                if (!span_above && f->get(x, y - 1) == old) {
                    if (top >= capacity) {
                        capacity *= 2;
                        ScanlinePoint *new_stack = (ScanlinePoint *)realloc(stack, capacity * sizeof(ScanlinePoint));
                        if (!new_stack) { free(stack); return; }
                        stack = new_stack;
                    }
                    stack[top++] = {x, y - 1};
                    span_above = true;
                } else if (span_above && f->get(x, y - 1) != old) {
                    span_above = false;
                }
            }

            if (y < f->height - 1) {
                if (!span_below && f->get(x, y + 1) == old) {
                    if (top >= capacity) {
                        capacity *= 2;
                        ScanlinePoint *new_stack = (ScanlinePoint *)realloc(stack, capacity * sizeof(ScanlinePoint));
                        if (!new_stack) { free(stack); return; }
                        stack = new_stack;
                    }
                    stack[top++] = { start_x, start_y };
                    span_below = true;
                } else if (span_below && f->get(x, y + 1) != old) {
                    span_below = false;
                }
            }
            x++;
        }
    }
    free(stack);
}

void text(pixel_buffer* f, int x, int y, const char* s, int scale, uint32_t c) {
    while (*s) {
        char chr = *s++;
        if (chr > 32) {
            uint16_t bmp = font5x3[chr - 32];
            for (int dy = 0; dy < 5; dy++) {
                for (int dx = 0; dx < 3; dx++) {
                    if (bmp >> (dy * 3 + dx) & 1) {
                        rect(f, x + dx * scale, y + dy * scale, scale, scale, c);
                    }
                }
            }
        }
        x = x + 4 * scale;
    }
}

int save_to_bmp(const char* filename, pixel_buffer* f) {
    if (!f || !f->buffer || f->width <= 0 || f->height <= 0) return -1;

    FILE *file = fopen(filename, "wb");
    if (!file) return -1;

    int width = f->width;
    int height = f->height;

    int row_padded_size = (width * 3 + 3) & ~3;
    int pixel_array_size = row_padded_size * height;
    int file_size = 54 + pixel_array_size;

    // 1. 14-Byte Bitmap File Header
    uint8_t file_header[] = {
        'B', 'M',
        (uint8_t)(file_size & 0xFF),
        (uint8_t)((file_size >> 8) & 0xFF),
        (uint8_t)((file_size >> 16) & 0xFF),
        (uint8_t)((file_size >> 24) & 0xFF),
        0, 0, 0, 0,
        54, 0, 0, 0
    };

    // 2. 40-Byte DIB Info Header (BITMAPINFOHEADER)
    uint8_t info_header[] = {
        40, 0, 0, 0,
        (uint8_t)(width & 0xFF), (uint8_t)((width >> 8) & 0xFF), (uint8_t)((width >> 16) & 0xFF), (uint8_t)((width >> 24) & 0xFF),
        (uint8_t)(height & 0xFF), (uint8_t)((height >> 8) & 0xFF), (uint8_t)((height >> 16) & 0xFF), (uint8_t)((height >> 24) & 0xFF),
        1, 0,
        24, 0,
        0, 0, 0, 0,
        (uint8_t)(pixel_array_size & 0xFF), (uint8_t)((pixel_array_size >> 8) & 0xFF), (uint8_t)((pixel_array_size >> 16) & 0xFF), (uint8_t)((pixel_array_size >> 24) & 0xFF),
        0x13, 0x0B, 0, 0,
        0x13, 0x0B, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    fwrite(file_header, 1, 14, file);
    fwrite(info_header, 1, 40, file);

    // write pixels bottom up
    uint8_t padding[3] = {0, 0, 0};

    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            uint32_t pixel = f->get(x,y);

            uint8_t b = pixel & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t r = (pixel >> 16) & 0xFF;

            fputc(b, file);
            fputc(g, file);
            fputc(r, file);
        }
        int current_row_bytes = width * 3;
        int padding_needed = row_padded_size - current_row_bytes;
        if (padding_needed > 0) {
            fwrite(padding, 1, padding_needed, file);
        }
    }

    fclose(file);
    return 0; // success
}




} // namespace DreiZehn::PixelBuffer




