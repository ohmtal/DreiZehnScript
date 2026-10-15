//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Base Objects
//-----------------------------------------------------------------------------
#pragma once

#include <algorithm>
#include "core/Value.h"
#include "core/ValueObject.h"
#include "core/FunctionMap.h"
#include "VariableFrame.h"


namespace DreiZehn {



    const int TypeBaseObject =  RegisterUserObjectType("Object");
    const int TypeStructObject =  RegisterUserObjectType("Struct");
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
        std::vector<uint32_t> mFieldOrder;

        inline static ValueObjectProperty mSetProp;

        StructObject() : ValueObject(TypeStructObject) {
            mSupportClone = true;
            mAutoCreateDynamicFields = false;
        }

        StructObject(std::vector<Value>& args) : ValueObject(TypeStructObject) {
            mSupportClone = true;
            mAutoCreateDynamicFields = false;

            for(size_t i = 0; i < args.size(); i++) {

                const std::string &fieldName = args[i].getStringRef();
                if (!isValidVariableName(fieldName)) continue;


                uint32_t id = SymbolTable::insert(fieldName);
                this->mFieldOrder.push_back(id);
                insertUpdateDynamicField(id, NumberNullValue);
            }
        }
        // -------------------------------------------------------------------------

        virtual ValueObject* clone() override {
            StructObject* clone = new StructObject();
            ValueObject::cloneBase(clone);
            clone->mFieldOrder = this->mFieldOrder;

            return clone;
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
                size_t count = std::min(args.size(), mFieldOrder.size());

                for(size_t i = 0; i < count; i++) {
                    insertUpdateDynamicField(mFieldOrder[i], args[i]);
                    // mDynmaicFields[mFieldOrder[i]] = args[i];
                }

                ret =  Value((uint32_t)count);
                return true;
            }

            return ValueObject::onMethodCall(methodId, args, ret);
        }

    };

void RegisterBaseObject(Environment& env) {
    static bool registered = false; if (registered) return; registered = true;
    using namespace FunctionMap;
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
            Tools::errorf("Usage struct [string field] [string field] ...\n");
            return true;
        }

        for(size_t i = 0; i < args.size(); i++) {
            if (!args[i].isStringId() ) {
                Tools::errorf("Usage Struct::new [string field] [string field] ...\n");
                return true;
            }
        }
        StructObject* obj = new StructObject(args);
        ret = Value(obj);


        return true;
    },"Constructor of Struct. return a Struct, @params [string field] [string field] ....");


}

} //namespace

