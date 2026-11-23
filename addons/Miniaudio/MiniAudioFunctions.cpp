//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Mini Audio
//-----------------------------------------------------------------------------
// #pragma once

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <cstring>
#include "src/core/FunctionMap.h"
#include "src/core/VariableFrame.h"
#include "src/Globals.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

// =============================================================================
// --- MiniAudioObject ---
// =============================================================================

namespace DreiZehn {

    const int TypeMiniAudioCallBackObject = RegisterUserObjectType("MiniAudioCallback");

    struct MiniAudioCallBackObject : public ValueObject {
    private:
        ma_device mDevice;
        ma_pcm_rb mRingBuffer;
        std::vector<float> mAudioBuffer;
        U32 mLastRequestedAvailable = 0;
        bool mClosed = true;

        U32 mSampleRate = 44100;
        U32 mChannels = 1;
        U32 mBufferSize = 2048;

        // methods
        inline static ValueObjectProperty closeProp, writeProp, setProp, getProp;

        // fields
        inline static ValueObjectProperty availableProp;
        inline static ValueObjectProperty sampleRateProp;

        // ------------ CALLBACK
        static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
            auto* self = static_cast<MiniAudioCallBackObject*>(pDevice->pUserData);
            U32 CHANNELS = self->mChannels;
            float* out = static_cast<float*>(pOutput);

            ma_uint32 framesToRead = frameCount;
            void* pReadBuffer;

            ma_pcm_rb_acquire_read(&self->mRingBuffer, &framesToRead, &pReadBuffer);
            if (framesToRead > 0) {
                std::memcpy(out, pReadBuffer, framesToRead * CHANNELS * sizeof(float));
                ma_pcm_rb_commit_read(&self->mRingBuffer, framesToRead);
            }
            if (framesToRead < frameCount) {
                std::memset(out + (framesToRead * CHANNELS), 0, (frameCount - framesToRead) * CHANNELS * sizeof(float));
            }
            (void)pInput;
        }
        // ------------ push pending
        void commitPendingSamples() {
            if (mLastRequestedAvailable == 0) return;

            ma_uint32 framesToWrite = mLastRequestedAvailable;
            void* pWriteBuffer;

            if (ma_pcm_rb_acquire_write(&mRingBuffer, &framesToWrite, &pWriteBuffer) == MA_SUCCESS && framesToWrite > 0) {
                std::memcpy(pWriteBuffer, mAudioBuffer.data(), framesToWrite * mChannels * sizeof(float));
                ma_pcm_rb_commit_write(&mRingBuffer, framesToWrite);
            }
            mLastRequestedAvailable = 0;
        }

        U32 getAvail() {
            //write does this commitPendingSamples();
            ma_uint32 availableFrames = ma_pcm_rb_available_write(&mRingBuffer);
            if (availableFrames > mBufferSize) {
                availableFrames = mBufferSize;
            }
            mLastRequestedAvailable = availableFrames;
            return availableFrames;
        }

    public:
        // ------------ Constructor // Destructor
        MiniAudioCallBackObject() : ValueObject(TypeMiniAudioCallBackObject),mClosed(true) { }

        ~MiniAudioCallBackObject() {
            Close();
        }

        // ------------ Close Device and RingBuffer
        void Close() {
            if  (!mClosed) {
                ma_device_uninit(&mDevice);
                ma_pcm_rb_uninit(&mRingBuffer);
                mClosed = true;
            }
        }

        // ------------ Open Device and Init RingBuffer
        bool Open(U32 sampleRate, U32 channels, U32 bufferSize = 2048) {
            if (!mClosed) {
                Tools::errorf("MiniAudio: Device is open i will not open it again!");
                return false;
            }

            mSampleRate = sampleRate; mChannels = channels; mBufferSize = bufferSize;
            mAudioBuffer.resize(mBufferSize * 2);
            ma_pcm_rb_init(ma_format_f32, mChannels, mBufferSize * 2, nullptr, nullptr, &mRingBuffer);

            ma_device_config config = ma_device_config_init(ma_device_type_playback);
            config.playback.format   = ma_format_f32;
            config.playback.channels = mChannels;
            config.sampleRate        = mSampleRate;
            config.dataCallback      = data_callback;
            config.pUserData         = this;

            ma_result result = ma_device_init(nullptr, &config, &mDevice);
            if (result != MA_SUCCESS) {
                Tools::errorf("MiniAudio: Failed to initialize device: %s (Code: %d)\n", ma_result_description(result), result);
                ma_pcm_rb_uninit(&mRingBuffer);
                return false;
            }

            result = ma_device_start(&mDevice);
            if (result != MA_SUCCESS) {
                Tools::errorf("MiniAudio: Failed to start device: %s (Code: %d)\n", ma_result_description(result), result);
                ma_pcm_rb_uninit(&mRingBuffer);
                return false;
            }
            mClosed = false;
            return true;
        }

        // ------------ static Register Methods and Fields
         static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            // Methods
            closeProp = ValueObjectProperty("close", 0,0
            , "call the auto object" , TypeMiniAudioCallBackObject);

            writeProp = ValueObjectProperty("write", 0,1
            , "commit the pending bytes into audio stream." , TypeMiniAudioCallBackObject);

            setProp = ValueObjectProperty("set", 2,2
            , "set a float value at index to the buffer. @params uint index float value" , TypeMiniAudioCallBackObject);

            getProp = ValueObjectProperty("get", 1,1
            , "get the float value at index. @params uint index" , TypeMiniAudioCallBackObject);



            availableProp  = ValueObjectProperty("getAvail",0,0,"floats avaiable", TypeMiniAudioCallBackObject);
            sampleRateProp = ValueObjectProperty("getSampleRate",0,0,"get the samplerate", TypeMiniAudioCallBackObject);
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        // ------------ Handle a Method Call
         bool onMethodCall(Environment& env,uint32_t methodId, std::span<Value> args, Value& ret) override {

            if (mClosed) {
                ret = NumberNullValue;
                return true;
            }

            // ------- close
            if (closeProp.matchMethod( methodId , args) == 1) {
                if (!mClosed) {
                    Close();
                }
                ret = Value(1);
                return true;
            }
            // ------- write
            else if (writeProp.matchMethod( methodId , args) == 1) {
                commitPendingSamples();
                ret = Value(1);
                return true;
            }
            // ------- set
            else if (setProp.matchMethod( methodId , args) == 1) {
                uint32_t n =  args[0].getUInt();
                if (n >= mBufferSize) {
                    Tools::errorf("MiniAudioObject->set buffer overflow!\n");
                    ret = NumberNullValue;
                    return true;
                }
                float value = args[1].getFloat();
                mAudioBuffer[n] = value;
                ret = Value(1);
                return true;
            }
            // ------- get
            else if (getProp.matchMethod(methodId, args) == 1) {
                uint32_t n = args[0].getUInt();
                if (n >= mBufferSize) {
                    Tools::errorf("AudioObject->get buffer overflow!\n");
                    ret = NumberNullValue;
                    return true;
                }
                ret = Value(static_cast<double>(mAudioBuffer[n]));
                return true;
            }
            // ------- get available buffer bytes
            else if (availableProp.matchField(methodId)) {
                ret = Value(getAvail());
                return true;
            }
            // ------- get sample rate
            else if (sampleRateProp.matchField( methodId)) {
                ret = Value(mSampleRate);
                return true;
            }
             return ValueObject::onMethodCall(env,methodId, args, ret);
        }
        // -------------------------------------------------------------------------
    }; //  struct MiniAudioObject

    // -------------------------------------------------------------------------
    //  Static Functions and Constants registry:
    // -------------------------------------------------------------------------
    void RegisterMiniAudioFunctions(Environment& env) {
        static bool registered = false; if (registered) return; registered = true;
        using namespace FunctionMap;
        // --------------------
        MiniAudioCallBackObject::RegisterSymbols();
        // --------------------
        RegisterFunction("MiniAudioCallback::new", [](std::span<Value> args, Value& ret) -> bool {
            MiniAudioCallBackObject* obj = new MiniAudioCallBackObject();
            U32 defSR = 8000; U32 defChannel = 1; //8k mono ;)
            if (args.size() > 0) defSR = args[0].getUInt();
            if (args.size() > 1) defChannel = args[1].getUInt();
            if (!obj->Open(defSR, defChannel)) {
                return false;
            }
            ret = Value(obj);
            return true;
        }, "Initialize MiniAudioObject: MiniAudio::new [sampleRate:8000] [channel:1]");

    }
} //namespace DreiZehn
