//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Array - a simple one dimensional Array

//-----------------------------------------------------------------------------
// GarbageCollection for objects works fine:
// a = Array.new; b = Array.new; a->push b; b = 0;debug.garbage
// c = a->back; debug.garbage
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include "core/Value.h"
#include "core/ValueObject.h"
#include "core/FunctionMap.h"

namespace DreiZehn {


    const int TypeArrayObject =  RegisterUserObjectType("Array");

    struct ArrayValueObject : public ValueObject {
        std::vector<Value> mElements;

        ArrayValueObject() : ValueObject(TypeArrayObject) {  }
        ~ArrayValueObject() {
            // GarbageCollection: cleanup assigned flags for object members
            for (auto e: mElements) {
                if (e.isPointer())  static_cast<ValueObject*>(e.asPointer())->setAssigned(false);
            }
        }

        inline static ValueObjectProperty mPushProp;
        inline static ValueObjectProperty mPopProp;
        inline static ValueObjectProperty mSizeProp;
        inline static ValueObjectProperty mGetProp;
        inline static ValueObjectProperty mAtProp;
        inline static ValueObjectProperty mSetProp;
        inline static ValueObjectProperty mBackProp;
        inline static ValueObjectProperty mFrontProp;
        inline static ValueObjectProperty mAppendProp;
        inline static ValueObjectProperty mClearProp;
        inline static ValueObjectProperty mPrintProp;

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;
            //  ValueObjectProperty(std::string name,  uint32_t minParams, uint32_t maxParams, std::string help)
            mPushProp  = ValueObjectProperty("push", 1,1, "push a value to the end of the Array. @param Value", TypeArrayObject);
            mPopProp   = ValueObjectProperty("pop", 0,0,  "pop the last value and return it", TypeArrayObject);
            mSizeProp  = ValueObjectProperty("size", 0,0, "get to size (count)", TypeArrayObject);
            mGetProp   = ValueObjectProperty("get", 1,1,  "get a value at index. @param index", TypeArrayObject);
            mAtProp    = ValueObjectProperty("at", 1,1,   "get a value at index. @param index", TypeArrayObject);
            mSetProp   = ValueObjectProperty("set", 2,2,  "set a value at index. @param index, @param Value", TypeArrayObject);
            mFrontProp   = ValueObjectProperty("front", 0,0,  "get the first value", TypeArrayObject);
            mBackProp   = ValueObjectProperty("back", 0,0,  "get the last value", TypeArrayObject);
            mAppendProp   = ValueObjectProperty("append", 1,256,  "append up to 256 arguments to the end", TypeArrayObject);

            mClearProp   = ValueObjectProperty("clear", 0,0,  "clear the list.", TypeArrayObject);
            mPrintProp   = ValueObjectProperty("print", 0,0,  "print the values", TypeArrayObject);
            mSymbolsLoaded = true;
        }

        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override {


            if ( methodId == mPushProp.mSymbolId ) {
                if (!mPushProp.ValidateArgs(args)) return false;
                if (args[0].isPointer()) static_cast<ValueObject*>(args[0].asPointer())->setAssigned(true);
                mElements.push_back(args[0]);
                ret = Value(args[0]);
                return true;
            }
            else
            if (methodId == mPopProp.mSymbolId) {
                if (!mPopProp.ValidateArgs(args)) return false;
                if (mElements.size() > 0) {
                    ret = Value(mElements.back());
                    if (ret.isPointer()) static_cast<ValueObject*>(ret.asPointer())->setAssigned(false);
                    mElements.pop_back();
                } else {
                    ret= Value();
                }
                return true;
            }
            else
            if (methodId == mSizeProp.mSymbolId) {
                if (!mSizeProp.ValidateArgs(args)) return false;
                ret = Value(static_cast<int>(mElements.size()));
                return true;
            }
            else
            if (methodId == mGetProp.mSymbolId|| methodId == mAtProp.mSymbolId) {
                if (!mGetProp.ValidateArgs(args)) return false;
                if (mElements.size() > args[0].getInt()) {
                    ret = Value(mElements.at(args[0].getInt()));
                }
                return true;
            }
            else
            if (methodId == mSetProp.mSymbolId)  {
                if (!mSetProp.ValidateArgs(args)) return false;
                // slowdown a bit but need it for GarbageCollection
                Value pre = mElements[args[0].getInt()];
                if (pre.isPointer()) static_cast<ValueObject*>(pre.asPointer())->setAssigned(false);
                // ------- i guess i need a assinged counter !!!

                mElements[args[0].getInt()] = args[1];
                ret =  args[1];
                if (ret.isPointer()) static_cast<ValueObject*>(ret.asPointer())->setAssigned(true);
                return true;

            }
            else
            if (methodId == mFrontProp.mSymbolId) {
                if (!mFrontProp.ValidateArgs(args)) return false;
                ret = Value(mElements.front());
                return true;
            }
            else
            if (methodId == mBackProp.mSymbolId) {
                if (!mBackProp.ValidateArgs(args)) return false;
                ret = Value(mElements.back());
                return true;
            }
            else
            if (methodId == mAppendProp.mSymbolId) {
                if (!mAppendProp.ValidateArgs(args)) return false;
                for (auto& arg: args) {
                    this->mElements.push_back(arg);
                }
                ret = Value(1);
                return true;
            }
            else
            if (methodId == mClearProp.mSymbolId) {
                if (!mClearProp.ValidateArgs(args)) return false;
                this->mElements.clear();
                ret = Value(1);
                return true;
            }
            else
            if (methodId == mPrintProp.mSymbolId) {
                if (!mPrintProp.ValidateArgs(args)) return false;
                for (auto& value: mElements) {
                    value.print();
                }
                Tools::printf("\n");
                ret = Value(1);
                return true;
            }

            return ValueObject::onMethodCall(methodId, args, ret);
        }
    };
    // -------------------------------------------------------------------------
    void RegisterArrayFunctions(Environment& env) {
        // init Methods:
        ArrayValueObject::RegisterSymbols();


        using namespace FunctionMap;

        RegisterFunction("Array::new", [&env](std::vector<Value>& args, Value& ret) -> bool {
            ArrayValueObject* arr = new ArrayValueObject();
            ret = Value(arr);

            // add args!
            for (auto& arg: args) {
                arr->mElements.push_back(arg);
            }


            return true;
        });

    }

}
