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

#include <vector>

#include "core/FunctionMap.h"
#include "core/VariableFrame.h"
#include "Globals.h"



namespace DreiZehn::Fenster {
#include "ext/fenster/fenster_audio.h"
}
// #include "ext/melodyMaker.h"


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
            if   (!mClosed) {
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


            // Fields
            availableProp  = ValueObjectProperty("avail","readonly floats avaiable", TypeFensterAudioObject);
            sampleRateProp = ValueObjectProperty("sampleRate","readonly return the samplerate", TypeFensterAudioObject);
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline bool onGetField(uint32_t fieldSymbolId, Value& ret) override {
            if (mClosed) {
                return true;
            }
            if (availableProp.matchField( fieldSymbolId)) {
                ret = Value(fenster_audio_available(&mFensterAudio));
                return true;
            }
            else
            if (sampleRateProp.matchField( fieldSymbolId)) {
                ret = Value(static_cast<uint32_t>(FENSTER_SAMPLE_RATE));
                return true;
            }

            return ValueObject::onGetField(fieldSymbolId, ret);
        }
        // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override {
            // Read-Only!
            if (availableProp.matchField( fieldSymbolId)) {
                return false;
            }
            else
            if (sampleRateProp.matchField( fieldSymbolId)) {
                return false;
            }

            return ValueObject::onSetField(fieldSymbolId, value);
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId, std::vector<Value>& args, Value& ret) override {

            if (mClosed) {
                ret = Value(0);
                return true;
            }

            // // static uint32_t testId = SymbolTable::insert("test");
            // // if (methodId == testId) {
            // //     // //  C4, D4, E4, F4, G4, A4, B4, C5
            // //     // //  60, 62, 64, 65, 67, 69, 71, 72
            // //
            // //     // FENSTER_SAMPLE_RATE
            // //     uint32_t  byteCount = 0;
            // //     float amplitute = 0.2f;
            // //     float* noteBuffer =  MelodyMaker::generateNoteF(
            // //             60, 0.3f, &byteCount, amplitute
            // //             ,FENSTER_SAMPLE_RATE
            // //     );
            // //     uint32_t sampleCount = byteCount / sizeof(float);
            // //     VectorValueObject* vec = new VectorValueObject();
            // //      vec->mElements.reserve(sampleCount);
            // //     for (uint32_t i = 0; i < sampleCount; i++) {
            // //         float f = noteBuffer[i];
            // //         Value v = Value(static_cast<double>(f));
            // //         vec->mElements.push_back(v);
            // //     }
            // //     std::free(noteBuffer);
            // //     ret = Value(vec);
            // //     return true;
            // //
            // //
            // //     // // MelodyMaker::Melody melody;
            // //     // // melody.push_back({10, {60, 0.3f}}); // C
            // //     // // melody.push_back({20, {64, 0.3f}}); // E
            // //     // // melody.push_back({30, {67, 0.3f}}); // G
            // //     // // melody.push_back({40, { 0, 0.3f}}); // silence
            // //     // // melody.push_back({50, {67, 0.3f}}); // G
            // //     // // melody.push_back({60, {64, 0.3f}}); // E
            // //     // // melody.push_back({70, {60, 0.3f}}); // C
            // //     // //
            // //     // //     return Audio::GenerateMelody(melody);
            // //     // //     return true;
            // // }
            // //
            // // else
            // ------- close
            if (closeProp.matchMethod( methodId , args) == 1) {
                fenster_audio_close(&mFensterAudio);
                mClosed = true;
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
                     ret = Value(0);
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
                    ret = Value(0);
                    return true;
                }
                float value = args[1].getFloat();
                mAudioBuffer[n] = value;
                ret = Value(1);
                return true;
            }
            // ------- get
            if (getProp.matchMethod( methodId , args) == 1) {
                uint32_t n =  args[0].getUInt();
                if (n >= FENSTER_AUDIO_BUFSZ) {
                    Tools::errorf("FensterAudioObject->set buffer overflow!");
                    ret = Value(0);
                    return true;
                }
                ret = Value (static_cast<double>(mAudioBuffer[n]));
                return true;
            }

             return ValueObject::onMethodCall(methodId, args, ret);
        }
        // -------------------------------------------------------------------------
    }; //  struct FensterAudioObject

    // -------------------------------------------------------------------------
    void RegisterFensterAudioFunctions(Environment& env) {

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
