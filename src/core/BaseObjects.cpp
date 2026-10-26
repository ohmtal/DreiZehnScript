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
#include <algorithm>
#include "core/Value.h"
#include "core/ValueObject.h"
#include "core/FunctionMap.h"
#include "VariableFrame.h"
#include "BaseObjects.h"
#include <syntax13/Syntax13.h>
namespace DreiZehn {

    // ============================ KeyValueObject ==================================
    std::string KeyValueObject::toString() {
        return std::string(SymbolTable::getName( mKeyId)  + " => " + mValue.toString());
    }

    KeyValueObject::KeyValueObject(const std::string& key, const Value& value): ValueObject(TypeKeyValueObject){
        mSupportClone = false;
        mAutoCreateDynamicFields = false;
        mKeyId = SymbolTable::insert( key );
        mValue = value;
    }

    KeyValueObject::KeyValueObject() : ValueObject(TypeKeyValueObject){
        mSupportClone = false;
        mAutoCreateDynamicFields = false;
    }

    void KeyValueObject::Dump(){
        ValueObject::Dump();
        Tools::printf("%s => %s\n", SymbolTable::getName( mKeyId).c_str(), mValue.toString().c_str());
    }

     void KeyValueObject::RegisterSymbols(){
         static bool mSymbolsLoaded = false;
         if (mSymbolsLoaded) return;
         // // mKeyProp    = ValueObjectProperty("key", 0,0,  "get the first (key)", TypeStructObject);
         // // mValueProp  = ValueObjectProperty("second", 0,0,  "get the second (value)", TypeStructObject);
         mSymbolsLoaded = true;

    }
    // // bool KeyValueObject::onMethodCall(uint32_t methodId, std::span<Value> args, Value& ret) {
    // //     if (mFirstProp.matchMethod(methodId, args) == 1) {
    // //         ret = Value(SymbolTable::getName(mKeyId));
    // //         return true;
    // //     }
    // //     else
    // //     if (mFirstProp.matchMethod(methodId, args) == 1) {
    // //         ret = Value(SymbolTable::getName(mKeyId));
    // //         return true;
    // //     }
    // //     return ValueObject::onMethodCall(methodId, args, ret);
    // // }
     // ============================ StructObject ==================================
    StructObject::StructObject(std::span<Value> args) : ValueObject(TypeStructObject){
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

            //FIXME Syntax13
            if (!Syntax13::isValidVariableName(fieldName)) {
                Tools::warnf("Field `%s` is not a valid variable name\n", fieldName.c_str());
                continue;
            }

            insertUpdateDynamicField(id, v);
        }
    }

    void StructObject::RegisterSymbols(){
        static bool mSymbolsLoaded = false;
        if (mSymbolsLoaded) return;
        mSetProp   = ValueObjectProperty("set", 1,999,  "set struct Values.Usage ->set value ... ", TypeStructObject);
        mSymbolsLoaded = true;
    }

    bool StructObject::onMethodCall(uint32_t methodId, std::span<Value> args, Value& ret) {
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

    // ============================ ArrayObject ==================================
    ArrayObject::ArrayObject(std::span<Value> args)  : ValueObject(TypeArrayObject){
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

    DreiZehn::ValueObject* ArrayObject::clone(){
        ArrayObject* clone = new ArrayObject();
        ValueObject::cloneBase(clone);
        return clone;
    }

    void ArrayObject::PrintTree(int depth){
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

    void ArrayObject::RegisterSymbols(){
        static bool mSymbolsLoaded = false;
        if (mSymbolsLoaded) return;
        mCountProp   = ValueObjectProperty("count", 0,0,  "get the count of the array", TypeStructObject);
        mValueProp   = ValueObjectProperty("value", 1,1,  "get the value at index", TypeStructObject);
        mKeyProp     = ValueObjectProperty("key", 1,1,  "get the key at index", TypeStructObject);
        mTreeProp    = ValueObjectProperty("tree", 0,0,  "print a tree of the array", TypeStructObject);
        mSymbolsLoaded = true;
    }

    bool ArrayObject::onMethodCall(uint32_t methodId, std::span<Value> args, Value& ret){
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

    // ============================ RegisterBaseObject ==================================
    void RegisterBaseObject(DreiZehn::Environment& env) {
        static bool registered = false; if (registered) return; registered = true;
        using namespace FunctionMap;

        // ---------------------------------------------------------------------
        // Base Objects
        // ---------------------------------------------------------------------
        KeyValueObject::RegisterSymbols();
        RegisterFunction("KeyValue::new", [&env](std::span<Value> args, Value& ret) -> bool {
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

        RegisterFunction("Object::new", [&env](std::span<Value> args, Value& ret ) -> bool {
            BaseValueObject* obj = new BaseValueObject();
            ret = Value(obj);
            return true;
        } ,"Constructor of Object. return Object");

        // ---------------------------------------------------------------------
        StructObject::RegisterSymbols();
        RegisterFunction("Struct::new", [&env](std::span<Value> args, Value& ret) -> bool {
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
        RegisterFunction("Array::new", [&env](std::span<Value> args, Value& ret) -> bool {
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
        RegisterFunction("obj::isType", [&env](std::span<Value> args, Value& ret) -> bool {
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
} // namespace
