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
            for (const auto& val : args) {
                if (val.isInt()) {
                    Tools::printf("%d ", val.asInt());
                } else if (val.isDouble()) {
                    Tools::printf("%f ", val.asDouble());
                } else if (val.isPointer()) {
                    auto* obj = static_cast<ValueObject*>(val.asPointer());
                    if (obj->mType == ValueObjectType::String) {
                        auto* strObj = static_cast<StringValueObject*>(obj);
                        Tools::printf("%s ", strObj->mValue.c_str());
                    } else {
                        Tools::printf("%p ", (void*)obj);
                    }
                }
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
        RegisterFunction("core.gc", [&env](std::vector<Value>& args, Value& ret) -> bool {
            if (gCurrentFrame) {
                gCurrentFrame->doGarbageCollection(false);
                return true;
            }
            return false;
        });
        // ---------------------------------------------------------------------
    } //RegisterCoreFunctions

} //namespace
