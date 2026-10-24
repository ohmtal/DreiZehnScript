//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Glue and Module Registry / import
//-----------------------------------------------------------------------------
#include "DreiZehn.h"
#include "Environment.h"

#include "core/BaseObjects.h"
#include "functions/VectorFunctions.h"
#include "functions/PointVectorObjectFunctions.h"
#include "functions/FileFunctions.h"
#include "ScriptLoader.h"


namespace DreiZehn {


void Init(Environment& env, int argc, char* argv[]) {
    static bool registered = false; if (registered) return; registered = true;

    RegisterBaseObject(env);


    char buffer[256];
    ArrayObject* obj = new ArrayObject();
    FunctionMap::RegisterConstants("sys::args", Value(obj));
    for (int i = 0; i < argc; i++) {
        snprintf(buffer, sizeof(buffer), "%d" , i);
        obj->insertUpdateDynamicField(SymbolTable::insert(buffer), Value(std::string(argv[i])));
    }


    using namespace FunctionMap;


    ModuleRegistry::Register("Core", RegisterCoreFunctions);
    ModuleRegistry::Register("Math", RegisterMathFunctions);
    ModuleRegistry::Register("String", RegisterStringFunctions);
    ModuleRegistry::Register("Vector", RegisterVectorFunctions);
    ModuleRegistry::Register("Debug", RegisterDebugFunctions);
    ModuleRegistry::Register("PointVector", RegisterPointVectorObjectFunctions);
    ModuleRegistry::Register("File", RegisterFileFunctions);
    ModuleRegistry::Register("Sys", RegisterSysFunctions);


    // ---------------------------------------------------------------------
    // also push some CORE Constants here:
    RegisterConstants("true", Value(1));
    RegisterConstants("false", NumberNullValue);

    // ---------------------------------------------------------------------

    RegisterConstants("type::float",    (int32_t)ValueType::Double);
    RegisterConstants("type::integer",  (int32_t)ValueType::Int);
    RegisterConstants("type::object",  (int32_t)ValueType::Pointer);
    RegisterConstants("type::pointer",  (int32_t)ValueType::ValuePointer);
    RegisterConstants("type::string",  (int32_t)ValueType::StringId);

    RegisterFunction("getType", [&env](std::vector<Value>& args, Value& ret ) -> bool {
        if (args.size() != 1) {
            Tools::errorf("Usage: getType value. return the type id of the expression. (Example: Type::float)");
            ret =  Value(-1);
            return true;
        }
        ret = Value((int) args[0].getType());
        return true;
    } ,"Usage: getType value. return the type id of the expression. (Example: Type::float)");



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
    }, "Print a line to console. @param value value value ...");

    RegisterFunction("printraw", [](std::vector<Value>& args, Value& ret) -> bool {
        for ( auto& value : args) {
            value.print();
        }
        return true;
    }, "Print a line without linefeed to console. @param value value value ...");

    RegisterFunction("error", [](std::vector<Value>& args, Value& ret) -> bool {
        std::string line = "";
        for ( auto& value : args) {
            line = line + value.toString() + " ";
        }
        Tools::errorf("%s\n", line.c_str());
        return true;
    }, "Print a error to console. @param value value value ...");
    // ---------------------------------------------------------------------
    // run
    // ---------------------------------------------------------------------
    RegisterFunction("run", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if (Globals::ByteCodeMode) {
            Tools::errorf("Sorry cant run injected bytecode at the moment!");
            return false;
        }

        if (args.size() != 1) {
            Tools::errorf("file name requires for run, Usage: run filename\n");
            ret = NumberNullValue;
            return true;
        }

        if (args[0].isString() ) {
            Tools::printf("Loading Script: %s\n", args[0].getStringRef().c_str());
            bool success = false;
            success = RunScriptFile(args[0].getStringRef(), env, false);

            ret = Value(success ? 1 : 0);
            return success;
        }


        Tools::errorf("file name requires for run\n");
        ret = NumberNullValue;
        return true;
    }, "Run a script file. @param filename");

    RegisterFunction("runbc", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if (Globals::ByteCodeRunning) {
            Tools::errorf("Sorry cant run recursive injected bytecode at the moment!");
            return false;
        }

        if (args.size() != 1) {
            Tools::errorf("file name requires for run, Usage: run filename\n");
            ret = NumberNullValue;
            return true;
        }

        if (args[0].isString() ) {
            Tools::printf("Loading Script: %s\n", args[0].getStringRef().c_str());
            bool success = false;
            success = RunScriptFile(args[0].getStringRef(), env, true);

            ret = Value(success ? 1 : 0);
            return success;
        }


        Tools::errorf("file name requires for run\n");
        ret = NumberNullValue;
        return true;
    }, "Compile Script to ByteCode and run. @param filename");

    // ---------------------------------------------------------------------
    // list modules
    // ---------------------------------------------------------------------
    RegisterFunction("modules", [&env](std::vector<Value>& args, Value& ret) -> bool {
        ModuleRegistry::Print();
        return true;
    }, "List all available modules which can be used by import");
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
                if (args[0].isString()) ret = Value( std::atoi( args[0].getStringRef().c_str()));
                else
                    Tools::PrintRuntimeError("Cant cast pointer to integer!\n");

        return true;
    }, "cast any type to integer.");


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
                if (args[0].isString()) ret = Value( std::atof( args[0].getStringRef().c_str()));
                else
                    Tools::PrintRuntimeError("Cant cast pointer to float!\n");

        return true;
    }, "cast any type to float");

}

} //namespace

