//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// MiniFB
// NOTE: Require PixelBuffer!
//   https://github.com/emoon/minifb
// - [X] enums register  MiniFB_enums.h
// - [ ] change window size => mfb_close / mfb_open_ex
// - [ ] mfb_set_title(window, "New Window Title"); /get
// - [ ] events
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

#include <MiniFB.h>

    #define MFB_CONST_LIST(_) \
    _(MFB_KB_KEY_ESCAPE); \
    _(MFB_KB_KEY_ENTER); \
    _(MFB_KB_KEY_TAB); \
    _(MFB_KB_KEY_BACKSPACE); \
    _(MFB_KB_KEY_INSERT); \
    _(MFB_KB_KEY_DELETE); \
    _(MFB_KB_KEY_RIGHT); \
    _(MFB_KB_KEY_LEFT); \
    _(MFB_KB_KEY_DOWN); \
    _(MFB_KB_KEY_UP); \
    _(MFB_KB_KEY_PAGE_UP); \
    _(MFB_KB_KEY_PAGE_DOWN); \
    _(MFB_KB_KEY_HOME); \
    _(MFB_KB_KEY_END); \
    _(MFB_KB_KEY_SPACE); \
    _(MFB_KB_KEY_LEFT_SHIFT); \
    _(MFB_KB_KEY_RIGHT_SHIFT); \
    _(MFB_KB_KEY_LEFT_CONTROL); \
    _(MFB_KB_KEY_RIGHT_CONTROL); \
    _(MFB_KB_KEY_LEFT_ALT); \
    _(MFB_KB_KEY_RIGHT_ALT); \
    _(MFB_KB_KEY_F1); \
    _(MFB_KB_KEY_F2); \
    _(MFB_KB_KEY_F3); \
    _(MFB_KB_KEY_F4); \
    _(MFB_KB_KEY_F5); \
    _(MFB_KB_KEY_F6); \
    _(MFB_KB_KEY_F7); \
    _(MFB_KB_KEY_F8); \
    _(MFB_KB_KEY_F9); \
    _(MFB_KB_KEY_F10); \
    _(MFB_KB_KEY_F11); \
    _(MFB_KB_KEY_F12); \
    _(MFB_MOUSE_BTN_1); \
    _(MFB_MOUSE_BTN_2); \
    _(MFB_MOUSE_BTN_3); \
    /* MODS */ \
    _(MFB_KB_MOD_SHIFT); \
    _(MFB_KB_MOD_CONTROL); \
    _(MFB_KB_MOD_ALT); \
    _(MFB_KB_MOD_SUPER); \
    _(MFB_KB_MOD_CAPS_LOCK); \
    _(MFB_KB_MOD_NUM_LOCK); \
    /* WINDOW */ \
    _(MFB_WF_RESIZABLE); \
    _(MFB_WF_FULLSCREEN); \
    _(MFB_WF_FULLSCREEN_DESKTOP); \
    _(MFB_WF_BORDERLESS); \
    _(MFB_WF_ALWAYS_ON_TOP); \
    /* STATE results: */ \
    _(MFB_STATE_OK); \
    _(MFB_STATE_EXIT); \
    _(MFB_STATE_INVALID_WINDOW); \
    _(MFB_STATE_INVALID_BUFFER); \
    _(MFB_STATE_INTERNAL_ERROR); \
    /* stretch mode: */ \
    _(MFB_STRETCH_NEAREST); \
    _(MFB_STRETCH_BILINEAR); \
    /* mouse: */ \
    _(MFB_MOUSE_BTN_0); \
    _(MFB_MOUSE_BTN_1); \
    _(MFB_MOUSE_BTN_2); \
    _(MFB_MOUSE_BTN_3); \
    _(MFB_MOUSE_BTN_4); \
    _(MFB_MOUSE_BTN_5); \
    _(MFB_MOUSE_BTN_6); \
    _(MFB_MOUSE_BTN_7); \
    _(MFB_MOUSE_LEFT); \
    _(MFB_MOUSE_RIGHT); \
    _(MFB_MOUSE_MIDDLE); \


// =============================================================================
// --- MiniFBObject ---
// =============================================================================
namespace DreiZehn
{
    inline const int TypeMiniFBObject = RegisterUserObjectType("MiniFB");
    // ......................... FensterObject

    struct MiniFBObject : public ValueObject {
        struct mfb_window *mWindow = nullptr;
        mfb_update_state mState;
        pixel_buffer mPixelBuffer; // the pixel we write to
        size_t mPixelCount;
        int64_t mLastTime = 0;
        double  mFrameTime = 0.0;



        // Methods
        inline static ValueObjectProperty loopProp, setPixelProp, pixelProp, isKeyDownProp;
        inline static ValueObjectProperty closeProp, clearProp, lineProp;
        inline static ValueObjectProperty rectProp, circleProp, fillProp;
        inline static ValueObjectProperty textProp, sleepProp, exportProp;
        inline static ValueObjectProperty drawBufferProp;

        // Fields (read only)
        inline static ValueObjectProperty titleProp, widthProp, heightProp;
        inline static ValueObjectProperty mouseXProp, mouseYProp, mouseDownProp;
        inline static ValueObjectProperty timeProp, keymodProp;



        MiniFBObject(const char* title, U32 bufferW, U32 bufferH,
                     U32 WindowW, U32 WindowH, U32 flags)
                    : ValueObject(TypeMiniFBObject) {

            mWindow = mfb_open_ex(title, WindowW, WindowH, flags);
            if (!mWindow) {
                mState = MFB_STATE_INVALID_WINDOW;
                return;
            } ;
            mPixelBuffer.width = bufferW;
            mPixelBuffer.height = bufferH;
            mPixelCount = bufferW * bufferH;
            mPixelBuffer.buffer = new uint32_t[mPixelCount]();
            if (!mPixelBuffer.buffer) {
                mState = MFB_STATE_INVALID_BUFFER;
                return ;
            }
            mPixelBuffer.width = bufferW;
            mPixelBuffer.height = bufferH;
            mPixelBuffer.buffer = new uint32_t[bufferW * bufferH]();

            // mLastTime = fenster_time();
        }

        void unloadBuffers() {
            if (mWindow) {
                mfb_close(mWindow);
                mWindow = nullptr;
            }
            if (mPixelBuffer.buffer) {
                delete[] mPixelBuffer.buffer;
                mPixelBuffer.buffer = nullptr;
            }
        }

        ~MiniFBObject() {
            unloadBuffers();
            mWindow = nullptr;
        }

        static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            loopProp = ValueObjectProperty("loop", 0,1
            , "call the loop"
            , TypeMiniFBObject);

            setPixelProp = ValueObjectProperty("setPixel", 3,3
            , "set one pixel. @param int x int y uint color"
            , TypeMiniFBObject);

            pixelProp = ValueObjectProperty("pixel", 1,2
            , "set or get one pixel. @param index [color]"
            , TypeMiniFBObject);

            isKeyDownProp = ValueObjectProperty("isKeyDown", 1,1
            , "return if the given ASCII code key is pressed. @params int keycode"
            , TypeMiniFBObject);

            closeProp = ValueObjectProperty("close", 0,1
            , "close the window "
            , TypeMiniFBObject);

            clearProp = ValueObjectProperty("clear", 0,1
            , "clear the buffer with a color. @params [uint color] default COLOR_WHITE "
            , TypeMiniFBObject);

            lineProp = ValueObjectProperty("line", 5,5
            , "flood fill at point. @param int x0 int y0 int x1 int y1 uint color"
            , TypeMiniFBObject);

            rectProp = ValueObjectProperty("rect", 5,5
            , "flood fill at point. @param int x int y int width int height uint color"
            , TypeMiniFBObject);

            circleProp = ValueObjectProperty("circle", 4,4
            , "flood fill at point. @param int x int y int radius uint color"
            , TypeMiniFBObject);

            fillProp = ValueObjectProperty("fill", 4,4
            , "flood fill at point. @param int x int y uint oldcolor uint color"
            , TypeMiniFBObject);

            textProp = ValueObjectProperty("text", 5,5
            , "print a text, @param int x int y string text int scale uint color"
            , TypeMiniFBObject);

            sleepProp = ValueObjectProperty("sleep", 1,1
            , "sleep for x ms , @param int ms", TypeMiniFBObject);

            exportProp = ValueObjectProperty("export", 1,1
            , "export the picture to file, @param filename", TypeMiniFBObject);

            drawBufferProp = ValueObjectProperty("drawBuffer", 3,3
            , "draw a pixel buffer object at position", TypeMiniFBObject);


            titleProp     = ValueObjectProperty("getTitle",0,0,"get the window title", TypeMiniFBObject);
            widthProp     = ValueObjectProperty("getWidth",0,0,"get the window with", TypeMiniFBObject);
            heightProp    = ValueObjectProperty("getHeight",0,0,"get the window height", TypeMiniFBObject);
            mouseXProp    = ValueObjectProperty("getMouseX",0,0,"get mouseX", TypeMiniFBObject);
            mouseYProp    = ValueObjectProperty("getMouseY",0,0,"get mouseY", TypeMiniFBObject);
            mouseDownProp = ValueObjectProperty("getMouseDown",0,1,"get is mouse button down", TypeMiniFBObject);
            timeProp      = ValueObjectProperty("getTime",0,0,"get the tome", TypeMiniFBObject);
            keymodProp    = ValueObjectProperty("getKeymod",0,0,"get: mod is 4 bits mask, ctrl=1, shift=2, alt=4, meta=8", TypeMiniFBObject);
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        bool onMethodCall(Environment& env,uint32_t methodId, std::span<Value> args, Value& ret) override {
            if ( !mPixelBuffer.buffer ) {
                Tools::errorf("MiniFB Object is closed! Method: %s ignored.\n", SymbolTable::getName(methodId).c_str());
                return true; // return false says method not found!
            }


            // ------- loop
            if (loopProp.matchMethod( methodId , args) == 1) {

                if (!mfb_wait_sync(mWindow) ) {
                    mState = MFB_STATE_EXIT;
                    ret = Value(static_cast<int32_t>(mState));
                    return true;
                }
                mState =  mfb_update_ex(mWindow, mPixelBuffer.buffer
                    , mPixelBuffer.width, mPixelBuffer.height);

                ret = Value(static_cast<int32_t>(mState));
                return true;
            }
            // ------- setPixel
            else if (setPixelProp.matchMethod( methodId , args) == 1) {
                int32_t x = args[0].getInt();
                int32_t y = args[1].getInt();

                uint32_t color = args[2].getUInt();

                if (x >= 0 && x < mPixelBuffer.width && y >= 0 && y < mPixelBuffer.height) {
                    mPixelBuffer.set(x,y,color);
                }
                return true;
            }
            // ------- pixel
            else if (pixelProp.matchMethod( methodId , args) == 1) {
                U32 index = args[0].getInt();
                if ( args.size() > 0 ) {
                    U32 color = args[1].getUInt();
                    if (!mPixelBuffer.setAt(index, color)) return false;
                    ret = args[1];
                    return true;
                }
                U32 resultColor = 0;
                if (!mPixelBuffer.getAt(index, resultColor)) return false;
                ret = Value(resultColor);
                return true;
            }
            // ------- isKeyDown
            else if (isKeyDownProp.matchMethod( methodId , args) == 1) {

                int32_t keyCode = args[0].getInt();

                if (keyCode >= 0 && keyCode < 256) {
                    const uint8_t *keys = mfb_get_key_buffer(mWindow);
                    ret = Value(keys[keyCode]);
                    return true;
                }
                ret = NumberNullValue;
                return true;
            }
            // ------- close
            else if (closeProp.matchMethod( methodId , args) == 1) {
                unloadBuffers();
                ret = Value(1);
                return true;
            }

            else
            // ------- clear
            if (clearProp.matchMethod( methodId , args) == 1) {
                if (!mPixelBuffer.buffer  ) {
                    ret = NumberNullValue;
                    return true;
                }
                uint32_t color = 0xFFFFFFFF;
                if (args.size() > 0) {
                    color = args[0].getUInt();
                }
                std::fill_n(mPixelBuffer.buffer, mPixelBuffer.width * mPixelBuffer.height, color);
                return true;
            }
            else
            // ------- line
            if (lineProp.matchMethod( methodId , args) == 1) {
                line(&mPixelBuffer, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getInt(),args[4].getUInt());
                return true;
            }
            else
            // ------- rect
            if (rectProp.matchMethod( methodId , args) == 1) {
                int x = args[0].getInt(); if (x < 0) {
                    return true; //no error message here!
                }
                int y = args[1].getInt(); if (y < 0) {
                    return true; //no error message here!
                }
                int w = args[2].getInt(); if (w < 0 || x+w > mPixelBuffer.width) {
                    return false;
                }
                int h = args[3].getInt(); if (h < 0 || y+h > mPixelBuffer.height) {
                    return false;
                }
                uint32_t c = args[4].getUInt();

                rect(&mPixelBuffer, x,y,w,h,c);
                return true;
            }
            else
            // ------- circle
            if (circleProp.matchMethod( methodId , args) == 1) {
                circle(&mPixelBuffer, args[0].getInt(), args[1].getInt(), args[2].getInt(), args[3].getUInt());
                return true;
            }
            else
            // ------- fill
            if (fillProp.matchMethod( methodId , args) == 1) {
                fill(&mPixelBuffer, args[0].getInt(), args[1].getInt(), args[2].getUInt(), args[3].getUInt());
                return true;
            }
            else
            // ------- text
            if (textProp.matchMethod( methodId , args) == 1) {
                text(&mPixelBuffer,
                                     args[0].getInt(), args[1].getInt(),
                                     args[2].getStringRef().c_str(),
                                     args[3].getInt(), args[4].getUInt());
                return true;
            }
            // TODO ?
            // else
            // // ------- sleep
            // if (sleepProp.matchMethod( methodId , args) == 1) {
            //     fenster_sleep(static_cast<int64_t>(args[0].getDouble()));
            //     return true;
            // }
            // else
            // ------- export
            else if (exportProp.matchMethod( methodId , args) == 1) {
                save_to_bmp(args[0].getStringRef().c_str(), &mPixelBuffer);
                return true;
            }
            // ------- drawBufferProp
            else if (drawBufferProp.matchMethod( methodId , args) == 1) {
                if (!args[2].isObjectPointer()) return false;
                //FIXME check it's pixelbuffer and static_cast
                PixelBufferObject* pb = dynamic_cast<PixelBufferObject*>(args[2].asPointerObject());
                if (!pb) return false;
                int x = args[0].getInt();
                int y = args[1].getInt();

                mPixelBuffer.blit(x,y,pb->mBuffer);

                return true;
            }

    //TODO ?!
            // // no parameter can use matchField
            // if (titleProp.matchField( methodId)) {
            //     ret = Value(std::string(mFenster.title));
            //     return true;
            // }
            else if (mouseXProp.matchField(methodId)) {
                U32 win_w = mfb_get_window_width(mWindow);
                if (win_w == 0 || mPixelBuffer.width == 0) {
                    ret = Value(static_cast<S32>(0));
                    return true;
                }
                S32 win_x = mfb_get_mouse_x(mWindow);
                S32 px = (win_x * static_cast<S32>(mPixelBuffer.width)) / static_cast<S32>(win_w);
                if (px < 0) px = 0;
                else if (px >= static_cast<S32>(mPixelBuffer.width)) px = static_cast<S32>(mPixelBuffer.width) - 1;
                ret = Value(px);
                return true;
            }
            else if (mouseYProp.matchField(methodId)) {
                U32 win_h = mfb_get_window_height(mWindow);
                if (win_h == 0 || mPixelBuffer.height == 0) {
                    ret = Value(static_cast<S32>(0));
                    return true;
                }
                S32 win_y = mfb_get_mouse_y(mWindow);
                S32 py = (win_y * static_cast<S32>(mPixelBuffer.height)) / static_cast<S32>(win_h);
                if (py < 0) py = 0;
                else if (py >= static_cast<S32>(mPixelBuffer.height)) py = static_cast<S32>(mPixelBuffer.height) - 1;
                ret = Value(py);
                return true;
            }
            else if (mouseDownProp.matchField(methodId)) {
                U8 button = MFB_MOUSE_BTN_1; // Default: Linke Maustaste
                if (args.size() > 0) {
                    U32 raw = args[0].getUInt();
                    if (raw >= MFB_MOUSE_BTN_1 && raw <= MFB_MOUSE_BTN_7) {
                        button = static_cast<U8>(raw);
                    }
                }
                const U8* buttons = mfb_get_mouse_button_buffer(mWindow);
                ret = Value(buttons ? buttons[button] : static_cast<U8>(0));
                return true;
            }
            else if ( widthProp.matchField(methodId)) {
                ret = Value(static_cast<int32_t>(mPixelBuffer.width));
                return true;
            }
            else if ( heightProp.matchField(methodId)) {
                ret = Value(static_cast<int32_t>(mPixelBuffer.height));
                return true;
            }
            // else
            // if ( timeProp.matchField(methodId)) {
            //     ret = Value(static_cast<double>(fenster_time()));
            //     return true;
            // }
            // else
            // if ( keymodProp.matchField(methodId)) {
            //     ret = Value(static_cast<uint32_t>(mFenster.mod));
            //     return true;
            // }
            //

            return ValueObject::onMethodCall(env,methodId, args, ret);
        }
        // -------------------------------------------------------------------------
    }; //  struct FensterObject

    // -------------------------------------------------------------------------
    void RegisterMiniFBFunctions(Environment& env) {
        static bool registered = false; if (registered) return; registered = true;

        // requires PixelBuffer
        RegisterPixelBufferFunctions(env);

        using namespace FunctionMap;

//NOTE USING THE STUFF FROM Fenter here ... TODO
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


        MFB_CONST_LIST(REGISTER_CONST);


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
        MiniFBObject::RegisterSymbols();
        PixelBufferObject::RegisterSymbols();
        // --------------------


        RegisterFunction("MiniFB::new", [](std::span<Value> args, Value& ret) -> bool {

            if (args.size() < 4 || !args[0].isString()
                || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber()) {
                Tools::errorf("Usage: Fenster:new \"Window Title\" width height [scale] [flags]\n");
                return false;
            }

            U32 W = args[1].getUInt();
            U32 H = args[2].getUInt();
            U32 WinW = W;
            U32 WinH = H;
            if (args.size() > 3) {
                F32 scale = args[3].getFloat();
                WinW = static_cast<U32>(W * scale);
                WinH = static_cast<U32>(H * scale);
            }
            U32 flags = MFB_WF_RESIZABLE;
            if (args.size() > 4) flags = args[4].getUInt();

            MiniFBObject* fb = new MiniFBObject(args[0].getStringRef().c_str(),
                                W,H,WinW,WinH, flags);

            ret = Value(fb);
            return true;
        }, "Retrun a new MiniFB object: MiniFB:new \"Window Title\" width height [scale] [flags]");

         RegisterFunction("MiniFB::color", [](std::span<Value> args, Value& ret) -> bool {

             if (args.size() < 3 ) {
                 Tools::errorf("Usage: MiniFB:color int r int g int b [int a]\n");
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
         }, "Convert r g b [a] into a unsigned integer. Format: 0xAARRGGBB");


    }
} //namespace DreiZehn::Fenster
