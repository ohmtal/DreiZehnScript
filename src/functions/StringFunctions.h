//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String Functions and Object
//-----------------------------------------------------------------------------
#pragma once
#include "core/FunctionMap.h"
#include "ScriptLoader.h"
#include <core/VariableFrame.h>

namespace DreiZehn {

    // =============================================================================
    // --- StringObject ---
    // =============================================================================

     const int TypeStringObject =  RegisterUserObjectType("String");

    struct StringObject : public ValueObject {
        Value mValue;
        StringObject(std::string str) : ValueObject(TypeStringObject), mValue(str) {}


        static inline ValueObjectProperty valueProp; //Field
        static inline ValueObjectProperty appendProp;
        static inline ValueObjectProperty toNumberProp;
        static inline ValueObjectProperty getLenProp;
        static inline ValueObjectProperty getCharProp;
        // -------------------------------------------------------------------------
        inline static void RegisterSymbols() {
            // ValueObjectProperty(std::string name,  uint32_t minParams, uint32_t maxParams, std::string help)

            // method
            valueProp = ValueObjectProperty("value","Return the String", TypeStringObject);

            appendProp =  ValueObjectProperty("append",1,16,"append up to 16 values to the string and return the result"
                , TypeStringObject
            );

            toNumberProp =  ValueObjectProperty("toNumber",0,0
                ,"Return the number representation of the String", TypeStringObject);

            getLenProp = ValueObjectProperty("len",0,0,"Return the length the String", TypeStringObject);

            getCharProp = ValueObjectProperty("char",1,1
                ,"Return the int value of on character. Usage: .char index", TypeStringObject);

        }
        // -------------------------------------------------------------------------
        inline std::string toString() override{
            return mValue.getString();
        }
        // -------------------------------------------------------------------------
        inline Value* onGetFieldPtr(uint32_t fieldSymbolId) override {
            if (fieldSymbolId == valueProp.mSymbolId) return &mValue;

            return nullptr;
        }
        // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override{
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (!ptr) return false;
            if (value.isStringId() ) {
                *ptr = value;
            } else {
                *ptr = Value("");
            }
            return true;
        }
        // -------------------------------------------------------------------------
        inline bool onGetField(uint32_t fieldSymbolId, Value& ret) override{
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (!ptr) return false;
            ret = *ptr;
            return true;
        }
        // -------------------------------------------------------------------------
        inline bool onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) override{


            // --------- append
            if (methodId == appendProp.mSymbolId ) {
                if (!appendProp.ValidateArgs(args)) return false;
                std::string resultStr = mValue.getString();

                for ( auto& val : args) {
                    if (val.isInt()) {
                        resultStr += std::to_string(val.asInt());
                    }
                    else if (val.isDouble()) {
                        std::string dStr = std::to_string(val.asDouble());
                        dStr.erase(dStr.find_last_not_of('0') + 1, std::string::npos);
                        if (dStr.back() == '.') dStr.pop_back();
                        resultStr += dStr;
                    }
                    else if (val.isStringId()) {
                        resultStr += val.getString();
                    }
                }
                mValue = Value(resultStr);
                ret = mValue;
                return true;
            }
            else
            // --------- toNumber
            if (methodId == toNumberProp.mSymbolId ) {
                if (!toNumberProp.ValidateArgs(args)) return false;
                char* endptr = nullptr;
                std::string s = mValue.getString();
                double resDouble = std::strtod(s.c_str(), &endptr);
                if (s.empty() || *endptr != '\0') {
                    ret = Value(0);
                } else {
                    ret = Value(resDouble);
                }
                return true;
            }
            else
                // --------- ->len
                if (methodId == getLenProp.mSymbolId ) {
                    if (!getLenProp.ValidateArgs(args)) return false;
                    ret = Value(static_cast<int32_t>(mValue.getStringRef().length()));
                    return true;

                }
                // --------- char
                if (methodId == getCharProp.mSymbolId ) {
                    if (!getCharProp.ValidateArgs(args)) return false;
                    int32_t offset = args[0].getInt();
                    std::string s = mValue.getStringRef();
                    if (offset >= 0 && offset < s.length()) {
                        ret = Value(static_cast<int32_t>(s[offset]));
                    }
                    return false;
                }
                else
                {
                    Tools::errorf("Unknown String method: %s\n", SymbolTable::getName(methodId).c_str());
                }

                return false;
        }

    };


    // =============================================================================
    // --- RegisterStringFunctions ---
    // =============================================================================

    void RegisterStringFunctions( ) {
        using namespace FunctionMap;

        // ---------------------------------------------------------------------
        StringObject::RegisterSymbols();
        RegisterFunction("String::new", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isStringId()) {
                Tools::errorf("Usage: String::new string");
                return false;
            }
            StringObject* s = new StringObject(args[0].getString());
            ret = Value(s);
            return true;
        });
        // ---------------------------------------------------------------------
    } //RegisterStringFunctions

} //namespace
