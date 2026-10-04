//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Core Commands
//-----------------------------------------------------------------------------
#pragma once
#include <algorithm> // std::sort
#include "core/FunctionMap.h"
#include "ScriptLoader.h"

namespace DreiZehn {

    inline std::function<bool()> OnBreath = nullptr;


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
    struct StructObject : public ValueObject {
        std::vector<uint32_t> mFieldOrder;
        StructObject() : ValueObject(TypeStructObject) {
            mSupportClone = true;
        }
        virtual ValueObject* clone() override {
            StructObject* clone = new StructObject();
            ValueObject::cloneBase(clone);
            clone->mFieldOrder = this->mFieldOrder;

            return clone;
        }

        //TODO ASSIGN {1,2,3}
        //TODO OVERRIDE SETTER/GETPTR TO DENY NEW DYNAMIC FIELDS !
    };
    // =============================================================================
    // --- RegisterCoreFunctions ---
    // =============================================================================



    void RegisterCoreFunctions( Environment& env) {
        using namespace FunctionMap;

        // ---------------------------------------------------------------------
        // also push some CORE Constants here:
        RegisterConstants("true", Value(1));
        RegisterConstants("false", Value(0));
        // ---------------------------------------------------------------------

        RegisterFunction("Object::new", [&env](std::vector<Value>& args, Value& ret) -> bool {
            BaseValueObject* obj = new BaseValueObject();
            ret = Value(obj);
            return true;
        });
        RegisterFunction("struct", [&env](std::vector<Value>& args, Value& ret) -> bool {
            for(size_t i = 0; i < args.size(); i++) {
                if (!args[i].isStringId() ) {
                    Tools::errorf("Usage struct [string field] [string field] ...\n");
                    return true;
                }
            }
            StructObject* obj = new StructObject();
            ret = Value(obj);

            for(size_t i = 0; i < args.size(); i++) {

                const std::string &fieldName = args[i].getStringRef();
                if (!isValidVariableName(fieldName)) continue;

                uint32_t id = SymbolTable::insert(fieldName);
                obj->mFieldOrder.push_back(id);
                obj->mDynmaicFields[id] = Value(0);

            }
            return true;
        });
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
        // RegisterFunction("int::reserve", [](std::vector<Value>& args, Value& ret) -> bool {
        //     if (args.size() < 2 || !args[0].isStringId() || !args[1].isInt()) {
        //         Tools::errorf("Reserve int variables for array like usage.Usage: int::reserve string variablename int count [int defaultValue]");
        //         return false;
        //     }
        //
        //     const std::string& varname = args[0].getStringRef();
        //
        //     if (!isValidVariableName(varname)) {
        //         Tools::errorf("Reserve int variable name %s invalid!\n", varname.c_str());
        //         return true;
        //     }
        //
        //     int count = args[1].getInt();
        //
        //     if (count < 1 || count > 100000) {
        //         Tools::errorf("Count is out of bounds: %d  (min 0, max 100000)\n", count);
        //         return true;
        //
        //     }
        //
        //     Value defaultVal = Value(0);
        //     if (args.size() > 2 && args[2].isNumber()) {
        //         defaultVal = Value(args[2].getInt());
        //     }
        //
        //     for ( int i = 0; i < count; i++) {
        //         uint32_t id = SymbolTable::insert(genArrayVar(varname,  Value(i)));
        //         gCurrentFrame->setVariable(id, Value(defaultVal));
        //     }
        //
        //
        //     return true;
        // });


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

        // ---------------------------------------------------------------------
        // RegisterFunction("float::reserve", [](std::vector<Value>& args, Value& ret) -> bool {
        //     if (args.size() < 2 || !args[0].isStringId() || !args[1].isInt()) {
        //         Tools::errorf("Reserve int variables for array like usage.Usage: int::reserve string variablename int count [int defaultValue]");
        //         return false;
        //     }
        //
        //     const std::string& varname = args[0].getStringRef();
        //
        //     if (!isValidVariableName(varname)) {
        //         Tools::errorf("Reserve int variable name %s invalid!\n", varname.c_str());
        //         return true;
        //     }
        //
        //     int count = args[1].getInt();
        //
        //     if (count < 1 || count > 100000) {
        //         Tools::errorf("Count is out of bounds: %d  (min 0, max 100000)\n", count);
        //         return true;
        //
        //     }
        //
        //     Value defaultVal = Value(0.0);
        //     if (args.size() > 2 && args[2].isNumber()) {
        //         defaultVal = Value(args[2].getDouble());
        //     }
        //
        //     for ( int i = 0; i < count; i++) {
        //         uint32_t id = SymbolTable::insert(genArrayVar(varname,  Value(i)));
        //         gCurrentFrame->setVariable(id, Value(defaultVal));
        //     }
        //
        //
        //     return true;
        // });

        // ---------------------------------------------------------------------
        // CORE
        // ---------------------------------------------------------------------
        RegisterFunction("core::eval", [&env](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isStringId()) {
                Tools::errorf("Usage: core::eval codeled\n");
                ret = Value(0);
                return true;
            }
            std::stringstream stream(args[0].getStringRef());
            ret = RunScriptStream(stream, env);
            return true;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("core::getType", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: core::getType value");
                return false;
            }
            ret = Value(args[0].getTypeName());
            return true;
        });
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
        });
        // ---------------------------------------------------------------------
        // sleep ms
        // a main loop in script
        // ---------------------------------------------------------------------
        RegisterFunction("core::sleep", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) {
                 Tools::errorf("Usage: core::sleep ms\n");
                 return false;
            }
            Tools::sleep(args[0].asUInt());
            return true;
        });


        // ---------------------------------------------------------------------
        // Garbage collection
        // ---------------------------------------------------------------------
        RegisterFunction("core::gc", [](std::vector<Value>& args, Value& ret) -> bool {
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
                "help::syntax    => print syntax informations.\n"
                "help::fn        => print the available Function\n"
                "help::objects   => print the available Object-Types\n"
                "help::const     => print the registered Constants\n"
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


        // ---------------------------------------------------------------------
        RegisterFunction("help::fn", [](std::vector<Value>& args, Value& ret) -> bool {
            bool doFilter = false;
            std::string filter;
            if (!args.empty() && args[0].isStringId()) {
                filter = args[0].getString();
                if (!filter.empty()) doFilter = true;
            }

            std::vector<std::string> nativeNames;
            for (const auto& [key, value] : RegisteredFunctions) {
                std::string name = SymbolTable::getName(key);
                if (!doFilter || name.find(filter) != std::string::npos) {
                    nativeNames.push_back(name);
                }
            }

            std::vector<std::string> scriptNames;
            for (const auto& [key, value] : RegisteredScriptFunctions) {
                std::string name = SymbolTable::getName(key);
                if (!doFilter || name.find(filter) != std::string::npos) {
                    scriptNames.push_back(name);
                }
            }

            std::sort(nativeNames.begin(), nativeNames.end());
            std::sort(scriptNames.begin(), scriptNames.end());

            Tools::printf("  --- Functions [%zu/%zu] --- \n", nativeNames.size(), RegisteredFunctions.size());
            for (const std::string& name : nativeNames) {
                Tools::printf("  - %s \n", name.c_str());
            }

            if (scriptNames.size() > 0) {
                Tools::printf("  --- Script Functions [%zu/%zu] --- \n", scriptNames.size(), RegisteredScriptFunctions.size());
                for (const std::string& name : scriptNames) {
                    Tools::printf("  - %s \n", name.c_str());
                }
            }

            return true;
        });

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
        });
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
        });

        // ---------------------------------------------------------------------
    } //RegisterCoreFunctions

} //namespace
