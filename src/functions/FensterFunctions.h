//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Fenster Commands
// FIXME: Register system for help!!!
// NOTE! Modified fenster.h!
//      - I had to remove const int width and height in fenster.h!
//      - added X11 wmDeleteMessage message so my prog does not crash when
//        Window is closed.
//-----------------------------------------------------------------------------
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <cstring>

#include "core/FunctionMap.h"
#include "core/VariableFrame.h"
#include "Globals.h"
#include "ArrayFunctions.h"

namespace DreiZehn::Fenster {
    #include "ext/fenster/fenster.h"

    struct pixel_buffer {
        uint32_t width   = 0;
        uint32_t height  = 0;
        uint32_t* buffer = nullptr;


        inline uint32_t get(uint32_t x, uint32_t y) {
            if (!buffer || x >= width || y >= height) return 0;
            return buffer[y * width + x];
        }

        inline void set(uint32_t x, uint32_t y, uint32_t color) {
            if (!buffer || x >= width || y >= height) return;
            buffer[y * width + x] = color;
        }
        inline void blit(uint32_t dest_x, uint32_t dest_y, const pixel_buffer& src) {
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

            for (uint32_t row = 0; row < copy_height; ++row) {
                uint32_t* dest_ptr = &this->buffer[(dest_y + row) * this->width + dest_x];
                const uint32_t* src_ptr = &src.buffer[(src_start_y + row) * src.width + src_start_x];
                std::memcpy(dest_ptr, src_ptr, copy_width * sizeof(uint32_t));
            }
        }
    };


    static inline void pixel(pixel_buffer* f, int x, int y, uint32_t c) {
        if (!f || x < 0 || x >= f->width || y < 0  || y >= f->height ) return;
        f->set(x,y,c);
    }

    // -------------------------------------------------------------------------
    // from drawing-c
    // -------------------------------------------------------------------------
    static inline void line(pixel_buffer *f, int x0, int y0, int x1, int y1,
                            uint32_t c) {
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
    // -------------------------------------------------------------------------
    static inline void rect(pixel_buffer *f, int x, int y, int w, int h,
                            uint32_t c) {
        for (int row = 0; row < h; row++) {
            for (int col = 0; col < w; col++) {
                pixel(f, x + col, y + row,  c);
            }
        }
    }

    static inline void circle(pixel_buffer *f, int x, int y, int r, uint32_t c) {
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                pixel(f, x + dx, y + dy, c);
            }
        }
    }
    }
    // -------------------------------------------------------------------------
    static inline void fill(pixel_buffer *f, int x, int y, uint32_t old,
                            uint32_t c) {
    if (x < 0 || y < 0 || x >= f->width || y >= f->height) {
        return;
    }
    if (f->get( x, y) == old) {
        f->set(x, y, c);
        fill(f, x - 1, y, old, c);
        fill(f, x + 1, y, old, c);
        fill(f, x, y - 1, old, c);
        fill(f, x, y + 1, old, c);
    }
    }
    // -------------------------------------------------------------------------
    constexpr uint16_t font5x3[] = {0x0000,0x2092,0x002d,0x5f7d,0x279e,0x52a5,0x7ad6,0x0012,0x4494,0x1491,0x017a,0x05d0,0x1400,0x01c0,0x0400,0x12a4,0x2b6a,0x749a,0x752a,0x38a3,0x4f4a,0x38cf,0x3bce,0x12a7,0x3aae,0x49ae,0x0410,0x1410,0x4454,0x0e38,0x1511,0x10e3,0x73ee,0x5f7a,0x3beb,0x624e,0x3b6b,0x73cf,0x13cf,0x6b4e,0x5bed,0x7497,0x2b27,0x5add,0x7249,0x5b7d,0x5b6b,0x3b6e,0x12eb,0x4f6b,0x5aeb,0x388e,0x2497,0x6b6d,0x256d,0x5f6d,0x5aad,0x24ad,0x72a7,0x6496,0x4889,0x3493,0x002a,0xf000,0x0011,0x6b98,0x3b79,0x7270,0x7b74,0x6750,0x95d6,0xb9ee,0x5b59,0x6410,0xb482,0x56e8,0x6492,0x5be8,0x5b58,0x3b70,0x976a,0xcd6a,0x1370,0x38f0,0x64ba,0x3b68,0x2568,0x5f68,0x54a8,0xb9ad,0x73b8,0x64d6,0x2492,0x3593,0x03e0};
    // -------------------------------------------------------------------------
    static inline void text(pixel_buffer *f, int x, int y, const char *s, int scale, uint32_t c) {
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
     // -------------------------------------------------------------------------
     int save_to_bmp(const char *filename, pixel_buffer *f ) {
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
     // ------------------------------------------------------------------------
     void scale_buffer_to_window(struct fenster *f, uint32_t *src_buf, uint32_t src_w, uint32_t src_h) {
         if (!f || !f->buf || !src_buf || f->width == 0 || f->height == 0) return;

         uint64_t step_x = (static_cast<uint64_t>(src_w) << 16) / f->width;
         uint64_t step_y = (static_cast<uint64_t>(src_h) << 16) / f->height;

         uint32_t* dest_ptr = f->buf;
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


     void scale_buffer_to_window(struct fenster *f,pixel_buffer *pb) {
            scale_buffer_to_window(f,pb->buffer, pb->width, pb->height);
     }

    // ------------------------------------------------------------------------


}


// =============================================================================
// --- FensterObject ---
// =============================================================================

namespace DreiZehn {
    using namespace DreiZehn::Fenster;
    const int TypeFensterObject = RegisterUserObjectType("Fenster");
    const int TypePixelBuffer = RegisterUserObjectType("PixelBuffer");


    // ......................... PixelBuffer
    struct PixelBufferObject: ValueObject {
        pixel_buffer mBuffer;

        // Methods
        inline static ValueObjectProperty  setPixelProp;
        inline static ValueObjectProperty clearProp, lineProp;
        inline static ValueObjectProperty rectProp, circleProp, fillProp;
        inline static ValueObjectProperty textProp, sleepProp, exportProp;
        inline static ValueObjectProperty printProp, setDataProp;

        // properties
        inline static ValueObjectProperty widthProp, heightProp;

        PixelBufferObject(int w, int h) : ValueObject(TypePixelBuffer) {
            mBuffer.width = w;
            mBuffer.height = h;
            mBuffer.buffer = new uint32_t[w * h]();
        }
        ~PixelBufferObject() {
            if (mBuffer.buffer) {
                delete[] mBuffer.buffer;
                mBuffer.buffer = nullptr;
            }
        }

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;


            setPixelProp = ValueObjectProperty("setPixel", 3,3
            , "set one pixel. @param int x int y uint color"
            , TypePixelBuffer);


            clearProp = ValueObjectProperty("clear", 0,1
            , "clear the buffer with a color. @params [uint color] default COLOR_WHITE "
            , TypePixelBuffer);

            lineProp = ValueObjectProperty("line", 5,5
            , "flood fill at point. @param int x0 int y0 int x1 int y1 uint color"
            , TypePixelBuffer);

            rectProp = ValueObjectProperty("rect", 5,5
            , "flood fill at point. @param int x int y int width int height uint color"
            , TypePixelBuffer);

            circleProp = ValueObjectProperty("circle", 4,4
            , "flood fill at point. @param int x int y int radius uint color"
            , TypePixelBuffer);

            fillProp = ValueObjectProperty("fill", 4,4
            , "flood fill at point. @param int x int y uint oldcolor uint color"
            , TypePixelBuffer);

            textProp = ValueObjectProperty("text", 5,5
            , "print a text, @param int x int y string text int scale uint color"
            , TypePixelBuffer);

            sleepProp = ValueObjectProperty("sleep", 1,1
            , "sleep for x ms , @param int ms", TypePixelBuffer);

            exportProp = ValueObjectProperty("export", 1,1
            , "export the picture to file, @param filename", TypePixelBuffer);

            printProp = ValueObjectProperty("print", 0,0
            , "print all pixels as hex numbers", TypePixelBuffer);

            setDataProp = ValueObjectProperty("setData", 1,65536
            , "read number params to set the pixels (max: 65536 == 256*256)", TypePixelBuffer);


            widthProp     = ValueObjectProperty("width","readonly", TypePixelBuffer);
            heightProp    = ValueObjectProperty("height","readonly", TypePixelBuffer);

            mSymbolsLoaded = true;
        }

        // -------------------------------------------------------------------------
        inline bool onGetField(uint32_t fieldSymbolId, Value& ret) override {

            if ( widthProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mBuffer.width));
                return true;
            }
            else
            if ( heightProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mBuffer.height));
                return true;
            }

            return ValueObject::onGetField(fieldSymbolId, ret);
        }
                // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override {
            // Read-Only!
            if ( widthProp.matchField(fieldSymbolId)) {
                return false;
            }
            else
            if ( heightProp.matchField(fieldSymbolId)) {
                return false;
            }

            return ValueObject::onSetField(fieldSymbolId, value);
        }
                // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId, std::vector<Value>& args, Value& ret) override {
            if (!mBuffer.buffer ) {
                return true;
            }


            // ------- setPixel
            if (setPixelProp.matchMethod( methodId , args) == 1) {
                int32_t x = args[0].getInt();
                int32_t y = args[1].getInt();

                uint32_t color = args[2].getUInt();

                mBuffer.set(x,y,color);
                return true;
            }
            else
            // ------- clear
            if (clearProp.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer  ) {
                    ret = Value(0);
                    return true;
                }
                uint32_t color = 0xFFFFFFFF;
                if (args.size() > 0) {
                    color = args[0].getUInt();
                }
                std::fill_n(mBuffer.buffer, mBuffer.width * mBuffer.height, color);
                return true;
            }
            else
            // ------- line
            if (lineProp.matchMethod( methodId , args) == 1) {
                line(&mBuffer, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getInt(),args[4].getUInt());
                return true;
            }
            else
            // ------- rect
            if (rectProp.matchMethod( methodId , args) == 1) {
                int x = args[0].getInt();
                int y = args[1].getInt();
                int w = args[2].getInt();
                int h = args[3].getInt();
                uint32_t c = args[4].getUInt();

                rect(&mBuffer, x,y,w,h,c);
                return true;
            }
            else
            // ------- circle
            if (circleProp.matchMethod( methodId , args) == 1) {
                circle(&mBuffer, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getUInt());
                return true;
            }
            else
            // ------- fill
            if (fillProp.matchMethod( methodId , args) == 1) {
                fill(&mBuffer, args[0].getInt(), args[1].getInt(), args[2].getUInt(), args[3].getUInt());
                return true;
            }
            else
            // ------- text
            if (textProp.matchMethod( methodId , args) == 1) {
                text(&mBuffer,
                                     args[0].getInt(), args[1].getInt(),
                                     args[2].getStringRef().c_str(),
                                     args[3].getInt(), args[4].getUInt());
                return true;
            }
            else

            // ------- export
            if (exportProp.matchMethod( methodId , args) == 1) {
                save_to_bmp(args[0].getStringRef().c_str(), &mBuffer);
                return true;
            }
            else
            // ------- print
            if (printProp.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer) return false;
                uint32_t count = mBuffer.width * mBuffer.height;
                for (uint32_t i = 0 ; i < count; i++) {
                    Tools::printf("%#x ",mBuffer.buffer[i]);
                }
                Tools::printf("\n");
                return true;
            }


            return ValueObject::onMethodCall(methodId, args, ret);
        }

    };
    // ......................... FensterObject

    struct FensterObject : public ValueObject {
        struct fenster mFenster = {0};
        float mScale = 1.0;
        // this is for scaling we have 2 buffers
        pixel_buffer mBuffer; // the pixel we write to
        int64_t mLastTime = 0;
        double  mFrameTime = 0.0;

        void Draw() {
            if (!mFenster.buf || !mBuffer.buffer ) {
                return;
            }
            scale_buffer_to_window(&mFenster, &mBuffer );
        }



        // Methods
        inline static ValueObjectProperty loopProp, setPixelProp, isKeyDownProp;
        inline static ValueObjectProperty closeProp, clearProp, lineProp;
        inline static ValueObjectProperty rectProp, circleProp, fillProp;
        inline static ValueObjectProperty textProp, sleepProp, exportProp;
        inline static ValueObjectProperty drawBufferProp;

        // Fields (read only)
        inline static ValueObjectProperty titleProp, widthProp, heightProp;
        inline static ValueObjectProperty mouseXProp, mouseYProp, mouseDownProp;
        inline static ValueObjectProperty timeProp, keymodProp;



        FensterObject(const char* title, int w, int h, float scale) : ValueObject(TypeFensterObject) {
            mFenster.title = title;
            if (scale < 0.f) scale == 1.f;
            mFenster.width =  (int)(w * scale);
            mFenster.height = (int)(h * scale);
            mScale = scale;
            mFenster.buf = new uint32_t[mFenster.width * mFenster.height]();

            mBuffer.width = w;
            mBuffer.height = h;
            mBuffer.buffer = new uint32_t[w * h]();

            fenster_open(&mFenster);
            mLastTime = fenster_time();
        }

        void unloadBuffers() {
            if (mFenster.buf) {
                fenster_close(&mFenster);
                delete[] mFenster.buf;
                mFenster.buf = nullptr;
            }
            if (mBuffer.buffer) {
                delete[] mBuffer.buffer;
                mBuffer.buffer = nullptr;
            }
        }

        ~FensterObject() {
            unloadBuffers();
        }

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            loopProp = ValueObjectProperty("loop", 0,1
            , "call the fenster loop"
            , TypeFensterObject);

            setPixelProp = ValueObjectProperty("setPixel", 3,3
            , "set one pixel. @param int x int y uint color"
            , TypeFensterObject);

            isKeyDownProp = ValueObjectProperty("isKeyDown", 1,1
            , "return if the given ASCII code key is pressed. @params int keycode"
            , TypeFensterObject);

            closeProp = ValueObjectProperty("close", 0,1
            , "close the window "
            , TypeFensterObject);

            clearProp = ValueObjectProperty("clear", 0,1
            , "clear the buffer with a color. @params [uint color] default COLOR_WHITE "
            , TypeFensterObject);

            lineProp = ValueObjectProperty("line", 5,5
            , "flood fill at point. @param int x0 int y0 int x1 int y1 uint color"
            , TypeFensterObject);

            rectProp = ValueObjectProperty("rect", 5,5
            , "flood fill at point. @param int x int y int width int height uint color"
            , TypeFensterObject);

            circleProp = ValueObjectProperty("circle", 4,4
            , "flood fill at point. @param int x int y int radius uint color"
            , TypeFensterObject);

            fillProp = ValueObjectProperty("fill", 4,4
            , "flood fill at point. @param int x int y uint oldcolor uint color"
            , TypeFensterObject);

            textProp = ValueObjectProperty("text", 5,5
            , "print a text, @param int x int y string text int scale uint color"
            , TypeFensterObject);

            sleepProp = ValueObjectProperty("sleep", 1,1
            , "sleep for x ms , @param int ms", TypeFensterObject);

            exportProp = ValueObjectProperty("export", 1,1
            , "export the picture to file, @param filename", TypeFensterObject);

            drawBufferProp = ValueObjectProperty("drawbuffer", 3,3
            , "draw a pixel buffer object at position", TypeFensterObject);


            titleProp     = ValueObjectProperty("title","readonly", TypeFensterObject);
            widthProp     = ValueObjectProperty("width","readonly", TypeFensterObject);
            heightProp    = ValueObjectProperty("height","readonly", TypeFensterObject);
            mouseXProp    = ValueObjectProperty("mouseX","readonly", TypeFensterObject);
            mouseYProp    = ValueObjectProperty("mouseY","readonly", TypeFensterObject);
            mouseDownProp = ValueObjectProperty("mouseDown","readonly", TypeFensterObject);
            timeProp      = ValueObjectProperty("time","readonly", TypeFensterObject);
            keymodProp    = ValueObjectProperty("mod","readonly: mod is 4 bits mask, ctrl=1, shift=2, alt=4, meta=8", TypeFensterObject);
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline bool onGetField(uint32_t fieldSymbolId, Value& ret) override {

            if (titleProp.matchField( fieldSymbolId)) {
                ret = Value(std::string(mFenster.title));
                return true;
            }
            else
            if (mouseXProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mFenster.x / mScale));
                return true;
            }
            else
            if ( mouseYProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mFenster.y / mScale));
                return true;
            }
            else
            if ( mouseDownProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mFenster.mouse));
                return true;
            }
            else
            if ( widthProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mBuffer.width));
                return true;
            }
            else
            if ( heightProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mBuffer.height));
                return true;
            }
            else
            if ( timeProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<double>(fenster_time()));
                return true;
            }
            else
            if ( keymodProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<uint32_t>(mFenster.mod));
                return true;
            }

            return ValueObject::onGetField(fieldSymbolId, ret);
        }
        // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override {
            // Read-Only!
            if (titleProp.matchField( fieldSymbolId)) {
                return false;
            }
            else
            if (mouseXProp.matchField(fieldSymbolId)) {
                return false;
            }
            else
            if ( mouseYProp.matchField(fieldSymbolId)) {
                return false;
            }
            else
            if ( mouseDownProp.matchField(fieldSymbolId)) {
                return false;
            }
            else
            if ( widthProp.matchField(fieldSymbolId)) {
                return false;
            }
            else
            if ( heightProp.matchField(fieldSymbolId)) {
                return false;
            }
            else
            if ( timeProp.matchField(fieldSymbolId)) {
                return false;
            }
            else
            if ( keymodProp.matchField(fieldSymbolId)) {
                return false;
            }

            return ValueObject::onSetField(fieldSymbolId, value);
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId, std::vector<Value>& args, Value& ret) override {
            if (!mFenster.buf || !mBuffer.buffer ) {
                Tools::errorf("Fenster Object is closed! Method: %s ignored.\n", SymbolTable::getName(methodId).c_str());
                return true; // return false says method not found!
            }


            // ------- loop
            if (loopProp.matchMethod( methodId , args) == 1) {
                Draw();
                int result = fenster_loop(&mFenster);

                static int64_t elapsed = 0;
                elapsed = fenster_time() - mLastTime;
                if (elapsed < 16) {
                    fenster_sleep(16 - elapsed);
                }
                mLastTime = fenster_time();

                ret = Value(static_cast<int32_t>(result == 0));
                return true;
            }
            else
            // ------- setPixel
            if (setPixelProp.matchMethod( methodId , args) == 1) {
                int32_t x = args[0].getInt();
                int32_t y = args[1].getInt();

                uint32_t color = args[2].getUInt();

                if (x >= 0 && x < mFenster.width && y >= 0 && y < mFenster.height) {
                    mBuffer.set(x,y,color);
                }
                return true;
            }
            else
            // ------- isKeyDown
            if (isKeyDownProp.matchMethod( methodId , args) == 1) {

                int32_t keyCode = args[0].getInt();

                if (keyCode >= 0 && keyCode < 256) {
                    int pressed = mFenster.keys[keyCode];
                    ret = Value(static_cast<int32_t>(pressed != 0));
                    return true;
                }
                ret = Value(0);
                return true;
            }
            // ------- close
            else
            if (closeProp.matchMethod( methodId , args) == 1) {
                unloadBuffers();
                ret = Value(1);
                return true;
            }

            else
            // ------- clear
            if (clearProp.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer || !mFenster.buf ) {
                    ret = Value(0);
                    return true;
                }
                uint32_t color = 0xFFFFFFFF;
                if (args.size() > 0) {
                    color = args[0].getUInt();
                }
                std::fill_n(mBuffer.buffer, mBuffer.width * mBuffer.height, color);
                return true;
            }
            else
            // ------- line
            if (lineProp.matchMethod( methodId , args) == 1) {
                line(&mBuffer, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getInt(),args[4].getUInt());
                return true;
            }
            else
            // ------- rect
            if (rectProp.matchMethod( methodId , args) == 1) {
                int x = args[0].getInt(); if (x < 0) return false;
                int y = args[1].getInt(); if (y < 0) return false;
                int w = args[2].getInt(); if (w < 0 || x+w > mFenster.width) return false;
                int h = args[3].getInt(); if (h < 0 || y+h > mFenster.height) return false;
                uint32_t c = args[4].getUInt();

                rect(&mBuffer, x,y,w,h,c);
                return true;
            }
            else
            // ------- circle
            if (circleProp.matchMethod( methodId , args) == 1) {
                circle(&mBuffer, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getUInt());
                return true;
            }
            else
            // ------- fill
            if (fillProp.matchMethod( methodId , args) == 1) {
                fill(&mBuffer, args[0].getInt(), args[1].getInt(), args[2].getUInt(), args[3].getUInt());
                return true;
            }
            else
            // ------- text
            if (textProp.matchMethod( methodId , args) == 1) {
                text(&mBuffer,
                                     args[0].getInt(), args[1].getInt(),
                                     args[2].getStringRef().c_str(),
                                     args[3].getInt(), args[4].getUInt());
                return true;
            }
            else
            // ------- sleep
            if (sleepProp.matchMethod( methodId , args) == 1) {
                fenster_sleep(static_cast<int64_t>(args[0].getDouble()));
                return true;
            }
            else
            // ------- export
            if (exportProp.matchMethod( methodId , args) == 1) {
                save_to_bmp(args[0].getStringRef().c_str(), &mBuffer);
                return true;
            }
            // ------- drawBufferProp
            if (drawBufferProp.matchMethod( methodId , args) == 1) {
                if (!args[2].isPointer()) return false;
                PixelBufferObject* pb = dynamic_cast<PixelBufferObject*>(args[2].asPointerObject());
                if (!pb) return false;
                int x = args[0].getInt();
                int y = args[1].getInt();

                mBuffer.blit(x,y,pb->mBuffer);

                return true;
            }


            return ValueObject::onMethodCall(methodId, args, ret);
        }
        // -------------------------------------------------------------------------
    }; //  struct FensterObject

    // -------------------------------------------------------------------------
    void RegisterFensterFunctions() {

        using namespace FunctionMap;

        // --- most used KEY CODES ----
        // A-Z  (ASCII 65 - 90)
        for (int i = 65; i <= 90; i++) {
            char name[8];
            sprintf(name, "KEY_%C", (char)i);
            RegisterConstants(name, Value(i));
        }

        // 0 - 0(ASCII 48 - 57)
        for (int i = 48; i <= 57; i++) {
            char name[8];
            sprintf(name, "KEY_%C", (char)i);
            RegisterConstants(name, Value(i));
        }
        //
        RegisterConstants("KEY_ESC",       Value(27));
        RegisterConstants("KEY_SPACE",     Value(32));
        RegisterConstants("KEY_BACKSPACE", Value(8));
        RegisterConstants("KEY_TAB",       Value(9));
        RegisterConstants("KEY_ENTER",     Value(10)); // or 13

        RegisterConstants("KEY_UP",        Value(17));
        RegisterConstants("KEY_DOWN",      Value(18));
        RegisterConstants("KEY_LEFT",      Value(20));
        RegisterConstants("KEY_RIGHT",     Value(19));


        // --- colors ----
        RegisterConstants("COLOR_BLACK",   Value(0xFF000000));
        RegisterConstants("COLOR_WHITE",   Value(0xFFFFFFFF));
        RegisterConstants("COLOR_RED",     Value(0xFFFF0000));
        RegisterConstants("COLOR_GREEN",   Value(0xFF00FF00));
        RegisterConstants("COLOR_BLUE",    Value(0xFF0000FF));

        RegisterConstants("COLOR_YELLOW",  Value(0xFFFFFF00));
        RegisterConstants("COLOR_MAGENTA", Value(0xFFFF00FF));
        RegisterConstants("COLOR_CYAN",    Value(0xFF00FFFF));

        RegisterConstants("COLOR_GRAY",    Value(0xFF808080));
        RegisterConstants("COLOR_DARKGRAY",Value(0xFF333333));
        RegisterConstants("COLOR_ORANGE",  Value(0xFFFFA500));


        // --------------------
        FensterObject::RegisterSymbols();
        PixelBufferObject::RegisterSymbols();
        // --------------------


        RegisterFunction("Fenster::new", [](std::vector<Value>& args, Value& ret) -> bool {

            if (args.size() < 4 || !args[0].isStringId()
                || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber()) {
                Tools::errorf("Usage: Fenster:new \"Window Title\" width height float scale [int sleepms default 16]\n");
                return false;
            }

            FensterObject* f = new FensterObject(args[0].getStringRef().c_str(),
                                args[1].getInt(), args[2].getInt(), args[3].getFloat());
            // if (args.size() == 5) f->mSleepMS = args[4].getInt();

            ret = Value(f);
            return true;
        });

         RegisterFunction("Fenster::color", [](std::vector<Value>& args, Value& ret) -> bool {

             if (args.size() < 3 ) {
                 Tools::errorf("Usage: Fenster:color int r int g int b [int a]\n");
                 return false;
             }

             uint8_t r = static_cast<uint8_t>(args[0].getInt());
             uint8_t g = static_cast<uint8_t>(args[1].getInt());
             uint8_t b = static_cast<uint8_t>(args[2].getInt());
             uint8_t a;

             if (args.size() < 4) a = 255;
             else a = static_cast<uint8_t>(args[3].getInt());

             // Format: 0xAARRGGBB
             uint32_t color = (static_cast<uint32_t>(a) << 24) |
             (static_cast<uint32_t>(r) << 16) |
             (static_cast<uint32_t>(g) << 8)  |
             (static_cast<uint32_t>(b));

             ret = Value(color);
             return true;
         });

         // ======================= PIXEL BUFFER ==============================
         RegisterFunction("PixelBuffer::new", [](std::vector<Value>& args, Value& ret) -> bool {

             if (args.size() !=2 || !args[0].isInt() || !args[1].isInt()) {
                 Tools::errorf("Usage: PixelBuffer:new  width height\n");
                 return false;
             }

             PixelBufferObject* b = new PixelBufferObject(args[0].asInt(), args[1].asInt());

             ret = Value(b);
             return true;
         });


    }
} //namespace DreiZehn::Fenster
