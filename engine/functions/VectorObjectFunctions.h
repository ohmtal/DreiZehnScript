//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Vector Objects: Vector3,
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

    const int TypeVector3Object =  RegisterUserObjectType("Vector3");

    struct Vector3Object : public ValueObject {
        Vector3 mVec = {0};


        Vector3Object() : ValueObject(TypeVector3Object) { }
        ~Vector3Object() { }

        inline static ValueObjectProperty mToString;
        inline static ValueObjectProperty mNormalize;
        inline static ValueObjectProperty mXprop;
        inline static ValueObjectProperty mYprop;
        inline static ValueObjectProperty mZprop;

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            mXprop   = ValueObjectProperty("x","return double X", TypeVector3Object);

            mYprop   = ValueObjectProperty("y","return double Y", TypeVector3Object);

            mZprop   = ValueObjectProperty("z","return double Z", TypeVector3Object);

            mToString   = ValueObjectProperty("toString", 0,0, "return as string", TypeVector3Object);
            // mNormalize  = ValueObjectProperty("normalize", 0,0, "normalize vector", TypeVector3Object);

            mSymbolsLoaded = true;
        }

        // -------------------------------------------------------------------------
        inline Value* onGetFieldPtr(uint32_t fieldSymbolId) override {
            if (fieldSymbolId == mXprop.mSymbolId) return &mVec.x;
            else if (fieldSymbolId == mYprop.mSymbolId) return &mVec.y ;
            else if (fieldSymbolId == mZprop.mSymbolId) return &mVec.z ;

            return nullptr;
        }
        // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override{
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (!ptr) return false;
            *ptr = Value(value.getDouble());
            return true;
        }
        // -------------------------------------------------------------------------
        inline bool onGetField(uint32_t fieldSymbolId, Value& ret) override{
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (!ptr) return false;
            ret = *ptr;
            return true;

        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override {

            if (mToString.matchMethod( methodId , args) == 1) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%f %f %f", mVec.x.getDouble(), mVec.y.getDouble(), mVec.z.getDouble());
                ret = Value(std::string(buffer));
            }
//             else if ( methodId == mNormalize.mSymbolId ) {
//
//
//             }
            else return false;

            return true;
        }
    };
    // -------------------------------------------------------------------------
    void RegisterVectorObjectFunctions() {
        using namespace FunctionMap;

        Vector3Object::RegisterSymbols();

        RegisterFunction("Vector3::new", [](std::vector<Value>& args, Value& ret) -> bool {
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
