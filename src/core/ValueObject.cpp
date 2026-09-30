//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value Object
//-----------------------------------------------------------------------------
#include <vector>

#include <string.h>

#include "Value.h"
#include "ValueObject.h"
#include "VariableFrame.h"


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
    ValueObject::ValueObject(int t) : mType(t) {
        if (gCurrentFrame) gCurrentFrame->addToGarbageCollection(this);
    }
    // -------------------------------------------------------------------------
    std::string  ValueObject::toString() {
        char buff[64];
        snprintf(buff, sizeof(buff), "%s [%p] ", GetObjectTypeName(this), (void*)this);
        return std::string(buff);
    }
    // -------------------------------------------------------------------------

}
