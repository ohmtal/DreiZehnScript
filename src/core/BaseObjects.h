//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Base Objects
// - Object the implementation of the base ValueObject
// - Struct with fixed fields defined on constructor
// - Array also using the Dynamic Fields like Object and Struct but allowes n
//         non variable conform key names and field getter is disabled.
//-----------------------------------------------------------------------------
#pragma once

#include <algorithm>
#include "core/Value.h"
#include "core/ValueObject.h"
#include "core/FunctionMap.h"
#include "VariableFrame.h"


namespace DreiZehn {

    inline const int TypeKeyValueObject =  RegisterUserObjectType("KeyValue");
    inline const int TypeBaseObject =  RegisterUserObjectType("Object");
    inline const int TypeStructObject =  RegisterUserObjectType("Struct");
    inline const int TypeArrayObject =  RegisterUserObjectType("Array");

    // -------------------------------------------------------------------------
    struct KeyValueObject : public ValueObject {
        // inline static ValueObjectProperty mKeyProp, mValueProp;
        uint32_t mKeyId = 0;
        Value mValue = NumberNullValue;
        KeyValueObject() ;
        KeyValueObject(const std::string& key, const Value& value);
        std::string  toString() override;
        // -------------------------------------------------------------------------
        static void RegisterSymbols();
        // bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override;
        // -------------------------------------------------------------------------
        void Dump() override;
    };
    // -------------------------------------------------------------------------
    struct BaseValueObject : public ValueObject {
        BaseValueObject() : ValueObject(TypeBaseObject) {
            mSupportClone = true;

        }
         ValueObject* clone() override {
            BaseValueObject* clone = new BaseValueObject();
            ValueObject::cloneBase(clone);
            return clone;
        }
    };
    // -------------------------------------------------------------------------
    struct StructObject : public ValueObject {

        inline static ValueObjectProperty mSetProp;

        StructObject() : ValueObject(TypeStructObject) {
            mSupportClone = true;
            mAutoCreateDynamicFields = false;
        }

        StructObject(std::vector<Value>& args) ;
        // -------------------------------------------------------------------------
         ValueObject* clone() override {
            StructObject* clone = new StructObject();
            ValueObject::cloneBase(clone);
            return clone;
        }
        // -------------------------------------------------------------------------
        // .... array by integer index ....
        inline size_t onGetArraySize() override{
            return mDynamicFieldValues.size();
        }
        // -------------------------------------------------------------------------
        inline Value* onGetArrayIndexPtr(size_t arrayIndex) override{
            return getDynamicFieldPtrByIndex(arrayIndex);
        }
        // .... array by string key ....
        inline Value* onGetArrayByStringPtr(std::string strKey) override{
            return getDynamicFieldPtr(SymbolTable::insert( strKey ));
        }

        // -------------------------------------------------------------------------

        static void RegisterSymbols();
        // -------------------------------------------------------------------------
        bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override;

    };
    // -------------------------------------------------------------------------
    struct ArrayObject : public ValueObject {

        inline static ValueObjectProperty mCountProp, mValueProp, mKeyProp, mTreeProp;

        ArrayObject() : ValueObject(TypeArrayObject) {
            mSupportClone = true;
            mValidateFieldName = false;
        }

        ArrayObject(std::vector<Value>& args);
        // -------------------------------------------------------------------------
         ValueObject* clone() override;
        // -------------------------------------------------------------------------
        bool onGetField(uint32_t fieldSymbolId, Value& ret) override{
            return false;
        }
        Value* onGetFieldPtr(uint32_t fieldSymbolId ) override{
            return nullptr;
        }
        // -------------------------------------------------------------------------
        // .... array by integer index ....
        inline size_t onGetArraySize() override{
            return mDynamicFieldValues.size();
        }
        // -------------------------------------------------------------------------
        inline Value* onGetArrayIndexPtr(size_t arrayIndex) override{
            return getDynamicFieldPtrByIndex(arrayIndex);
        }
        // .... array by string key ....
        inline Value* onGetArrayByStringPtr(std::string strKey) override{
            uint32_t id = SymbolTable::insert( strKey );
            Value* valPtr =  getDynamicFieldPtr(id);
            if (valPtr) return valPtr;
            insertUpdateDynamicField(id, NumberNullValue);
            return getDynamicFieldPtr(id);
        }

        // -------------------------------------------------------------------------
        void PrintTree(int depth = 0);

       // -------------------------------------------------------------------------
        static void RegisterSymbols();
        // -------------------------------------------------------------------------
        bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override;
        // -------------------------------------------------------------------------
    };
// ============================ RegisterBaseObject ==================================
void RegisterBaseObject(Environment& env);

} //namespace

