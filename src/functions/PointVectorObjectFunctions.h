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
    struct Vector2 {
        Value x = Value(0.0);
        Value y = Value(0.0);
    };

    struct Vector3 {
        Value x = Value(0.0);
        Value y = Value(0.0);
        Value z = Value(0.0);
    };

    struct Vector4 {
        Value x = Value(0.0);
        Value y = Value(0.0);
        Value z = Value(0.0);
        Value w = Value(0.0);
    };


    inline const int TypeVector3Object =  RegisterUserObjectType("Vector3");
    inline const int TypeVector2Object =  RegisterUserObjectType("Vector2");
    inline const int TypeVector4Object =  RegisterUserObjectType("Vector4");

    // -------------------------------------------------------------------------
    // BasePointObject
    // -------------------------------------------------------------------------
    struct BasePointObject : public ValueObject {

    public:
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
    protected:
        // -------------------------------------------------------------------------
        BasePointObject(int t) : ValueObject(t) {
            mSupportClone = true;
        }
    };

    // -------------------------------------------------------------------------
    // Vector3Object
    // -------------------------------------------------------------------------
    struct Vector3Object : public BasePointObject {
        Vector3 mVec = {0};


        Vector3Object() : BasePointObject(TypeVector3Object) {}

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
        inline bool onMethodCall(uint32_t methodId,  std::span<Value> args, Value& ret) override {

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
    // Vector2Object
    // -------------------------------------------------------------------------
    struct Vector2Object : public BasePointObject {
        Vector2 mVec = {0};


        Vector2Object() : BasePointObject(TypeVector2Object) {}
        ~Vector2Object() { }

        virtual ValueObject* clone() override {
            Vector2Object* clone = new Vector2Object();
            ValueObject::cloneBase(clone);
            clone->mVec = this->mVec;
            return clone;
        }

        inline static ValueObjectProperty mToStringProp;
        inline static ValueObjectProperty mSetProp;
        inline static ValueObjectProperty mXprop;
        inline static ValueObjectProperty mYprop;

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            mXprop   = ValueObjectProperty("x","return double X", TypeVector2Object);

            mYprop   = ValueObjectProperty("y","return double Y", TypeVector2Object);


            mToStringProp   = ValueObjectProperty("toString", 0,0, "return as string", TypeVector2Object);
            mSetProp  = ValueObjectProperty("set", 1,3, "set new values", TypeVector2Object);

            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline std::string toString() override{
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%f %f", mVec.x.getDouble(), mVec.y.getDouble());
            return std::string(buffer);
        }
        // -------------------------------------------------------------------------
        inline Value* onGetFieldPtr(uint32_t fieldSymbolId) override {
            if (fieldSymbolId == mXprop.mSymbolId) return &mVec.x;
            else if (fieldSymbolId == mYprop.mSymbolId) return &mVec.y ;

            return  ValueObject::onGetFieldPtr(fieldSymbolId);
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::span<Value> args, Value& ret) override {

            if (mToStringProp.matchMethod( methodId , args) == 1) {
                ret = Value(toString());
                return true;
            }
            else if (mSetProp.matchMethod( methodId , args) == 1) {
                if (args.size() > 0 ) mVec.x = args[0].getDouble();
                if (args.size() > 1 ) mVec.y = args[1].getDouble();
                ret = Value(1);
                return true;
            }

            return ValueObject::onMethodCall(methodId, args, ret);

        }
    };

    // -------------------------------------------------------------------------
    // Vector4Object
    // -------------------------------------------------------------------------
    struct Vector4Object : public BasePointObject {
        Vector4 mVec = {0};


        Vector4Object() : BasePointObject(TypeVector4Object) {}
        ~Vector4Object() { }

        virtual ValueObject* clone() override {
            Vector4Object* clone = new Vector4Object();
            ValueObject::cloneBase(clone);
            clone->mVec = this->mVec;
            return clone;
        }

        inline static ValueObjectProperty mToStringProp;
        inline static ValueObjectProperty mSetProp;
        inline static ValueObjectProperty mXprop;
        inline static ValueObjectProperty mYprop;
        inline static ValueObjectProperty mZprop;
        inline static ValueObjectProperty mWprop;

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;

            mXprop   = ValueObjectProperty("x","return double X", TypeVector4Object);

            mYprop   = ValueObjectProperty("y","return double Y", TypeVector4Object);

            mZprop   = ValueObjectProperty("z","return double Z", TypeVector4Object);

            mWprop   = ValueObjectProperty("w","return double W", TypeVector4Object);

            mToStringProp   = ValueObjectProperty("toString", 0,0, "return as string", TypeVector4Object);
            mSetProp  = ValueObjectProperty("set", 1,3, "set new values", TypeVector4Object);

            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline std::string toString() override{
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%f %f %f %f",
                     mVec.x.getDouble(), mVec.y.getDouble(), mVec.z.getDouble(), mVec.w.getDouble());
            return std::string(buffer);
        }
        // -------------------------------------------------------------------------
        inline Value* onGetFieldPtr(uint32_t fieldSymbolId) override {
            if (fieldSymbolId == mXprop.mSymbolId) return &mVec.x;
            else if (fieldSymbolId == mYprop.mSymbolId) return &mVec.y ;
            else if (fieldSymbolId == mZprop.mSymbolId) return &mVec.z ;
            else if (fieldSymbolId == mWprop.mSymbolId) return &mVec.w ;

            return  ValueObject::onGetFieldPtr(fieldSymbolId);
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::span<Value> args, Value& ret) override {

            if (mToStringProp.matchMethod( methodId , args) == 1) {
                ret = Value(toString());
                return true;
            }
            else if (mSetProp.matchMethod( methodId , args) == 1) {
                if (args.size() > 0 ) mVec.x = args[0].getDouble();
                if (args.size() > 1 ) mVec.y = args[1].getDouble();
                if (args.size() > 2 ) mVec.z = args[2].getDouble();
                if (args.size() > 2 ) mVec.w = args[3].getDouble();
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
        Vector2Object::RegisterSymbols();
        Vector4Object::RegisterSymbols();

        RegisterFunction("Vector3::new", [](std::span<Value> args, Value& ret) -> bool {
            Vector3Object* v = new Vector3Object();
            if (args.size() > 0 ) v->mVec.x = args[0].getDouble();
            if (args.size() > 1 ) v->mVec.y = args[1].getDouble();
            if (args.size() > 2 ) v->mVec.z = args[2].getDouble();
            ret = Value(v);
            return true;
        }, "Return a new Vector3 (3D floating point) Object. Optional x,y,z");

        RegisterFunction("Vector2::new", [](std::span<Value> args, Value& ret) -> bool {
            Vector2Object* v = new Vector2Object();
            if (args.size() > 0 ) v->mVec.x = args[0].getDouble();
            if (args.size() > 1 ) v->mVec.y = args[1].getDouble();
            ret = Value(v);
            return true;
        }, "Return a new Vector2 (2D floating point) Object. Optional x,y");

        RegisterFunction("Vector4::new", [](std::span<Value> args, Value& ret) -> bool {
            Vector4Object* v = new Vector4Object();
            if (args.size() > 0 ) v->mVec.x = args[0].getDouble();
            if (args.size() > 1 ) v->mVec.y = args[1].getDouble();
            if (args.size() > 2 ) v->mVec.z = args[2].getDouble();
            if (args.size() > 3 ) v->mVec.w = args[3].getDouble();
            ret = Value(v);
            return true;
        }, "Return a new Vector4 (4D floating point) Object. Optional x,y,z,w");

    }

}
