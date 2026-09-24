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
    struct Vector3 {
        double x = 0.f;
        double y = 0.f;
        double z = 0.f;
    };

    const int TypeVector3Object =  registerUserObjectType("Vector3");

    struct Vector3Object : public ValueObject {
        Vector3 mVec = {0};

        Vector3Object() : ValueObject(TypeVector3Object) { initSymbols(); }
        ~Vector3Object() { }

        inline static ValueObjectMethod mX;
        inline static ValueObjectMethod mY;
        inline static ValueObjectMethod mZ;

        inline static void initSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            mX  = ValueObjectMethod("x", 0,0, "get x");
            mY  = ValueObjectMethod("y", 0,0, "get y");
            mZ  = ValueObjectMethod("z", 0,0, "get z");
            mSymbolsLoaded = true;
        }

        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override {

            if ( methodId == mX.mSymbolId ) { ret =  Value(mVec.x); }
            if ( methodId == mY.mSymbolId ) { ret =  Value(mVec.y); }
            if ( methodId == mZ.mSymbolId ) { ret =  Value(mVec.z); }
            else return false;

            return true;
        }
    };
    // -------------------------------------------------------------------------
    void RegisterVectorObjectFunctions() {
        using namespace FunctionMap;

        RegisterFunction("Vector3.new", [](std::vector<Value>& args, Value& ret) -> bool {
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
