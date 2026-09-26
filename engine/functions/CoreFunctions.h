//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Core Commands
//-----------------------------------------------------------------------------
#pragma once
#include "core/FunctionMap.h"
#include "ScriptLoader.h"

namespace DreiZehn {



    void RegisterCoreFunctions( Environment& env) {
        using namespace FunctionMap;
        // ---------------------------------------------------------------------
        // also push some CORE Constants here:
        RegisterConstants("true", Value(1));
        RegisterConstants("false", Value(0));
        // ---------------------------------------------------------------------
        // -------- print --------------
        RegisterFunction("print", [](std::vector<Value>& args, Value& ret) -> bool {
            for ( auto& value : args) {
                value.print();
            }
            Tools::printf("\n");
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("run", [&env](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() < 1) {
                Tools::errorf("file name requires for run\n");
                return false;
            }

            if (args[0].isPointer()) {
                auto* obj = static_cast<ValueObject*>(args[0].asPointer());
                if (obj->mType == ValueObjectType::String) {
                    auto* strObj = static_cast<StringValueObject*>(obj);

                    Tools::printf("Loading Script: %s\n", strObj->mValue.c_str());
                    bool success = RunScriptFile(strObj->mValue, env);

                    ret = Value(success ? 1 : 0);
                    return success;
                }
            }

            Tools::errorf("file name requires for run\n");
            return false;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("concat", [&env](std::vector<Value>& args, Value& ret) -> bool {
            std::string resultStr = "";

            for (const auto& val : args) {
                if (val.isInt()) {
                    resultStr += std::to_string(val.asInt());
                }
                else if (val.isDouble()) {
                    std::string dStr = std::to_string(val.asDouble());
                    dStr.erase(dStr.find_last_not_of('0') + 1, std::string::npos);
                    if (dStr.back() == '.') dStr.pop_back();
                    resultStr += dStr;
                }
                else if (val.isPointer()) {
                    auto* obj = static_cast<ValueObject*>(val.asPointer());
                    if (obj && obj->mType == ValueObjectType::String) {
                        auto* strObj = static_cast<StringValueObject*>(obj);
                        resultStr += strObj->mValue;
                    }
                }
            }

            auto* newStrObj = new StringValueObject(resultStr);
            env.getVariableFrame()->addToGarbageCollection(newStrObj);
            ret = Value(newStrObj);
            return true;
        });

        // ---------------------------------------------------------------------
        // Garbage collection
        // ---------------------------------------------------------------------
        RegisterFunction("core::gc", [&env](std::vector<Value>& args, Value& ret) -> bool {
            if (gCurrentFrame) {
                gCurrentFrame->doGarbageCollection(false);
                return true;
            }
            return false;
        });
        RegisterFunction("core::printgc", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("---------------- Garbage Collection ------------------\n");
            if (gCurrentFrame) gCurrentFrame->listGarbageObjects();
            Tools::printf("------------------------------------------------------\n");
            return true;
        });

        // ---------------------------------------------------------------------
        // Help
        // ---------------------------------------------------------------------
        RegisterFunction("help", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("---------------- DreiZehn Help ------------------\n");
            Tools::printf(
                "help::syntax  => print syntax informations.\n"
                "help::fn      => print the available Function\n"
                "help::objects => print the available Object-Types\n"
            );
            return true;
        });
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
        });

        RegisterFunction("help::fn", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("  --- Functions [%zu] --- \n", RegisteredFunctions.size());
            for (const auto& [key, value] : RegisteredFunctions) {
                Tools::printf("  - %s \n", SymbolTable::getName(key).c_str());
            }
            return true;

            return true;
        });

        RegisterFunction("help::objects", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("---------------- Object Types ------------------\n");


            if (args.size() > 0) {

                int id = 0;
                bool found = false;
                for (id = 0; id <= gLastValueObjectType; id++) {
                    if (args[0].getString() == gUserObjectTypes[id].mName){
                        found = true;
                        break;
                    }
                }

                if (found) {
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
                for (int i = 0; i <= gLastValueObjectType; i++) {
                    Tools::printf("%d: %s\n",i, gUserObjectTypes[i].mName.c_str());
                }
                Tools::printf("\n** To get fields and methods of an object use: help::objects \"String\"\n");
            }
            return true;
        });
        // ---------------------------------------------------------------------
    } //RegisterCoreFunctions

} //namespace
