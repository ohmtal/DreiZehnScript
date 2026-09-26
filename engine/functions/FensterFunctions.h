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
#include "core/FunctionMap.h"
#include "core/VariableFrame.h"
#include "Globals.h"
#include "ArrayFunctions.h"
#include "ext/fenster/fenster.h"

// NOTE this allowes to contunue when a window is closed!
// // #ifndef _WIN32
// // #include <X11/Xlib.h>
// // int handle_x11_error(Display *display, XErrorEvent *error) {
// //     return 0; // ignore
// // }
// // int handle_x11_io_error(Display *display) {
// //     return 0;
// // }
// // #endif

namespace DreiZehn::FensterWrapper {
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
}

// =============================================================================
// --- FensterObject ---
// =============================================================================

namespace DreiZehn {

    const int TypeFensterObject = RegisterUserObjectType("Fenster");

    struct FensterObject : public ValueObject {
        struct fenster mFenster = {0};
        uint32_t* mPixelBuffer = nullptr;

        // Symbol-IDs
        inline static uint32_t titleId = 0, widthId = 0, heightId = 0;
        inline static uint32_t mouseXId = 0, mouseYId = 0, mouseDownId = 0;


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

            titleId     = SymbolTable::insert("title");
            widthId     = SymbolTable::insert("width");
            heightId    = SymbolTable::insert("height");
            mouseXId    = SymbolTable::insert("mouseX");
            mouseYId    = SymbolTable::insert("mouseY");
            mouseDownId = SymbolTable::insert("mouseDown");
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline bool onGetField(uint32_t fieldSymbolId, Value& ret) override {
            if (fieldSymbolId == mouseXId) {
                ret = Value(static_cast<int32_t>(mFenster.x));
                return true;
            }
            if (fieldSymbolId == mouseYId) {
                ret = Value(static_cast<int32_t>(mFenster.y));
                return true;
            }
            if (fieldSymbolId == mouseDownId) {
                ret = Value(static_cast<int32_t>(mFenster.mouse));
                return true;
            }
            if (fieldSymbolId == widthId) {
                ret = Value(static_cast<int32_t>(mFenster.width));
                return true;
            }
            if (fieldSymbolId == heightId) {
                ret = Value(static_cast<int32_t>(mFenster.height));
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
                return false;
            }

            //TODO add ValueObjectMethod ...
            static uint32_t loopId = SymbolTable::insert("loop");
            static uint32_t setPixelId = SymbolTable::insert("setPixel");
            static uint32_t isKeyDownId = SymbolTable::insert("isKeyDown");
            static uint32_t closeId = SymbolTable::insert("close");
            static uint32_t clearId = SymbolTable::insert("clear") ;

            static uint32_t lineId = SymbolTable::insert("line");
            static uint32_t rectId = SymbolTable::insert("rect");
            static uint32_t circleId = SymbolTable::insert("circle");
            static uint32_t fillId = SymbolTable::insert("fill");
            static uint32_t textId = SymbolTable::insert("text");

            // ------- loop
            if (methodId == loopId) {
                int result = fenster_loop(&mFenster);
                fenster_sleep(16);
                ret = Value(static_cast<int32_t>(result == 0));
                return true;
            }
            else
            // ------- setPixel
            if (methodId == setPixelId) {
                if (args.size() != 3) {
                    Tools::errorf("Usage .setPixel int x int y uint color\n");
                    return false;
                }
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
            if (methodId == isKeyDownId) {
                if (args.size() != 1) {
                    Tools::errorf("Usage .isKeyDown int keycode\n");
                    return false;
                }
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
            if (methodId == closeId) {
                if (mPixelBuffer) {
                    fenster_close(&mFenster);
                    delete[] mPixelBuffer;
                    mPixelBuffer = nullptr;
                }
                ret = Value(1);
                return true;
            }

            // ------- clear
            if (methodId == clearId && args.size() >= 1) {
                uint32_t color = 0xFFFFFFFF;
                if (args.size() > 0) {
                    color = args[0].getUInt();
                }
                std::fill_n(mPixelBuffer, mFenster.width * mFenster.height, color);
                return true;
            }


            // ------- line
            if (methodId == lineId) {
                if (args.size() != 5) {
                     Tools::errorf("Usage .line x0 y0 x1 y1 color\n");
                    return false;
                }

                FensterWrapper::line(&mFenster, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getInt(),args[4].getUInt());
                return true;
            }
            // ------- rect
            if (methodId == rectId) {
                if (args.size() != 5) {
                    Tools::errorf("Usage .rect x y w h color\n");
                    return false;
                }

                FensterWrapper::rect(&mFenster, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getInt(),args[4].getUInt());
                return true;
            }

            // ------- circle
            if (methodId == circleId) {
                if (args.size() != 4) {
                    Tools::errorf("Usage .circle x y r color\n");
                    return false;
                }
                FensterWrapper::circle(&mFenster, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getUInt());
                return true;
            }

            // ------- fill
            if (methodId == fillId) {
                if (args.size() != 4) {
                    Tools::errorf("Usage .fill x y oldcolor color\n");
                    return false;
                }
                FensterWrapper::fill(&mFenster, args[0].getInt(), args[1].getInt(), args[2].getUInt(), args[3].getUInt());
                return true;
            }

            // ------- text
            if (methodId == textId) {
                if (args.size() != 5) {
                    Tools::errorf("Usage .text x y text scale color\n");
                    return false;
                }
                FensterWrapper::text(&mFenster,
                                     args[0].getInt(), args[1].getInt(),
                                     args[2].getString(),
                                     args[3].getInt(), args[4].getUInt());
                return true;
            }
            // static inline void text(struct fenster *f, int x, int y, char *s, int scale, uint32_t c)

            // ------- nothing found
            else {
                Tools::errorf("Unknown Fenster method: %s\n", SymbolTable::getName(methodId).c_str());
            }


            return false;
        }
        // -------------------------------------------------------------------------
    }; //  struct FensterObject

    // -------------------------------------------------------------------------
    void RegisterFensterFunctions() {

        // // #ifndef _WIN32
        // // // X11 close window handler
        // // XSetErrorHandler(handle_x11_error);
        // // XSetIOErrorHandler(handle_x11_io_error);
        // // #endif

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


        RegisterFunction("Fenster::new", [](std::vector<Value>& args, Value& ret) -> bool {

            if (args.size() != 3 || !args[0].isString() || !args[1].isInt() || !args[2].isInt()) {
                Tools::errorf("Usage: Fenster:new \"Window Title\" width height\n");
                return false;
            }

            FensterObject* f = new FensterObject(args[0].getString(), args[1].asInt(), args[2].asInt());
            ret = Value(f);
            if (gCurrentFrame) gCurrentFrame->addToGarbageCollection(f);
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
