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

    // ValueObjectProperty::ValueObjectProperty(std::string name,
    //     uint32_t minParams, uint32_t maxParams, std::string help, int valueObjectTypeId )
    // : mIsMethod(true),mName(name),  mHelp(help), mMinParams(minParams), mMaxParams(maxParams) {
    //     mSymbolId = SymbolTable::insert(name);
    //     if (valueObjectTypeId > 0) {
    //         auto it = gUserObjectTypes.find(valueObjectTypeId);
    //         if (it != gUserObjectTypes.end()) {
    //             it->second.mProperties.push_back(*this);
    //         }
    //         assert(true && "registerObjectProperty impossible on unknown valueObjectTypeId");
    //     }
    // }
    // -------------------------------------------------------------------------
    bool ValueObjectProperty::ValidateArgs( std::vector<Value>& args) {
        if (args.size() < mMinParams || args.size() > mMaxParams) {
            Tools::errorf("Method %s parameter error. min:%d max:%d\n%s\n", mName.c_str(),mMinParams, mMaxParams, mHelp.c_str());
            return false;
        }
        return true;
    }


    // -------------------------------------------------------------------------

}
