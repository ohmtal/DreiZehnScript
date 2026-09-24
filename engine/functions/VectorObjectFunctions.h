//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Vector Objects .. like Vector 3
//-----------------------------------------------------------------------------
// TODO impement fields ...
//      - setter FIXME
//      - getter is onMethodCall
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include "core/Value.h"
#include "core/ValueObject.h"
#include "core/FunctionMap.h"
#include <core/VariableFrame.h>

namespace DreiZehn {
    // using value so i can directly link it
    struct Vector3 {
        Value x = Value(0.0);
        Value y = Value(0.0);
        Value z = Value(0.0);
    };

    const int TypeVector3Object =  registerUserObjectType("Vector3");

    struct Vector3Object : public ValueObject {
        Vector3 mVec = {0};


        Vector3Object() : ValueObject(TypeVector3Object) { initSymbols(); }
        ~Vector3Object() { }

        inline static ValueObjectMethod mToString;
        inline static ValueObjectMethod mNormalize;
        inline static uint32_t xId = 0;
        inline static uint32_t yId = 0;
        inline static uint32_t zId = 0;

        inline static void initSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            xId = SymbolTable::insert("x");
            yId = SymbolTable::insert("y");
            zId = SymbolTable::insert("z");

            mToString   = ValueObjectMethod("toString", 0,0, "return vector as string");
            mNormalize  = ValueObjectMethod("normalize", 0,0, "normalize vector");
            mSymbolsLoaded = true;
        }

        // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override{
            if (fieldSymbolId == xId) mVec.x = Value(value.getDouble());
            else if (fieldSymbolId == yId) mVec.y = Value(value.getDouble());
            else if (fieldSymbolId == zId) mVec.z = Value(value.getDouble());
            else return false;
            return true;
        }
        // -------------------------------------------------------------------------
        inline bool onGetField(uint32_t fieldSymbolId, Value& ret) override{
            if (fieldSymbolId == xId) ret = mVec.x;
            else if (fieldSymbolId == yId) ret = mVec.y;
            else if (fieldSymbolId == zId) ret = mVec.z;
            else return false;
            return true;
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override {

            if ( methodId == mToString.mSymbolId ) { /*FIXME*/ return false;}
            if ( methodId == mNormalize.mSymbolId ) { /*FIXME*/ return false;}
            else return false;

            return true;
        }
    };
    // -------------------------------------------------------------------------
    void RegisterVectorObjectFunctions() {
        using namespace FunctionMap;

        Vector3Object::initSymbols();

        RegisterFunction("Vector3:new", [](std::vector<Value>& args, Value& ret) -> bool {
            Vector3Object* v = new Vector3Object();
            if (args.size() > 0 ) v->mVec.x = args[0].getDouble();
            if (args.size() > 1 ) v->mVec.y = args[1].getDouble();
            if (args.size() > 2 ) v->mVec.z = args[2].getDouble();
            ret = Value(v);
            if (gCurrentFrame) gCurrentFrame->addToGarbageCollection(v);
            return true;
        });

    }

}
