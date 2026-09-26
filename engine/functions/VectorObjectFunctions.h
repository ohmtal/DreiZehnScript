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

            mXprop   = ValueObjectProperty("x","return double X");
            RegisterObjectProperty(TypeVector3Object, mXprop);

            mYprop   = ValueObjectProperty("y","return double Y");
            RegisterObjectProperty(TypeVector3Object, mYprop);

            mZprop   = ValueObjectProperty("z","return double Z");
            RegisterObjectProperty(TypeVector3Object, mZprop);


            // TODO
            mNormalize  = ValueObjectProperty("normalize", 0,0, "normalize vector");
            RegisterObjectProperty(TypeVector3Object, mNormalize);

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

            if ( methodId == mToString.mSymbolId ) { /*TODO*/ return false;}
            if ( methodId == mNormalize.mSymbolId ) { /*TODO*/ return false;}
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
