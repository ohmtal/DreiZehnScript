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
    bool ValueObjectMethod::ValidateArgs( std::vector<Value>& args) {
        if (args.size() < mMinParams || args.size() > mMaxParams) {
            Tools::errorf("Method %s parameter error. min:%d max:%d %s\n", mName.c_str(),mMinParams, mMaxParams, mHelp.c_str());
            return false;
        }
        return true;
    }
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    bool StringValueObject::onMethodCall(uint32_t methodId,  std::vector<Value>& args, Value& ret) {
        static uint32_t toNumberId = SymbolTable::insert("toNumber");
        static uint32_t getCharId = SymbolTable::insert("char");
        static uint32_t getLen = SymbolTable::insert("len");
        // --------- toNumber
        if (methodId == toNumberId ) {
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
        if (methodId == getLen ) {
            if (args.size() != 0) {
                Tools::errorf("Usage obj->len\n");
                return false;
            }
            ret = Value(static_cast<int32_t>(mValue.length()));
            return true;

        }
        // --------- char
        if (methodId == getCharId ) {
            if (args.size() != 1) {
                Tools::errorf("Usage obj->char OFFSET\n");
                return false;
            }
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
