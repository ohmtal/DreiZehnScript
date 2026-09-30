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
#include "FunctionMap.h"


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
        mObjectTypeName = GetObjectTypeName(this);
    }
    // -------------------------------------------------------------------------
    std::string  ValueObject::toString() {
        char buff[64];
        snprintf(buff, sizeof(buff), "%s [%p] ", GetObjectTypeName(this), (void*)this);
        return std::string(buff);
    }
    // -------------------------------------------------------------------------
    bool ValueObject::onMethodCall(uint32_t methodNameSymbolId,  std::vector<Value>& args, Value& ret) {

        // ------ Base methods ----------
        if (methodNameSymbolId == sClassNameMethodId) {
            if (args.size() != 1 || !args[0].isStringId()) {
                Tools::errorf("Usage: setClassName string ClassName\n");
                ret = Value(0);
                return true;
            }
            mClassName = args[0].getString();
            ret = Value(1);
            return true;
        }
        if (methodNameSymbolId == sDumpMethodId) {
            Tools::printf("Object Type: %s, Class:%s\n", mObjectTypeName.c_str(), mClassName.c_str());

            Tools::printf("Dynamic Fields: %zu\n", mDynmaicFields.size());
            for ( auto& [key, value] : mDynmaicFields) {
                Tools::printf("  - %s = %s\n"
                    , SymbolTable::getName(key).c_str()
                    , value.toString().c_str());
            }

            ret = Value(1);
            return true;
        }

        //FIXME need also to do the same as the calling code below so I should add a function
        // if (methodNameSymbolId == sIsMethodId) {
        //     if (args.size() != 1 || !args[0].isStringId()) {
        //         Tools::errorf("Usage: isScriptMethod string MethodName\n");
        //         ret = Value(0);
        //         return true;
        //     }
        //     auto it = mMethodMap.find(StringTable::insert(args[0].getStringRef()));
        //     if (it != mMethodMap.end()) {
        //         ret = it->second;
        //         ret = Value(1);
        //         return true;
        //     }
        //     ret = Value(0);
        //     return true;
        // }
        // ----- User defined methods ----

        FunctionMap::ScriptFunction* sf;
        if (Globals::gCurEnv) {
            // try to fetch from mMethodMap
            auto it = mMethodMap.find(methodNameSymbolId);
            if (it != mMethodMap.end()) {
                sf = it->second;
            } else {
                // lookup Object name
                std::string methodStr = SymbolTable::getName(methodNameSymbolId);
                uint32_t fnId = SymbolTable::insert(std::string(mObjectTypeName + "::" + methodStr));
                sf = FunctionMap::GetScriptFunction(fnId);
                if (!sf && !mClassName.empty()) {
                    fnId = SymbolTable::insert(std::string(mClassName + "::" + methodStr));
                    sf = FunctionMap::GetScriptFunction(fnId);
                }
                // add to mMethodMap
                if (sf) mMethodMap[methodNameSymbolId] = sf;
            }

            if (sf) {
                args.insert(args.begin(), Value(this));
                ret = FunctionMap::CallScriptFunction(*Globals::gCurEnv, sf, args);
                return true;
            }
        }


        Tools::errorf("Runtime Error: %s Method %s not found.\n",mObjectTypeName.c_str(), SymbolTable::getName(methodNameSymbolId).c_str());
        return false;
    }

    // -------------------------------------------------------------------------
    bool ValueObject::onGetField(uint32_t fieldSymbolId, Value& ret) {
        auto it = mDynmaicFields.find(fieldSymbolId);
        if (it != mDynmaicFields.end()) {
            ret = it->second;
            return true;
        }
        Tools::errorf("Runtime Error GetField: Field %s not found.\n",SymbolTable::getName(fieldSymbolId).c_str());
        return false;
    }
    // -------------------------------------------------------------------------

    Value* ValueObject::onGetFieldPtr(uint32_t fieldSymbolId) {
        auto it = mDynmaicFields.find(fieldSymbolId);
        if (it != mDynmaicFields.end()) {
            return  &it->second;
        }
        Tools::errorf("Runtime Error GetField: Field %s not found.\n",SymbolTable::getName(fieldSymbolId).c_str());
        return nullptr;
    }
    // -------------------------------------------------------------------------

    bool ValueObject::onSetField(uint32_t fieldSymbolId, const Value& value) {
        auto it = mDynmaicFields.find(fieldSymbolId);
        if (it != mDynmaicFields.end()) {
            it->second = value;
            return true;
        } else {
            // insert
            mDynmaicFields[fieldSymbolId] = value;
            return true;
        }

        // Tools::errorf("Runtime Error SetField: Field %s not found.\n",SymbolTable::getName(fieldSymbolId).c_str());
        return false;
    }
}
