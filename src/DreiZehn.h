//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Glue and Module Registry / import
//-----------------------------------------------------------------------------
#pragma once

#include "Environment.h"

#include "functions/CoreFunctions.h"
#include "functions/StringFunctions.h"
#include "functions/MathFunctions.h"
#include "functions/VectorFunctions.h"
#include "functions/DebugFunctions.h"
#include "functions/PointVectorObjectFunctions.h"
#include "functions/FileFunctions.h"


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
                insertUpdateDynamicField(id, Value(0));
                // this->mDynmaicFields[id] = Value(0);
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

void Init(Environment& env) {
    static bool registered = false; if (registered) return; registered = true;

    using namespace FunctionMap;
    ModuleRegistry::Register("Core", RegisterCoreFunctions);
    ModuleRegistry::Register("Math", RegisterMathFunctions);
    ModuleRegistry::Register("String", RegisterStringFunctions);
    ModuleRegistry::Register("Vector", RegisterVectorFunctions);
    ModuleRegistry::Register("Debug", RegisterDebugFunctions);
    ModuleRegistry::Register("PointVector", RegisterPointVectorObjectFunctions);
    ModuleRegistry::Register("File", RegisterFileFunctions);


    // ---------------------------------------------------------------------
    // also push some CORE Constants here:
    RegisterConstants("true", Value(1));
    RegisterConstants("false", Value(0));

    // ---------------------------------------------------------------------
    // Base Objects
    // ---------------------------------------------------------------------

    RegisterFunction("Object::new", [&env](std::vector<Value>& args, Value& ret) -> bool {
        BaseValueObject* obj = new BaseValueObject();
        ret = Value(obj);
        return true;
    });

    // ---------------------------------------------------------------------
    StructObject::RegisterSymbols();
    RegisterFunction("Struct::new", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if(args.size() == 0) {
            Tools::errorf("Usage struct [string field] [string field] ...\n");
            return true;
        }

        for(size_t i = 0; i < args.size(); i++) {
            if (!args[i].isStringId() ) {
                Tools::errorf("Usage struct [string field] [string field] ...\n");
                return true;
            }
        }
        StructObject* obj = new StructObject(args);
        ret = Value(obj);


        return true;
    });

    // ---------------------------------------------------------------------
    // -------- print --------------
    // ---------------------------------------------------------------------
    RegisterFunction("print", [](std::vector<Value>& args, Value& ret) -> bool {
        std::string line = "";
        for ( auto& value : args) {
            line = line + value.toString() + " ";
        }
        Tools::printf("%s\n", line.c_str());
        return true;
    });
    RegisterFunction("printraw", [](std::vector<Value>& args, Value& ret) -> bool {
        for ( auto& value : args) {
            value.print();
        }
        return true;
    });

    RegisterFunction("error", [](std::vector<Value>& args, Value& ret) -> bool {
        std::string line = "";
        for ( auto& value : args) {
            line = line + value.toString() + " ";
        }
        Tools::errorf("%s\n", line.c_str());
        return true;
    });
    // ---------------------------------------------------------------------
    // run
    // ---------------------------------------------------------------------
    RegisterFunction("run", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if (args.size() != 1) {
            Tools::errorf("file name requires for run, Usage: run filename\n");
            ret = Value(0);
            return true;
        }

        if (args[0].isStringId() ) {
            Tools::printf("Loading Script: %s\n", args[0].getStringRef().c_str());
            bool success = RunScriptFile(args[0].getStringRef(), env);
            ret = Value(success ? 1 : 0);
            return success;
        }


        Tools::errorf("file name requires for run\n");
        ret = Value(0);
        return true;
    });

    // ---------------------------------------------------------------------
    // list modules
    // ---------------------------------------------------------------------
    RegisterFunction("modules", [&env](std::vector<Value>& args, Value& ret) -> bool {
        ModuleRegistry::Print();
        return true;
    });
    // ---------------------------------------------------------------------
    // int
    // ---------------------------------------------------------------------
    RegisterFunction("int::cast", [](std::vector<Value>& args, Value& ret) -> bool {
        if (args.size() != 1) {
            Tools::errorf("Usage: int::cast value\n");
            return false;
        }
        if (args[0].isInt()) ret =  args[0];
        else
            if (args[0].isDouble()) ret = Value(args[0].getInt());
            else
                if (args[0].isStringId()) ret = Value( std::atoi( args[0].getStringRef().c_str()));
                else
                    Tools::PrintRuntimeError("Cant cast pointer to integer!\n");

        return true;
    });


    // ---------------------------------------------------------------------
    // float
    // ---------------------------------------------------------------------
    RegisterFunction("float::cast", [](std::vector<Value>& args, Value& ret) -> bool {
        if (args.size() != 1) {
            Tools::errorf("Usage: float::cast value\n");
            return false;
        }
        if (args[0].isDouble()) ret =  args[0];
        else
            if (args[0].isInt()) ret = Value(args[0].getDouble());
            else
                if (args[0].isStringId()) ret = Value( std::atof( args[0].getStringRef().c_str()));
                else
                    Tools::PrintRuntimeError("Cant cast pointer to float!\n");

        return true;
    });

}

} //namespace

