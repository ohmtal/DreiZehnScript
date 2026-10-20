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
        // std::string mKey = "";
        uint32_t mKeyId = 0;
        Value mValue = NumberNullValue;
        KeyValueObject() : ValueObject(TypeKeyValueObject) {
            mSupportClone = false;
            mAutoCreateDynamicFields = false;
        }
        KeyValueObject(const std::string& key, const Value& value) : ValueObject(TypeKeyValueObject) {
            mSupportClone = false;
            mAutoCreateDynamicFields = false;
            mKeyId = SymbolTable::insert( key );
            mValue = value;
        }
        std::string  toString() override{
            return std::string(SymbolTable::getName( mKeyId)  + " => " + mValue.toString());
        }
        // -------------------------------------------------------------------------
        void Dump() override {
            ValueObject::Dump();
            Tools::printf("%s => %s\n", SymbolTable::getName( mKeyId).c_str(), mValue.toString().c_str());
        }
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

        StructObject(std::vector<Value>& args) : ValueObject(TypeStructObject) {
            mSupportClone = true;
            mAutoCreateDynamicFields = false;

            for(int64_t i = 0; i <  args.size() ; i++) {
                Value v = NumberNullValue;

                uint32_t id = 0;
                std::string fieldName = "";

                if (args[i].isPointer() && (args[i].asPointerObject()->mType == TypeKeyValueObject )) {
                    KeyValueObject* kv = static_cast<KeyValueObject*>(args[i].asPointerObject());
                    id = kv->mKeyId;
                    fieldName = SymbolTable::getName(id);
                    v = kv->mValue;

                } else if (args[i].isString()) {
                    fieldName = args[i].getStringRef();
                    id = SymbolTable::insert(fieldName);
                } else {
                    continue;
                }

                if (!isValidVariableName(fieldName)) {
                    Tools::warnf("Field `%s` is not a valid variable name\n", fieldName.c_str());
                    continue;
                }

                insertUpdateDynamicField(id, v);
            }
        }
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

        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;
            mSetProp   = ValueObjectProperty("set", 1,999,  "set struct Values.Usage ->set value ... ", TypeStructObject);
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override {
            if (mSetProp.matchMethod(methodId, args) == 1) {
                size_t count = std::min(args.size(), mDynamicFieldValues.size());

                for(size_t i = 0; i < count; i++) {
                    mDynamicFieldValues[i] = std::make_unique<Value>(args[i]);
                }

                ret =  Value((uint32_t)count);
                return true;
            }

            return ValueObject::onMethodCall(methodId, args, ret);
        }

    };
    // -------------------------------------------------------------------------
    struct ArrayObject : public ValueObject {

        inline static ValueObjectProperty mCountProp, mValueProp, mKeyProp, mTreeProp;

        ArrayObject() : ValueObject(TypeArrayObject) {
            mSupportClone = true;
            mValidateFieldName = false;
        }

        ArrayObject(std::vector<Value>& args) : ValueObject(TypeArrayObject) {
            mSupportClone = true;
            mValidateFieldName = false;

            for(int64_t i = 0; i <  args.size() ; i++) {
                Value v = NumberNullValue;

                uint32_t id = 0;
                std::string fieldName = "";

                if (args[i].isPointer() && (args[i].asPointerObject()->mType == TypeKeyValueObject )) {
                    KeyValueObject* kv = static_cast<KeyValueObject*>(args[i].asPointerObject());
                    id = kv->mKeyId;
                    fieldName = SymbolTable::getName(id);
                    v = kv->mValue;

                } else if (args[i].isString()) {
                    fieldName = args[i].getStringRef();
                    id = SymbolTable::insert(fieldName);
                } else {
                    continue;
                }

                insertUpdateDynamicField(id, v);
            }
        }
        // -------------------------------------------------------------------------
         ValueObject* clone() override {
            ArrayObject* clone = new ArrayObject();
            ValueObject::cloneBase(clone);
            return clone;
        }
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
        inline void PrintTree(int depth = 0) {
            uint32_t keyindex = 0;
            for (uint32_t i = 0; i < mDynamicFieldValues.size(); i++) {
                const std::string key = SymbolTable::getName(SymbolIdByIndex(i, keyindex));
                const Value* value = mDynamicFieldValues[i].get();
                if (value->isPointer() && value->asPointerObject()->mType == TypeArrayObject) {
                    for (int j = 0; j < depth; j++) Tools::printf("│  ");
                    Tools::printf("├──┬ %s\n", key.c_str());
                    static_cast<ArrayObject*>(value->asPointerObject())->PrintTree( depth + 1 );
                } else {
                    for (int j = 0; j < depth; j++) Tools::printf("│  ");
                    if (i == mDynamicFieldValues.size() - 1 )  Tools::printf("└── ");
                    else Tools::printf("├── ");
                    Tools::printf("%s => %s\n", key.c_str(), value->toString().c_str());
                }
            }
        }

       // -------------------------------------------------------------------------
        inline static void RegisterSymbols() {
            static bool mSymbolsLoaded = false;
            if (mSymbolsLoaded) return;
            mCountProp   = ValueObjectProperty("count", 0,0,  "get the count of the array", TypeStructObject);
            mValueProp   = ValueObjectProperty("value", 1,1,  "get the value at index", TypeStructObject);
            mKeyProp     = ValueObjectProperty("key", 1,1,  "get the key at index", TypeStructObject);
            mTreeProp    = ValueObjectProperty("tree", 0,0,  "print a tree of the array", TypeStructObject);
            mSymbolsLoaded = true;
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override {
            if (mCountProp.matchMethod(methodId, args) == 1) {
                ret =  Value((uint32_t)mDynamicFieldValues.size());
                return true;
            } else
            if (mTreeProp.matchMethod(methodId, args) == 1) {
                this->PrintTree();
                ret = Value(1);
                return true;
            } else
            if (mKeyProp.matchMethod(methodId, args)== 1) {
                uint32_t index = 0;
                if (SymbolIdByIndex(args[0].getUInt(), index) ) {
                    ret = Value( SymbolTable::getName(index) );
                } else  {
                    ret =  NumberNullValue;
                }
                return true;
            } else
            if (mValueProp.matchMethod(methodId, args) == 1) {
                Value* valPtr = getDynamicFieldPtrByIndex(args[0].getUInt());
                if (valPtr) ret =  *valPtr;
                else ret =  NumberNullValue;
                return true;
            } else
            return ValueObject::onMethodCall(methodId, args, ret);
        }
        // -------------------------------------------------------------------------
    };
// ============================ RegisterBaseObject ==================================
inline void RegisterBaseObject(Environment& env) {
    static bool registered = false; if (registered) return; registered = true;
    using namespace FunctionMap;

    // ---------------------------------------------------------------------
    // Base Objects
    // ---------------------------------------------------------------------
    RegisterFunction("KeyValue::new", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if(args.size() != 2 || !args[0].isString()) {
            Tools::errorf("Usage KeyValue::new [string key] Value\n");
            return true;
        }

        KeyValueObject* obj = new KeyValueObject(args[0].getStringRef(), args[1]);
        ret = Value(obj);

        return true;
    },"Constructor of KeyValue which can be compined with Struct or Array. Usage KeyValue::new [string key] Value");
    // ---------------------------------------------------------------------
    // Base Objects
    // ---------------------------------------------------------------------

    RegisterFunction("Object::new", [&env](std::vector<Value>& args, Value& ret ) -> bool {
        BaseValueObject* obj = new BaseValueObject();
        ret = Value(obj);
        return true;
    } ,"Constructor of Object. return Object");

    // ---------------------------------------------------------------------
    StructObject::RegisterSymbols();
    RegisterFunction("Struct::new", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if(args.size() == 0) {
            Tools::errorf("Usage Struct::new [string field || KeyValue] [ string field || KeyValue ] ...\n");
            return true;
        }

        for(size_t i = 0; i < args.size(); i++) {
            if (!args[i].isString() && !(args[i].isPointer() && (args[i].asPointerObject()->mType == TypeKeyValueObject ))) {
                Tools::errorf("Usage Struct::new [string field || KeyValue] [ string field || KeyValue ] ...\n");
                return true;
            }
        }
        StructObject* obj = new StructObject(args);
        ret = Value(obj);


        return true;
    },"Constructor of Struct. return a Struct, @params [string fieldname] or [KeyValue ] ....");
    // ---------------------------------------------------------------------
    ArrayObject::RegisterSymbols();
    RegisterFunction("Array::new", [&env](std::vector<Value>& args, Value& ret) -> bool {
        // if(args.size() == 0) {
        //     Tools::errorf("Usage Array::new [string field || KeyValue] [ string field || KeyValue ] ...\n");
        //     return true;
        // }

        for(size_t i = 0; i < args.size(); i++) {
            if (!args[i].isString() && !(args[i].isPointer() && (args[i].asPointerObject()->mType == TypeKeyValueObject ))) {
                Tools::errorf("Usage Array::new [string field || KeyValue] [ string field || KeyValue ] ...\n");
                return true;
            }
        }
        ArrayObject* obj = new ArrayObject(args);
        ret = Value(obj);


        return true;
    },"Constructor of Array. return a Array, @params [string fieldname] or [KeyValue ] ....");
    // ---------------------------------------------------------------------
    // Helper
    // ---------------------------------------------------------------------
    RegisterFunction("obj::isType", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if(args.size() != 2  || !args[1].isNumber()) {
            Tools::errorf("Usage obj::isType var Type\n");
            return true;
        }
        if ( !args[0].isPointer()) {
            ret = Value(0);
            return true;
        }

        bool isType = args[0].asPointerObject()->mType == args[1].asUInt();
        ret = Value(isType);
        return true;
    },"Constructor of Array. return a Array, @params [string fieldname] or [KeyValue ] ....");
}

} //namespace

