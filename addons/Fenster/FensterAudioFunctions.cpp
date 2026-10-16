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
// #pragma once

#include <stdio.h>
#include <stdint.h>

#include <vector>

#include "src/core/FunctionMap.h"
#include "src/core/VariableFrame.h"
#include "src/Globals.h"



namespace DreiZehn::Fenster {
#if defined(_WIN32)
    #ifdef PlaySound
    #undef PlaySound
    #endif
#endif

#include "fenster/fenster_audio.h"
}

// =============================================================================
// --- FensterAudioObject ---
// =============================================================================

namespace DreiZehn {
    using namespace DreiZehn::Fenster;

    const int TypeFensterAudioObject = RegisterUserObjectType("FensterAudio");

    struct FensterAudioObject : public ValueObject {
        struct fenster_audio mFensterAudio = {0};
        float mAudioBuffer[FENSTER_AUDIO_BUFSZ];
        bool mClosed = false;

        // methods
        inline static ValueObjectProperty closeProp, writeProp, setProp, getProp;

        // fields
        inline static ValueObjectProperty availableProp;
        inline static ValueObjectProperty sampleRateProp;


        FensterAudioObject() : ValueObject(TypeFensterAudioObject) {
            fenster_audio_open(&mFensterAudio);
            mClosed = false;
        }

        ~FensterAudioObject() {
            if  (!mClosed) {
                fenster_audio_close(&mFensterAudio);
                mClosed = true;
            }
        }

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            // Methods
            closeProp = ValueObjectProperty("close", 0,0
            , "call the auto object" , TypeFensterAudioObject);

            writeProp = ValueObjectProperty("write", 1,1
            , "write the buffer. @params uint count" , TypeFensterAudioObject);

            setProp = ValueObjectProperty("set", 2,2
            , "set a float value at index to the buffer. @params uint index float value" , TypeFensterAudioObject);

            getProp = ValueObjectProperty("get", 1,1
            , "get the float value at index. @params uint index" , TypeFensterAudioObject);



            availableProp  = ValueObjectProperty("getAvail",0,0,"floats avaiable", TypeFensterAudioObject);
            sampleRateProp = ValueObjectProperty("getSampleRate",0,0,"get the samplerate", TypeFensterAudioObject);
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId, std::vector<Value>& args, Value& ret) override {

            if (mClosed) {
                ret = NumberNullValue;
                return true;
            }

            // ------- close
            if (closeProp.matchMethod( methodId , args) == 1) {
                if (!mClosed) {
                    fenster_audio_close(&mFensterAudio);
                    mClosed = true;
                }
                ret = Value(1);
                return true;
            }
            else
            // ------- write
            if (writeProp.matchMethod( methodId , args) == 1) {
                 size_t n =  static_cast<size_t>(args[0].getUInt());
                 size_t avail = fenster_audio_available(&mFensterAudio);
                 if (n > avail) {
                     Tools::errorf("FensterAudioObject->write buffer overflow! n=%zu avail=%zu \n", n, avail);
                     ret = NumberNullValue;
                     return true;
                }
                fenster_audio_write(&mFensterAudio, mAudioBuffer,n);
                ret = Value(1);
                return true;
            }
            else
            // ------- set
            if (setProp.matchMethod( methodId , args) == 1) {
                uint32_t n =  args[0].getUInt();
                if (n >= FENSTER_AUDIO_BUFSZ) {
                    Tools::errorf("FensterAudioObject->set buffer overflow!\n");
                    ret = NumberNullValue;
                    return true;
                }
                float value = args[1].getFloat();
                mAudioBuffer[n] = value;
                ret = Value(1);
                return true;
            }
            // ------- get
            else
            if (getProp.matchMethod( methodId , args) == 1) {
                uint32_t n =  args[0].getUInt();
                if (n >= FENSTER_AUDIO_BUFSZ) {
                    Tools::errorf("FensterAudioObject->set buffer overflow!");
                    ret = NumberNullValue;
                    return true;
                }
                ret = Value (static_cast<double>(mAudioBuffer[n]));
                return true;
            }
            else
            if (availableProp.matchField( methodId)) {
                ret = Value(fenster_audio_available(&mFensterAudio));
                return true;
            }
            else
            if (sampleRateProp.matchField( methodId)) {
                ret = Value(static_cast<uint32_t>(FENSTER_SAMPLE_RATE));
                return true;
            }

             return ValueObject::onMethodCall(methodId, args, ret);
        }
        // -------------------------------------------------------------------------
    }; //  struct FensterAudioObject

    // -------------------------------------------------------------------------
    void RegisterFensterAudioFunctions(Environment& env) {
        static bool registered = false; if (registered) return; registered = true;

        using namespace FunctionMap;

        // --------------------
        FensterAudioObject::RegisterSymbols();
        // --------------------


        RegisterFunction("FensterAudio::new", [](std::vector<Value>& args, Value& ret) -> bool {


            FensterAudioObject* fa = new FensterAudioObject();

            ret = Value(fa);
            return true;
        });

    }
} //namespace DreiZehn
