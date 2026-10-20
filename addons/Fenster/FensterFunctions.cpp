//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Fenster Commands
// NOTE: Require PixelBuffer!
// NOTE! Modified fenster.h!
//      - I had to remove const int width and height in fenster.h!
//      - added X11 wmDeleteMessage message so my prog does not crash when
//        Window is closed.
//-----------------------------------------------------------------------------
// #pragma once
#include <stdio.h>
#include <stdint.h>
#include <cstring>

#include <fstream>
#include <cstdint>

#include "src/core/FunctionMap.h"
#include "src/core/VariableFrame.h"
#include "src/Globals.h"

#include "src/functions/PixelBufferFunctions.h"
#include "src/toolbox/PixelBuffer.h"
#include "core/Value.h"

namespace DreiZehn::Fenster {
    #include "fenster/fenster.h"

     // ------------------------------------------------------------------------
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
    inline const int TypeFensterObject = RegisterUserObjectType("Fenster");
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
            if (scale < 0.f) scale = 1.f;
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

        static void RegisterSymbols() {
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

            drawBufferProp = ValueObjectProperty("drawBuffer", 3,3
            , "draw a pixel buffer object at position", TypeFensterObject);


            titleProp     = ValueObjectProperty("getTitle",0,0,"get the window title", TypeFensterObject);
            widthProp     = ValueObjectProperty("getWidth",0,0,"get the window with", TypeFensterObject);
            heightProp    = ValueObjectProperty("getHeight",0,0,"get the window height", TypeFensterObject);
            mouseXProp    = ValueObjectProperty("getMouseX",0,0,"get mouseX", TypeFensterObject);
            mouseYProp    = ValueObjectProperty("getMouseY",0,0,"get mouseY", TypeFensterObject);
            mouseDownProp = ValueObjectProperty("getMouseDown",0,0,"get is mouse button down", TypeFensterObject);
            timeProp      = ValueObjectProperty("getTime",0,0,"get the tome", TypeFensterObject);
            keymodProp    = ValueObjectProperty("getKeymod",0,0,"get: mod is 4 bits mask, ctrl=1, shift=2, alt=4, meta=8", TypeFensterObject);
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        bool onMethodCall(uint32_t methodId, std::vector<Value>& args, Value& ret) override {
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
                ret = NumberNullValue;
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
                    ret = NumberNullValue;
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

            // no parameter can use matchField
            if (titleProp.matchField( methodId)) {
                ret = Value(std::string(mFenster.title));
                return true;
            }
            else
            if (mouseXProp.matchField(methodId)) {
                ret = Value(static_cast<int32_t>(mFenster.x / mScale));
                return true;
            }
            else
            if ( mouseYProp.matchField(methodId)) {
                ret = Value(static_cast<int32_t>(mFenster.y / mScale));
                return true;
            }
            else
            if ( mouseDownProp.matchField(methodId)) {
                ret = Value(static_cast<int32_t>(mFenster.mouse));
                return true;
            }
            else
            if ( widthProp.matchField(methodId)) {
                ret = Value(static_cast<int32_t>(mBuffer.width));
                return true;
            }
            else
            if ( heightProp.matchField(methodId)) {
                ret = Value(static_cast<int32_t>(mBuffer.height));
                return true;
            }
            else
            if ( timeProp.matchField(methodId)) {
                ret = Value(static_cast<double>(fenster_time()));
                return true;
            }
            else
            if ( keymodProp.matchField(methodId)) {
                ret = Value(static_cast<uint32_t>(mFenster.mod));
                return true;
            }


            return ValueObject::onMethodCall(methodId, args, ret);
        }
        // -------------------------------------------------------------------------
    }; //  struct FensterObject

    // -------------------------------------------------------------------------
    void RegisterFensterFunctions(Environment& env) {
        static bool registered = false; if (registered) return; registered = true;

        // requires PixelBuffer
        RegisterPixelBufferFunctions(env);

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

            if (args.size() < 4 || !args[0].isString()
                || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber()) {
                Tools::errorf("Usage: Fenster:new \"Window Title\" width height float scale\n");
                return false;
            }

            FensterObject* f = new FensterObject(args[0].getStringRef().c_str(),
                                args[1].getInt(), args[2].getInt(), args[3].getFloat());
            // if (args.size() == 5) f->mSleepMS = args[4].getInt();

            ret = Value(f);
            return true;
        }, "Retrun a new Fenster object: Fenster:new \"Window Title\" width height float scale");

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
         }, "Convert r g b [a] into a integer. Format: 0xAARRGGBB");


    }
} //namespace DreiZehn::Fenster
