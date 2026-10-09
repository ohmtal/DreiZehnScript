//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Point Vector Objects: Vector3,
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


        Vector3Object() : ValueObject(TypeVector3Object) {
            mSupportClone = true;
        }
        ~Vector3Object() { }

        virtual ValueObject* clone() override {
            Vector3Object* clone = new Vector3Object();
            ValueObject::cloneBase(clone);
            clone->mVec = this->mVec;
            return clone;
        }

        inline static ValueObjectProperty mToStringProp;
        inline static ValueObjectProperty mSetProp;
        inline static ValueObjectProperty mXprop;
        inline static ValueObjectProperty mYprop;
        inline static ValueObjectProperty mZprop;

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            mXprop   = ValueObjectProperty("x","return double X", TypeVector3Object);

            mYprop   = ValueObjectProperty("y","return double Y", TypeVector3Object);

            mZprop   = ValueObjectProperty("z","return double Z", TypeVector3Object);

            mToStringProp   = ValueObjectProperty("toString", 0,0, "return as string", TypeVector3Object);
            mSetProp  = ValueObjectProperty("set", 1,3, "set new values", TypeVector3Object);

            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline std::string toString() override{
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%f %f %f", mVec.x.getDouble(), mVec.y.getDouble(), mVec.z.getDouble());
            return std::string(buffer);
        }
        // -------------------------------------------------------------------------
        inline Value* onGetFieldPtr(uint32_t fieldSymbolId) override {
            if (fieldSymbolId == mXprop.mSymbolId) return &mVec.x;
            else if (fieldSymbolId == mYprop.mSymbolId) return &mVec.y ;
            else if (fieldSymbolId == mZprop.mSymbolId) return &mVec.z ;

            return  ValueObject::onGetFieldPtr(fieldSymbolId);
        }
        // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override{
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (!ptr) return ValueObject::onSetField(fieldSymbolId, value);
            *ptr = Value(value.getDouble());
            return true;
        }
        // -------------------------------------------------------------------------
        inline bool onGetField(uint32_t fieldSymbolId, Value& ret) override{
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (!ptr) return ValueObject::onGetField(fieldSymbolId, ret);
            ret = *ptr;
            return true;

        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override {

            if (mToStringProp.matchMethod( methodId , args) == 1) {
                ret = Value(toString());
                return true;
            }
            else if (mSetProp.matchMethod( methodId , args) == 1) {
                if (args.size() > 0 ) mVec.x = args[0].getDouble();
                if (args.size() > 1 ) mVec.y = args[1].getDouble();
                if (args.size() > 2 ) mVec.z = args[2].getDouble();
                ret = Value(1);
                return true;
            }

            return ValueObject::onMethodCall(methodId, args, ret);

        }
    };
    // -------------------------------------------------------------------------
    void RegisterPointVectorObjectFunctions(Environment& env) {
        static bool registered = false; if (registered) return; registered = true;

        using namespace FunctionMap;

        Vector3Object::RegisterSymbols();

        RegisterFunction("Vector3::new", [](std::vector<Value>& args, Value& ret) -> bool {
            Vector3Object* v = new Vector3Object();
            if (args.size() > 0 ) v->mVec.x = args[0].getDouble();
            if (args.size() > 1 ) v->mVec.y = args[1].getDouble();
            if (args.size() > 2 ) v->mVec.z = args[2].getDouble();
            ret = Value(v);
            return true;
        });

    }

}
