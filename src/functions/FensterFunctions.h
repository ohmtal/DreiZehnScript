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

#include "core/FunctionMap.h"
#include "core/VariableFrame.h"
#include "Globals.h"
#include "ArrayFunctions.h"

namespace DreiZehn::Fenster {
#include "ext/fenster/fenster.h"
}


namespace DreiZehn::FensterWrapper {
    using namespace DreiZehn::Fenster;

    // -------------------------------------------------------------------------
    // from drawing-c
    // -------------------------------------------------------------------------
    static inline void line(struct fenster *f, int x0, int y0, int x1, int y1,
                            uint32_t c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;
    for (;;) {
        fenster_pixel(f, x0, y0) = c;
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
    static inline void rect(struct fenster *f, int x, int y, int w, int h,
                            uint32_t c) {
        for (int row = 0; row < h; row++) {
            for (int col = 0; col < w; col++) {
                fenster_pixel(f, x + col, y + row) = c;
            }
        }
    }

    static inline void circle(struct fenster *f, int x, int y, int r, uint32_t c) {
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                fenster_pixel(f, x + dx, y + dy) = c;
            }
        }
    }
    }
    // -------------------------------------------------------------------------
    static inline void fill(struct fenster *f, int x, int y, uint32_t old,
                            uint32_t c) {
    if (x < 0 || y < 0 || x >= f->width || y >= f->height) {
        return;
    }
    if (fenster_pixel(f, x, y) == old) {
        fenster_pixel(f, x, y) = c;
        fill(f, x - 1, y, old, c);
        fill(f, x + 1, y, old, c);
        fill(f, x, y - 1, old, c);
        fill(f, x, y + 1, old, c);
    }
    }
    // -------------------------------------------------------------------------
    constexpr uint16_t font5x3[] = {0x0000,0x2092,0x002d,0x5f7d,0x279e,0x52a5,0x7ad6,0x0012,0x4494,0x1491,0x017a,0x05d0,0x1400,0x01c0,0x0400,0x12a4,0x2b6a,0x749a,0x752a,0x38a3,0x4f4a,0x38cf,0x3bce,0x12a7,0x3aae,0x49ae,0x0410,0x1410,0x4454,0x0e38,0x1511,0x10e3,0x73ee,0x5f7a,0x3beb,0x624e,0x3b6b,0x73cf,0x13cf,0x6b4e,0x5bed,0x7497,0x2b27,0x5add,0x7249,0x5b7d,0x5b6b,0x3b6e,0x12eb,0x4f6b,0x5aeb,0x388e,0x2497,0x6b6d,0x256d,0x5f6d,0x5aad,0x24ad,0x72a7,0x6496,0x4889,0x3493,0x002a,0xf000,0x0011,0x6b98,0x3b79,0x7270,0x7b74,0x6750,0x95d6,0xb9ee,0x5b59,0x6410,0xb482,0x56e8,0x6492,0x5be8,0x5b58,0x3b70,0x976a,0xcd6a,0x1370,0x38f0,0x64ba,0x3b68,0x2568,0x5f68,0x54a8,0xb9ad,0x73b8,0x64d6,0x2492,0x3593,0x03e0};
    // -------------------------------------------------------------------------
    static inline void text(struct fenster *f, int x, int y, const char *s, int scale, uint32_t c) {
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
     int save_to_bmp(const char *filename, struct fenster *f) {
         if (!f || !f->buf || f->width <= 0 || f->height <= 0) return -1;

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
                 uint32_t pixel = fenster_pixel(f, x, y);

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
     //TODO implement
     void draw_scaled_pixel(struct fenster *f, int x, int y, int scale, uint32_t color) {
         for (int dy = 0; dy < scale; dy++) {
             for (int dx = 0; dx < scale; dx++) {
                 int screen_x = x * scale + dx;
                 int screen_y = y * scale + dy;

                 if (screen_x >= 0 && screen_x < f->width && screen_y >= 0 && screen_y < f->height) {
                     f->buf[screen_y * f->width + screen_x] = color;
                 }
             }
         }
     }
     // ------------------------------------------------------------------------
     //TODO implement
     void scale_buffer_to_window(struct fenster *f, uint32_t *src_buf, int src_w, int src_h) {
         for (int y = 0; y < f->height; y++) {
             int src_y = (y * src_h) / f->height;

             for (int x = 0; x < f->width; x++) {
                 int src_x = (x * src_w) / f->width;
                 f->buf[y * f->width + x] = src_buf[src_y * src_w + src_x];
             }
         }
     }
     // ------------------------------------------------------------------------


}


// =============================================================================
// --- FensterObject ---
// =============================================================================

namespace DreiZehn {
    using namespace DreiZehn::Fenster;
    const int TypeFensterObject = RegisterUserObjectType("Fenster");

    struct FensterObject : public ValueObject {
        struct fenster mFenster = {0};
        uint32_t* mPixelBuffer = nullptr;
        int64_t mSleepMS = 0;

        // Methods
        inline static ValueObjectProperty loopProp, setPixelProp, isKeyDownProp;
        inline static ValueObjectProperty closeProp, clearProp, lineProp;
        inline static ValueObjectProperty rectProp, circleProp, fillProp;
        inline static ValueObjectProperty textProp, sleepProp, exportProp;

        // Fields (read only)
        inline static ValueObjectProperty titleProp, widthProp, heightProp;
        inline static ValueObjectProperty mouseXProp, mouseYProp, mouseDownProp;
        inline static ValueObjectProperty timeProp, keymodProp;



        FensterObject(const char* title, int w, int h) : ValueObject(TypeFensterObject) {
            mFenster.title = title;
            mFenster.width = w;
            mFenster.height = h;
            mPixelBuffer = new uint32_t[w * h]();
            mFenster.buf = mPixelBuffer;
            fenster_open(&mFenster);
        }

        ~FensterObject() {
            if (mPixelBuffer) {
                fenster_close(&mFenster);
                delete[] mPixelBuffer;
                mPixelBuffer = nullptr;
            }
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
                ret = Value(static_cast<int32_t>(mFenster.x));
                return true;
            }
            else
            if ( mouseYProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mFenster.y));
                return true;
            }
            else
            if ( mouseDownProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mFenster.mouse));
                return true;
            }
            else
            if ( widthProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mFenster.width));
                return true;
            }
            else
            if ( heightProp.matchField(fieldSymbolId)) {
                ret = Value(static_cast<int32_t>(mFenster.height));
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

            return false;
        }
        // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override {
            // Read-Only!
            return false;
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId, std::vector<Value>& args, Value& ret) override {
            if (!mPixelBuffer ) {
                Tools::errorf("Fenster Object is closed! Method: %s ignored.\n", SymbolTable::getName(methodId).c_str());
                return true; // return false says method not found!
            }


            // ------- loop
            if (loopProp.matchMethod( methodId , args) == 1) {
                int result = fenster_loop(&mFenster);
                if (mSleepMS > 0) fenster_sleep(mSleepMS);
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
                    mPixelBuffer[y * mFenster.width + x] = color;
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
                if (mPixelBuffer) {
                    fenster_close(&mFenster);
                    delete[] mPixelBuffer;
                    mPixelBuffer = nullptr;
                }
                ret = Value(1);
                return true;
            }

            else
            // ------- clear
            if (clearProp.matchMethod( methodId , args) == 1) {
                uint32_t color = 0xFFFFFFFF;
                if (args.size() > 0) {
                    color = args[0].getUInt();
                }
                std::fill_n(mPixelBuffer, mFenster.width * mFenster.height, color);
                return true;
            }
            else
            // ------- line
            if (lineProp.matchMethod( methodId , args) == 1) {
                FensterWrapper::line(&mFenster, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getInt(),args[4].getUInt());
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

                FensterWrapper::rect(&mFenster, x,y,w,h,c);
                return true;
            }
            else
            // ------- circle
            if (circleProp.matchMethod( methodId , args) == 1) {
                FensterWrapper::circle(&mFenster, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getUInt());
                return true;
            }
            else
            // ------- fill
            if (fillProp.matchMethod( methodId , args) == 1) {
                FensterWrapper::fill(&mFenster, args[0].getInt(), args[1].getInt(), args[2].getUInt(), args[3].getUInt());
                return true;
            }
            else
            // ------- text
            if (textProp.matchMethod( methodId , args) == 1) {
                FensterWrapper::text(&mFenster,
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
                FensterWrapper::save_to_bmp(args[0].getStringRef().c_str(), &mFenster);
                return true;
            }
            // // ------- nothing found
            // else {
            //     Tools::errorf("Unknown Fenster method: %s\n", SymbolTable::getName(methodId).c_str());
            // }


            return false;
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
        RegisterConstants("KEY_ENTER",     Value(10)); // oder 13, fenster normalisiert meist auf \n (10)

        RegisterConstants("KEY_UP",        Value(17));
        RegisterConstants("KEY_DOWN",      Value(18));
        RegisterConstants("KEY_LEFT",      Value(19));
        RegisterConstants("KEY_RIGHT",     Value(20));


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
        // --------------------


        RegisterFunction("Fenster::new", [](std::vector<Value>& args, Value& ret) -> bool {

            if (args.size() < 3 || !args[0].isStringId() || !args[1].isInt() || !args[2].isInt()) {
                Tools::errorf("Usage: Fenster:new \"Window Title\" width height [int sleepms default 16]\n");
                return false;
            }

            FensterObject* f = new FensterObject(args[0].getStringRef().c_str(), args[1].asInt(), args[2].asInt());
            if (args.size() == 4) f->mSleepMS = args[3].asInt();

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

    }
} //namespace DreiZehn
