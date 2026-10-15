//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// PixelBuffer Functions
// this is currently used by Fenster ->drawBuffer but can be used for
// other libs supporting pixels too ;)
//-----------------------------------------------------------------------------
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <cstring>

#include <fstream>
#include <cstdint>

#include "core/FunctionMap.h"
#include "core/VariableFrame.h"
#include "Globals.h"
#include "ArrayFunctions.h"

#include "toolbox/PixelBuffer.h"


// =============================================================================
// --- PixelBufferObject ---
// =============================================================================

namespace DreiZehn {
    const int TypePixelBuffer = RegisterUserObjectType("PixelBuffer");


    // ......................... PixelBuffer
    struct PixelBufferObject: ValueObject {
        pixel_buffer mBuffer;

        // Methods
        inline static ValueObjectProperty  setPixelProp, setPixelAtProp;
        inline static ValueObjectProperty clearProp, lineProp;
        inline static ValueObjectProperty rectProp, circleProp, fillProp;
        inline static ValueObjectProperty textProp, sleepProp, exportProp;
        inline static ValueObjectProperty printProp, setDataProp, saveProp, loadProp;
        inline static ValueObjectProperty cloneRotate90Prop, cloneRotateFreeProp;

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

            setPixelAtProp = ValueObjectProperty("setPixelAt", 2,2
            , "set one pixel at absolute index. @param int index uint color"
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

            saveProp = ValueObjectProperty("save", 1,1
            , "save buffer to tga file format. Usage: ->save filename", TypePixelBuffer);

            loadProp = ValueObjectProperty("load", 1,1
            , "load buffer from tga file format. Usage: ->load filename", TypePixelBuffer);

            cloneRotate90Prop=ValueObjectProperty("cloneRotate90", 0,0
            , "clone the current buffer and return it rotated 90 degree blockwise", TypePixelBuffer);

            cloneRotateFreeProp=ValueObjectProperty("cloneRotateFree", 1,1
            , "clone the current buffer and return it rotated  degree ", TypePixelBuffer);





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
                int32_t x = args[0].getUInt();
                int32_t y = args[1].getUInt();

                uint32_t color = args[2].getUInt();

                mBuffer.set(x,y,color);
                return true;
            }
            // ------- setPixel
            if (setPixelAtProp.matchMethod( methodId , args) == 1) {
                int32_t index = args[0].getUInt();
                uint32_t color = args[1].getUInt();
                mBuffer.setAt(index,color);
                return true;
            }

            else
            // ------- clear
            if (clearProp.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer  ) {
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

                std::string filename = args[0].getStringRef().c_str();
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
            // ------- setData
            if (setDataProp.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer) return false;

                uint32_t buffercount = mBuffer.width * mBuffer.height;
                uint32_t argscount = args.size();
                if (argscount > buffercount) argscount = buffercount;
                for (uint32_t i = 0 ; i < argscount; i++) {
                    mBuffer.buffer[i] = args[i].getUInt();
                }
                ret = Value(argscount);
                return true;
            }

            // ------- save (tga)
            if (saveProp.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer) return false;

                if (!args[0].isStringId()) {
                    ret = NumberNullValue;
                    return true;
                }

                ret = Value(save_to_tga(args[0].getStringRef().c_str(), mBuffer));

                return true;
            }
            // ------- load (tga)
            if (loadProp.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer) return false;

                if (!args[0].isStringId()) {
                    ret = NumberNullValue;
                    return true;
                }

                ret = Value(load_from_tga(args[0].getStringRef().c_str(), mBuffer));

                return true;
            }
            // ------- clone Rotate
            if (cloneRotate90Prop.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer) return false;

                PixelBufferObject* obj = new PixelBufferObject( mBuffer.width, mBuffer.height);
                rotate90_clockwise(mBuffer, obj->mBuffer);
                ret = Value(obj);

                return true;
            }

            if (cloneRotateFreeProp.matchMethod( methodId , args) == 1) {
                if (!mBuffer.buffer) return false;

                float angle = args[0].getFloat();
                PixelBufferObject* obj = new PixelBufferObject( mBuffer.width, mBuffer.height);
                rotate_arbitrary(mBuffer, obj->mBuffer, angle);
                ret = Value(obj);

                return true;


                return ValueObject::onMethodCall(methodId, args, ret);
            }


            return ValueObject::onMethodCall(methodId, args, ret);
        }

    };

    // -------------------------------------------------------------------------
    void RegisterPixelBufferFunctions(Environment& env) {
        static bool registered = false; if (registered) return; registered = true;
        using namespace FunctionMap;


         // ======================= PIXEL BUFFER ==============================
         PixelBufferObject::RegisterSymbols();
         RegisterFunction("PixelBuffer::new", [](std::vector<Value>& args, Value& ret) -> bool {

             if (args.size() !=2 || !args[0].isInt() || !args[1].isInt()) {
                 Tools::errorf("Usage: PixelBuffer:new  width height\n");
                 return false;
             }

             PixelBufferObject* b = new PixelBufferObject(args[0].asInt(), args[1].asInt());

             ret = Value(b);
             return true;
         }, "Return a new PixelBuffer Object. PixelBuffer:new  width height ");


    }
} //namespace DreiZehn
