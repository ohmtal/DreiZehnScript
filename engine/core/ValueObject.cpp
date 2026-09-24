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
    bool StringValueObject::onMethodCall(uint32_t methodNameSymbolId,  std::vector<Value>& args, Value& ret) {

        if (methodNameSymbolId == SymbolTable::insert( "toNumber" ) ) {
            char* endptr = nullptr;
            double resDouble = std::strtod(mValue.c_str(), &endptr);
            if (mValue.empty() || *endptr != '\0') {
                ret = Value(0);
            } else {
                ret = Value(resDouble);
            }
            return true;
        }
        return false;
    }

    // -------------------------------------------------------------------------

}
