//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Core + Help Commands + sys commands
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <algorithm> // std::sort
#include <cstdlib> // std::getenv

#include "toolbox/Tools.h"
#include "core/FunctionMap.h"
#include "ScriptLoader.h"
#include "core/GarbageCollection.h"



// for FetchEnv
#if defined(_WIN32)
#define environ _environ
#else
extern char** environ;
#endif

namespace DreiZehn {



    inline std::function<bool()> OnBreath = nullptr;

    // =============================================================================
    // --- RegisterCoreFunctions ---
    // =============================================================================
    void RegisterCoreFunctions( Environment& env) {
        static bool registered = false; if (registered) return; registered = true;
        using namespace FunctionMap;


        // ---------------------------------------------------------------------
        // CORE
        // ---------------------------------------------------------------------
        RegisterFunction("core::eval", [&env](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isStringId()) {
                Tools::errorf("Usage: core::eval codeled\n");
                ret = NumberNullValue;
                return true;
            }
            std::stringstream stream(args[0].getStringRef());
            ret = RunScriptStream(stream, env);
            return true;
        }, "run a script embedded in a string");
        // ---------------------------------------------------------------------
        RegisterFunction("core::isFunction", [&env](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isString()) {
                Tools::errorf("Usage: core::isFunction functionname\n");
                ret = NumberNullValue;
                return true;
            }
            uint32_t nameTokenSymbolId = SymbolTable::insert(args[0].getStringRef());
            ret = Value(FunctionMap::IsFunction(nameTokenSymbolId));

            return true;
        }, "return if a function exists or not. useful to check a include failed");
        // ---------------------------------------------------------------------
        RegisterFunction("core::getTypeName", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: core::getTypeName value");
                return false;
            }
            ret = Value(args[0].getTypeName());
            return true;
        }, "get the value type name (see also getType for integer result )");
        // ---------------------------------------------------------------------
        // Let the console Breath ... return a boolean can also be used to cancel
        // a main loop in script
        // ---------------------------------------------------------------------
        RegisterFunction("core::breath", [](std::vector<Value>& args, Value& ret) -> bool {
            if (OnBreath) {
                ret = Value(OnBreath());
                return true;
            }
            return false;
        }, "let the programm breath - usally used for console.");
        // ---------------------------------------------------------------------
        // sleep ms
        // ---------------------------------------------------------------------
        RegisterFunction("core::sleep", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) {
                 Tools::errorf("Usage: core::sleep ms\n");
                 return false;
            }
            Tools::sleep(args[0].asUInt());
            return true;
        }, "sleep for x ms");

        // ---------------------------------------------------------------------
        // time
        // ---------------------------------------------------------------------
        RegisterFunction("core::time", [](std::vector<Value>& args, Value& ret) -> bool {
            ret = Value(Tools::time());
            return true;
        }, "get the unix timestamp");

        RegisterFunction("core::date", [](std::vector<Value>& args, Value& ret) -> bool {
            if (!args.empty() && !args[0].isString()) {
                Tools::errorf("Usage core::date [strng format] [double time ]\n");
                return true;
            }
            std::string fmt = (args.size() > 0) ? args[0].getStringRef() : "%Y-%m-%d %H:%M:%S";
            double ts = (args.size() > 1) ? args[1].getDouble() : -1.0;

            ret = Value(Tools::date(fmt, ts));
            return true;
        }, "Usage core::date format [double time]."
            "\nExample: core::date \"%Y-%m-%d %H:%M:%S\"");

        RegisterFunction("core::dateutc", [](std::vector<Value>& args, Value& ret) -> bool {
            if (!args.empty() && !args[0].isString()) {
                Tools::errorf("Usage core::dateutc [string format] [double time ]\n");
                return true;
            }
            std::string fmt = (args.size() > 0) ? args[0].getStringRef() : "%Y-%m-%d %H:%M:%S UTC";
            double ts = (args.size() > 1) ? args[1].getDouble() : -1.0;

            ret = Value(Tools::date(fmt, ts, true));
            return true;
        }, "Usage core::dateutc format [double time ]"
           "\nExample: core::dateutc \"%Y-%m-%d %H:%M:%S\"");
        // ---------------------------------------------------------------------
        // Garbage collection
        // ---------------------------------------------------------------------
        RegisterFunction("core::gc", [](std::vector<Value>& args, Value& ret) -> bool {
            GarbageCollection::run();
            return true;
        }, "Run the GarbageCollection manually");

        RegisterFunction("core::printgc", [](std::vector<Value>& args, Value& ret) -> bool {
            GarbageCollection::print();
            return true;
        }, "Print the current GarbageCollection objects");
        // ---------------------------------------------------------------------
        RegisterFunction("core::printGlob", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("---------------- Globals ------------------\n");
            for (auto & id: Globals::GlobalVarList) {
                Value* valPtr = Globals::MasterVariableFrame->getVariablePtr(id);
                Tools::printf("[%zu] %s = %s\n "
                    , id, SymbolTable::getName(id).c_str()
                    , valPtr ? valPtr->toString().c_str() : "NOT FOUND!!!"
                );
            }
            Tools::printf("------------------------------------------------------\n");
            return true;
        }, "Print all global variables.");

        // ---------------------------------------------------------------------
        //
        // ---------------------------------------------------------------------
        RegisterFunction("core::getEnv", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() < 1 || !args[0].isString()) {
                Tools::printf("Usage example: core::getenv \"PATH\"");
                ret = EmptyStringValue;
                return true;
            }
            const char* env_p = std::getenv(args[0].getStringRef().c_str());
            if (env_p) {
                ret = Value(std::string(env_p));
            } else {
                ret = EmptyStringValue;
            }

            return true;
        }, "get a Environment variable. param variable name as string");

        RegisterFunction("core::fetchEnv", [](std::vector<Value>& args, Value& ret) -> bool {
            BaseValueObject* resObj = new BaseValueObject();
            ret = Value(resObj);
            for (char** env = environ; *env != nullptr; ++env) {
                std::string env_entry(*env);
                size_t pos = env_entry.find('=');
                if (pos != std::string::npos) {
                    std::string key = env_entry.substr(0, pos);
                    std::string value = env_entry.substr(pos + 1);

                    if (!resObj->addDynamicField(key,Value(value))) {
                        Tools::warnf("Failed to set key:%s, Value:%s", key.c_str(), value.c_str());
                    }
                }
            }

            return true;
        }, "get all Environment variables and add them as fields to a Object");

        // ---------------------------------------------------------------------
        // Help
        // ---------------------------------------------------------------------
        RegisterFunction("help", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("---------------- DreiZehn Help ------------------\n");
            Tools::printf(
                "help::syntax    => print syntax informations.\n"
                "help::fn        => print the available Function\n"
                "help::objects   => print the available Object-Types\n"
                "help::const     => print the registered Constants\n"
            );
            return true;
        }, "print the help overview");
        RegisterFunction("help::syntax", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf(
                "Basics\n"
                "- parameters for functions can be simple written word by word.\n"
                "    Exception: when you call a parameter you have to add parantheses\n"
                "    Example: example `print (myobj->getX) (myobj->getX)`\n"
                "- Static functions like Array::new are separated by '::'\n"
                "- Tool functions first character should by lowercase: core::gc\n"
                "- Objects first character should by uppercase: Array::new\n"
                "- Object fields can be accessed by Dot '.' Separator: myVector.x = 3\n"
                "- Object methods can be called with the Arrow '->' like Array->print\n"
                "- optional ';' is a line break for the code so it's the same as your write it in a new line.\n"
                "Syntax examples:\n"
                "Operators: '+' '-' '*' '/' '|' '||' '&' '&&' '>>' '<<'\n"
                "Operators: '>' '<' '==' '!='\n"
                "Inline Operators: '++' '--'\n"
                "Assigment: '=' '+=' '-=' '*=' '/'\n"
                " ------------- \n"
                "Iterators:\n"
                "for i 1 10; print i; end ==> print i from 1 to inclusive 10 \n"
                "forRange i 10; print i; end => print i from 0 to 9\n"
                "i = 0;while i < 10; print i; i++; end => print i from 0 to 9\n"
                "Note: `break` break the current loop.\n"
                " ------------- \n"
                "Condition:\n"
                "i = 4; if i == 4; print \"It's four\"; else  print \"It's not four\";end\n"
                " ------------- \n"
                "Funtion Declaration\n"
                "fn myFunction param1 param2; print param1 param2; end; myFunction 21 22\n"
                "\n"

            ) ;
            return true;
        }, "basic Syntax informations");


        // ---------------------------------------------------------------------
        RegisterFunction("help::fn", [](std::vector<Value>& args, Value& ret) -> bool {
            bool doFilter = false;
            std::string filter;
            if (!args.empty() && args[0].isStringId()) {
                filter = args[0].getString();
                if (!filter.empty()) doFilter = true;
            }

            struct NamedFunction {
                uint32_t key;
                std::string name;
            };
            std::vector<NamedFunction> filteredFunctions;

            // std::vector<std::string> nativeNames;
            for (const auto& [key, value] : RegisteredFunctions) {
                std::string name = SymbolTable::getName(key);
                if (!doFilter || name.find(filter) != std::string::npos) {
                    // nativeNames.push_back(name);
                    filteredFunctions.push_back({key,name});
                }
            }

            std::vector<std::string> scriptNames;
            for (const auto& [key, value] : RegisteredScriptFunctions) {
                std::string name = SymbolTable::getName(key);
                if (!doFilter || name.find(filter) != std::string::npos) {
                    scriptNames.push_back(name);
                }
            }

            // std::sort(nativeNames.begin(), nativeNames.end());
            std::sort(filteredFunctions.begin(), filteredFunctions.end(),
                      [](const NamedFunction& a, const NamedFunction& b) {
                          return a.name < b.name;
                      }
            );

            std::sort(scriptNames.begin(), scriptNames.end());

            Tools::printf("  --- Functions [%zu/%zu] --- \n", filteredFunctions.size(), RegisteredFunctions.size());
            for (const auto& func : filteredFunctions) {
                uint32_t strId =  getFunctionDescStrId(func.key);
                const std::string desc = (strId > 0 ) ? StringTable::get(strId) : "";
                Tools::printf("  - %-30s %s\n", func.name.c_str(), desc.c_str());
            }

            if (scriptNames.size() > 0) {
                Tools::printf("  --- Script Functions [%zu/%zu] --- \n", scriptNames.size(), RegisteredScriptFunctions.size());
                for (const std::string& name : scriptNames) {
                    Tools::printf("  - %s \n", name.c_str());
                }
            }

            return true;
        }, "list the registered functions. you can call it with a filter like `help::fn \"str\"`");

        // ---------------------------------------------------------------------

        RegisterFunction("help::objects", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("---------------- Object Types ------------------\n");


            if (args.size() > 0) {
                int id = 0;

                if (args[0].isStringId()) {
                    for (id = 1; id <= gLastValueObjectType; id++) {
                        if (args[0].getString() == gUserObjectTypes[id].mName){
                            break;
                        }
                    }
                } else {
                    id = args[0].getInt();
                }

                if (id > 0 && id < gLastValueObjectType) {
                    Tools::printf("%d: %s\n",id, gUserObjectTypes[id].mName.c_str());
                    for (int i = 0; i < gUserObjectTypes[id].mProperties.size(); i++) {
                        Tools::printf(" %s%s  :: %s\n",
                                      gUserObjectTypes[id].mProperties[i].mIsMethod ? "->" : ".",
                                      gUserObjectTypes[id].mProperties[i].mName.c_str(),
                                      gUserObjectTypes[id].mProperties[i].mHelp.c_str()

                        );
                    }

                }
            } else {
                for (int i = 1; i <= gLastValueObjectType; i++) {
                    Tools::printf("%d: %s\n",i, gUserObjectTypes[i].mName.c_str());
                }
                Tools::printf("\n** To get fields and methods of an object use: help::objects \"String\"\n");
            }
            return true;
        }, "list the registered object types. can be called with parameter id of the object type");
        // ---------------------------------------------------------------------

        RegisterFunction("help::const", [](std::vector<Value>& args, Value& ret) -> bool {
            bool doFilter = false;
            std::string filter;
            if (!args.empty() && args[0].isStringId()) {
                filter = args[0].getString();
                if (!filter.empty()) doFilter = true;
            }


            std::vector<std::string> constNames;
            for (const auto& [key, value] : RegisteredConstants) {
                std::string name = SymbolTable::getName(key);
                if (!doFilter || name.find(filter) != std::string::npos) {
                    constNames.push_back(name);
                }
            }


            std::sort(constNames.begin(), constNames.end());

            Tools::printf("  --- Constants [%zu/%zu] --- \n", constNames.size(), RegisteredConstants.size());
            for (const std::string& name : constNames) {
                // lazy recover
                const uint32_t id = SymbolTable::insert(name);
                Value value = RegisteredConstants[id];
                Tools::printf(" %20s = %s\n", name.c_str(), value.toString().c_str());
            }



            return true;
        }, "list all registerd constants can be called with a filter.");

        // ---------------------------------------------------------------------
    } //RegisterCoreFunctions

} //namespace
