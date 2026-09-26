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

        inline static ValueObjectProperty mPush;
        inline static ValueObjectProperty mPop;
        inline static ValueObjectProperty mSize;
        inline static ValueObjectProperty mGet;
        inline static ValueObjectProperty mAt;
        inline static ValueObjectProperty mSet;
        inline static ValueObjectProperty mBack;
        inline static ValueObjectProperty mFront;
        inline static ValueObjectProperty mAppend;
        inline static ValueObjectProperty mClear;
        inline static ValueObjectProperty mPrint;

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;
            //  ValueObjectProperty(std::string name,  uint32_t minParams, uint32_t maxParams, std::string help)
            mPush  = ValueObjectProperty("push", 1,1, "push a value to the end of the Array. @param Value");
            RegisterObjectProperty(TypeArrayObject, mPush);
            mPop   = ValueObjectProperty("pop", 0,0,  "pop the last value and return it");
            RegisterObjectProperty(TypeArrayObject, mPop);
            mSize  = ValueObjectProperty("size", 0,0, "get to size (count)");
            RegisterObjectProperty(TypeArrayObject, mSize);
            mGet   = ValueObjectProperty("get", 1,1,  "get a value at index. @param index");
            RegisterObjectProperty(TypeArrayObject, mGet);
            mAt    = ValueObjectProperty("at", 1,1,   "get a value at index. @param index");
            RegisterObjectProperty(TypeArrayObject, mAt);
            mSet   = ValueObjectProperty("set", 2,2,  "set a value at index. @param index, @param Value");
            RegisterObjectProperty(TypeArrayObject, mSet);
            mFront   = ValueObjectProperty("front", 0,0,  "get the first value");
            RegisterObjectProperty(TypeArrayObject, mFront);
            mBack   = ValueObjectProperty("back", 0,0,  "get the last value");
            RegisterObjectProperty(TypeArrayObject, mBack);
            mAppend   = ValueObjectProperty("append", 1,256,  "append up to 256 arguments to the end");
            RegisterObjectProperty(TypeArrayObject, mAppend);

            mClear   = ValueObjectProperty("clear", 0,0,  "clear the list.");
            RegisterObjectProperty(TypeArrayObject, mClear);
            mPrint   = ValueObjectProperty("print", 0,0,  "print the values");
            RegisterObjectProperty(TypeArrayObject, mPrint);
            mSymbolsLoaded = true;
        }

        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override {


            if ( methodId == mPush.mSymbolId ) {
                if (!mPush.ValidateArgs(args)) return false;
                if (args[0].isPointer()) static_cast<ValueObject*>(args[0].asPointer())->setAssigned(true);
                mElements.push_back(args[0]);
                ret = Value(args[0]);
                return true;
            }
            else
            if (methodId == mPop.mSymbolId) {
                if (!mPop.ValidateArgs(args)) return false;
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
            if (methodId == mSize.mSymbolId) {
                if (!mSize.ValidateArgs(args)) return false;
                ret = Value(static_cast<int>(mElements.size()));
                return true;
            }
            else
            if (methodId == mGet.mSymbolId|| methodId == mAt.mSymbolId) {
                if (!mGet.ValidateArgs(args)) return false;
                if (mElements.size() > args[0].getInt()) {
                    ret = Value(mElements.at(args[0].getInt()));
                }
                return true;
            }
            else
            if (methodId == mSet.mSymbolId)  {
                if (!mSet.ValidateArgs(args)) return false;
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
            if (methodId == mFront.mSymbolId) {
                if (!mFront.ValidateArgs(args)) return false;
                ret = Value(mElements.front());
                return true;
            }
            else
            if (methodId == mBack.mSymbolId) {
                if (!mBack.ValidateArgs(args)) return false;
                ret = Value(mElements.back());
                return true;
            }
            else
            if (methodId == mAppend.mSymbolId) {
                if (!mAppend.ValidateArgs(args)) return false;
                for (auto& arg: args) {
                    this->mElements.push_back(arg);
                }
                ret = Value(1);
                return true;
            }
            else
            if (methodId == mClear.mSymbolId) {
                if (!mClear.ValidateArgs(args)) return false;
                this->mElements.clear();
                ret = Value(1);
                return true;
            }
            else
            if (methodId == mPrint.mSymbolId) {
                if (!mPrint.ValidateArgs(args)) return false;
                for (auto& value: mElements) {
                    value.print();
                }
                Tools::printf("\n");
                ret = Value(1);
                return true;
            }
            else
            {
                Tools::errorf("Unknown Array method: %s\n", SymbolTable::getName(methodId).c_str());
            }

            return false;
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


            if (gCurrentFrame) gCurrentFrame->addToGarbageCollection(arr);
            return true;
        });

    }

}
