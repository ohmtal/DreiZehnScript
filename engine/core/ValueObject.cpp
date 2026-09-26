//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value Object
//-----------------------------------------------------------------------------
#include <vector>

// #include <stdlib.h>
#include <string.h>
// #include <ctype.h>
// #include <cstdarg>

#include "Value.h"
#include "ValueObject.h"


namespace DreiZehn {
    // -------------------------------------------------------------------------
    bool ValueObjectProperty::ValidateArgs( std::vector<Value>& args) {
        if (args.size() < mMinParams || args.size() > mMaxParams) {
            Tools::errorf("Method %s parameter error. min:%d max:%d\n%s\n", mName.c_str(),mMinParams, mMaxParams, mHelp.c_str());
            return false;
        }
        return true;
    }
    // -------------------------------------------------------------------------
    void StringValueObject::RegisterSymbols() {
        // ValueObjectProperty(std::string name,  uint32_t minParams, uint32_t maxParams, std::string help)
        toNumberProp =  ValueObjectProperty("toNumber",0,0,"Return the number representation of the String");
        RegisterObjectProperty(ValueObjectType::String,toNumberProp);

        getLenProp = ValueObjectProperty("len",0,0,"Return the length the String");
        RegisterObjectProperty(ValueObjectType::String,getLenProp);

        getCharProp = ValueObjectProperty("char",1,1,"Return the int value of on character. Usage: .char index");
        RegisterObjectProperty(ValueObjectType::String, getCharProp);
    }
    // -------------------------------------------------------------------------
    bool StringValueObject::onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) {


        // --------- toNumber
        if (methodId == toNumberProp.mSymbolId ) {
            if (!toNumberProp.ValidateArgs(args)) return false;
            char* endptr = nullptr;
            double resDouble = std::strtod(mValue.c_str(), &endptr);
            if (mValue.empty() || *endptr != '\0') {
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
            ret = Value(static_cast<int32_t>(mValue.length()));
            return true;

        }
        // --------- char
        if (methodId == getCharProp.mSymbolId ) {
            if (!getCharProp.ValidateArgs(args)) return false;
            int32_t offset = args[0].getInt();
            if (offset >= 0 && offset < mValue.length()) {
                ret = Value(static_cast<int32_t>(mValue[offset]));
            }
            return false;
        }
        else
        {
            Tools::errorf("Unknown String method: %s\n", SymbolTable::getName(methodId).c_str());
        }

        return false;
    }

    // -------------------------------------------------------------------------

}
