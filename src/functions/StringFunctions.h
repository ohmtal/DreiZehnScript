//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Parts of formatString are borrowed from ElfScript/TorqueScript:
//      Copyright (c) 2012 GarageGames, LLC
//      Copyright (c) 2026 Thomas Hühn (XXTH)
//      SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String Functions and Object
//-----------------------------------------------------------------------------
#pragma once
#include "core/FunctionMap.h"
#include "ScriptLoader.h"
#include <core/VariableFrame.h>
#include <string.h>

namespace DreiZehn {

    // =============================================================================
    // --- formatString ---
    // =============================================================================
    std::string formatString(std::vector<Value>& args) {

        if (args.size() < 1) return "";

        const char* fmt = args[0].getStringRef().c_str();
        int currentArgIndex = 1; // First arg at 1

        std::string result = "";

        while (*fmt)
        {
            if (*fmt == '%')
            {
                if (*(fmt + 1) == '\0')
                {
                    result += "%";
                    break;
                }

                const char* specStart = fmt;
                fmt++;

                // We have a double %%, it's just an escaped percent sign
                if (*fmt == '%')
                {
                    result += '%';
                    fmt++;
                    continue;
                }

                if (currentArgIndex >= args.size())
                {
                    Tools::errorf("formatString: invalid argument count !!");
                    break;
                }

                Value& curVal = args[currentArgIndex];
                currentArgIndex++;

                // skip flags
                while (*fmt != '\0' && !isalpha(*fmt))
                {
                    fmt++;
                }

                // long/long flags check
                bool isLongLong = false;
                bool isLong = false;
                bool isLongDouble = false;

                // modifier flags
                while (*fmt == 'l' || *fmt == 'h' || *fmt == 'z' || *fmt == 'L')
                {
                    if (*fmt == 'l')
                    {
                        if (isLong) { isLongLong = true; isLong = false; }
                        else { isLong = true; }
                    }
                    else if (*fmt == 'L')
                    {
                        isLongDouble = true;
                    }
                    fmt++;
                }

                // null check
                if (*fmt == '\0')
                {
                    Tools::errorf("formatString: invalid format specifier!!");
                    break;
                }

                char tokenBuffer[512];
                tokenBuffer[0] = '\0';

                // calc length
                uint32_t specLen = (fmt - specStart) + 1;
                char specBuffer[32];
                if (specLen > 31)
                {
                    Tools::errorf("formatString: format specifier too long!");
                    break;
                }

                strncpy(specBuffer, specStart, specLen);
                specBuffer[specLen] = '\0';

                // here is the beaf
                switch (*fmt)
                {
                    case 's': // String
                        snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, curVal.getStringRef().c_str());
                        break;

                    case 'c':
                    case 'C':
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X':
                    {
                        if (isLongLong)
                        {
                            if (*fmt == 'u' || *fmt == 'x' || *fmt == 'X')
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (unsigned long long)curVal.getUInt());
                            else
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (long long)curVal.getInt());
                        }
                        else if (isLong)
                        {
                            if (*fmt == 'u' || *fmt == 'x' || *fmt == 'X')
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (unsigned long)curVal.getUInt());
                            else
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (long)curVal.getInt());
                        }
                        else //  32-Bit Integer
                        {
                            if (*fmt == 'u' || *fmt == 'x' || *fmt == 'X')
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (unsigned int)curVal.getUInt());
                            else
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (int)curVal.getInt());
                        }
                        break;
                    }

                    case 'e':
                    case 'E':
                    case 'f':
                    case 'g':
                    case 'G':
                    {
                        // sprintf erwartet für %f/%e/%g standardmäßig double (64-Bit Fließkomma)
                        if (isLongDouble)
                            snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (long double)curVal.getDouble());
                        else
                            snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, curVal.getDouble());
                        break;
                    }

                    case 'p':
                        snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (void*)curVal.getPointer());
                        break;

                    default:
                        Tools::errorf("formatString: unknown type: '%c'!", *fmt);
                        tokenBuffer[0] = '\0';
                        break;
                }

                result += tokenBuffer;
            }
            else
            {
                result += *fmt;
            }

            fmt++;
        }

        return result;
    }

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

            return  ValueObject::onGetFieldPtr(fieldSymbolId);
        }
        // -------------------------------------------------------------------------
        inline bool onSetField(uint32_t fieldSymbolId, const Value& value) override{
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (!ptr) return ValueObject::onSetField(fieldSymbolId, value);
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
            if (!ptr) return ValueObject::onGetField(fieldSymbolId, ret);
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
            else
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

            return ValueObject::onMethodCall(methodId, args, ret);
        }

    };


    // =============================================================================
    // --- RegisterStringFunctions ---
    // =============================================================================

    void RegisterStringFunctions(Environment& env ) {
        using namespace FunctionMap;

        // ---------------------------------------------------------------------
        StringObject::RegisterSymbols();
        RegisterFunction("String::new", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isStringId()) {
                Tools::errorf("Usage: String::new string\n");
                return false;
            }
            StringObject* s = new StringObject(args[0].getString());
            ret = Value(s);
            return true;
        });
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        RegisterFunction("str::concat", [](std::vector<Value>& args, Value& ret) -> bool {
            std::string resultStr = "";

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

            ret = Value(resultStr);
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("str::format", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() < 1) {
                ret = Value("");
                return true;
            }
            ret = Value( formatString(args));
            return true;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("str::cast", [](std::vector<Value>& args, Value& ret) -> bool {

            if (args.size() < 1 || args[0].isPointer()) {
                ret = Value("");
                return true;
            }
            if (args[0].isInt()) ret = Value (std::to_string(args[0].asFastInt()));
            else
            if (args[0].isDouble()) ret = Value (std::to_string(args[0].asFastDouble()));

            return true;
        });


    } //RegisterStringFunctions

} //namespace
