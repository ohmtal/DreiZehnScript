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

    const int TypeKeyValueObject =  RegisterUserObjectType("KeyValue");
    const int TypeBaseObject =  RegisterUserObjectType("Object");
    const int TypeStructObject =  RegisterUserObjectType("Struct");
    const int TypeArrayObject =  RegisterUserObjectType("Array");

    // -------------------------------------------------------------------------
    struct KeyValueObject : public ValueObject {
        std::string mKey = "";
        Value mValue = NumberNullValue;
        KeyValueObject() : ValueObject(TypeKeyValueObject) {
            mSupportClone = false;
            mAutoCreateDynamicFields = false;
        }
        KeyValueObject(const std::string& key, const Value& value) : ValueObject(TypeKeyValueObject) {
            mSupportClone = false;
            mAutoCreateDynamicFields = false;
            mKey = key;
            mValue = value;
        }
        std::string  toString() override{
            return std::string(mKey + " => " + mValue.toString());
        }
        // -------------------------------------------------------------------------
        void Dump() override {
            ValueObject::Dump();
            Tools::printf("%s => %s\n", mKey.c_str(), mValue.toString().c_str());
        }
    };
    // -------------------------------------------------------------------------
    struct BaseValueObject : public ValueObject {
        BaseValueObject() : ValueObject(TypeBaseObject) {
            mSupportClone = true;

        }
        virtual ValueObject* clone() override {
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

            for(int64_t i = args.size() - 1; i >= 0;  i--) {
                Value v = NumberNullValue;

                std::string fieldName = "";

                if (args[i].isPointer() && (args[i].asPointerObject()->mType == TypeKeyValueObject )) {
                    KeyValueObject* kv = static_cast<KeyValueObject*>(args[i].asPointerObject());
                    fieldName = kv->mKey;
                    v = kv->mValue;

                } else if (args[i].isString()) {
                    fieldName = args[i].getStringRef();
                } else {
                    continue;
                }

                if (!isValidVariableName(fieldName)) {
                    Tools::warnf("Field `%s` is not a valid variable name\n", fieldName.c_str());
                    continue;
                }

                uint32_t id = SymbolTable::insert(fieldName);
                insertUpdateDynamicField(id, v);
            }
        }
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
            if (mSetProp.matchMethod(methodId, args)) {
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
            if (!args[i].isStringId() && !(args[i].isPointer() && (args[i].asPointerObject()->mType == TypeKeyValueObject ))) {
                Tools::errorf("Usage Struct::new [string field || KeyValue] [ string field || KeyValue ] ...\n");
                return true;
            }
        }
        StructObject* obj = new StructObject(args);
        ret = Value(obj);


        return true;
    },"Constructor of Struct. return a Struct, @params [string fieldname] or [KeyValue ] ....");


}

} //namespace

