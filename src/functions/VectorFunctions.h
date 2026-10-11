//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Vector - a simple one dimensional Vector

//-----------------------------------------------------------------------------
// GarbageCollection for objects works fine:
// a = Vector.new; b = Vector.new; a->push b; b = 0;debug.garbage
// c = a->back; debug.garbage
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include "core/Value.h"
#include "core/ValueObject.h"
#include "core/FunctionMap.h"

namespace DreiZehn {


    const int TypeVectorObject =  RegisterUserObjectType("Vector");

    struct VectorValueObject : public ValueObject {
        std::vector<Value> mElements;

        VectorValueObject() : ValueObject(TypeVectorObject) {  }
        ~VectorValueObject() {
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
        inline static ValueObjectProperty mDumpProp;
        inline static ValueObjectProperty mFillProp;

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;
            //  ValueObjectProperty(std::string name,  uint32_t minParams, uint32_t maxParams, std::string help)
            mPushProp  = ValueObjectProperty("push", 1,1, "push a value to the end of the Vector. @param Value", TypeVectorObject);
            mPopProp   = ValueObjectProperty("pop", 0,0,  "pop the last value and return it", TypeVectorObject);
            mSizeProp  = ValueObjectProperty("size", 0,0, "get to size (count)", TypeVectorObject);
            mGetProp   = ValueObjectProperty("get", 1,1,  "get a value at index. @param index", TypeVectorObject);
            mAtProp    = ValueObjectProperty("at", 1,1,   "get a value at index. @param index", TypeVectorObject);
            mSetProp   = ValueObjectProperty("set", 2,2,  "set a value at index. @param index, @param Value", TypeVectorObject);
            mFrontProp   = ValueObjectProperty("front", 0,0,  "get the first value", TypeVectorObject);
            mBackProp   = ValueObjectProperty("back", 0,0,  "get the last value", TypeVectorObject);
            mAppendProp   = ValueObjectProperty("append", 1,256,  "append up to 256 arguments to the end", TypeVectorObject);

            mClearProp   = ValueObjectProperty("clear", 0,0,  "clear the list.", TypeVectorObject);
            mDumpProp   = ValueObjectProperty("dump", 0,0,  "dump this object", TypeVectorObject);

            mFillProp   = ValueObjectProperty("fill", 2,2,  "fill the vector with n values", TypeVectorObject);
            mSymbolsLoaded = true;
        }

        // -------------------------------------------------------------------------
        Value* onMethodCallGetAssignPtr(uint32_t methodNameSymbolId,  std::vector<Value>& args) override{

            if (mAtProp.matchMethod(methodNameSymbolId, args)) {
                if (mElements.size() > args[0].getInt()) {
                    return &mElements.at(args[0].getInt());
                }
                return nullptr;
            }
            else if (mGetProp.matchMethod(methodNameSymbolId, args)) {
                if (mElements.size() > args[0].getInt()) {
                    return &mElements.at(args[0].getInt());
                }
                return nullptr;
            }

            return nullptr;
        }
        // -------------------------------------------------------------------------
        inline size_t onGetArraySize() override{
            return mElements.size();
        }
        inline Value* onGetArrayIndexPtr(size_t arrayIndex) override{
            if (mElements.size() > arrayIndex) {
                return  &mElements.at(arrayIndex);
            }
            return nullptr;
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
            if (mDumpProp.matchMethod(methodId,args)) {
                this->DumpBase();
                Tools::printSeparator(40);
                Tools::printf("----- Vector Values -----\n");
                for (auto& value: mElements) {
                     Tools::printf("%s ",value.toString().c_str());
                }
                Tools::printf("\n");
                ret = Value(1);
                return true;
            }
            else
            if (mFillProp.matchMethod(methodId, args)) {

                // set GarbageCollection lock/unlock
                GarbageCollectionLockGuard guard;

                if (!args[0].isNumber()) {
                    Tools::errorf("Usage ->fill count value");
                    ret = Value(0);
                    return true;
                }
                ValueObject* cloneParent = nullptr;
                if (args[1].isPointer()) {
                    cloneParent = args[1].asPointerObject();
                    if (!cloneParent->mSupportClone) {
                        Tools::errorf("Sorry the object %s does not support cloneing!"
                            , cloneParent->mClassName.c_str());
                        ret = Value(0);
                        return true;
                    }
                }

                uint32_t count = args[0].getUInt();

                mElements.clear();
                mElements.reserve(count);
                if (cloneParent != nullptr) {
                    ValueObject* clone = nullptr;
                    for (uint32_t i = 0; i < count; i++){
                        clone = cloneParent->clone();
                        if (!clone) {
                            Tools::errorf("Runtime Error: fill Vector: Cloning of %s failed!"
                                , cloneParent->mClassName.c_str());
                            ret = Value(0);
                            return false;
                        }

                        mElements.push_back(clone);
                        clone->setAssigned(true);
                    }
                } else {
                    for (uint32_t i = 0; i < count; i++){
                        mElements.push_back(args[1]);
                    }
                }

                ret = Value(count);
                return true;
            }


            return ValueObject::onMethodCall(methodId, args, ret);
        }
    };
    // -------------------------------------------------------------------------
    void RegisterVectorFunctions(Environment& env) {
        static bool registered = false;
        if (registered) return;
        registered = true;
        // init Methods:
        VectorValueObject::RegisterSymbols();

        using namespace FunctionMap;
        RegisterFunction("Vector::new", [](std::vector<Value>& args, Value& ret) -> bool {
            VectorValueObject* arr = new VectorValueObject();
            ret = Value(arr);

            // add args!
            for (auto& arg: args) {
                if (arg.isPointer()) {
                    arg.asPointerObject()->setAssigned(true);
                }
                arr->mElements.push_back(arg);
            }
            return true;
        });
    }




}
