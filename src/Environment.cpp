//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "Environment.h"
#include "core/Types.h"
#include "core/Value.h"

namespace DreiZehn{


    S64 Environment::getGlobalRegister(U32 symId){
        auto it = mGlobalsLookUp.find(symId);
        if (it != mGlobalsLookUp.end()) [[likely]] {
            return it->second;
        }
        return -1;
    }

    size_t Environment::reserveGlobal(U32 symId){
        S64 index = getGlobalRegister(symId);
        if (index < 0 ) {
            index = mGlobalsRegister.size();
            mGlobalsLookUp[symId] = index;
            mGlobalsRegister.push_back(Value());
        }
        return (size_t) index;
    }

    bool Environment::getGlobalVariableValueRef(const std::string& varName, Value& ret){
        S64 index = getGlobalRegister(SymbolTable::insert(varName));
        if (index < 0 ) return false;
        ret = getGlobal((size_t)index);
        return true;
    }

    void Environment::printGlobals(){
        for (auto & globs: mGlobalsLookUp) {
            Value& val = getGlobal(globs.second);
            Tools::printf("%s = %s\n "
            ,  SymbolTable::getName(globs.first).c_str()
            , val.toString().c_str());
        }
    }

    S64 Environment::getLocalsRootRegister(U32 symId){
        auto it = mRootLocalsLookUp.find(symId);
        if (it != mRootLocalsLookUp.end()) [[likely]] {
            return it->second;
        }
        return -1;
    }

    // DANGER mLocalsSize
    size_t Environment::reserveRootLocal(U32 symId){
        S64 index = getLocalsRootRegister(symId);
        if (index < 0 ) {
            index = mLocalVarCount[0];
            mLocalVarCount[0]++;
            if (mLocalVarCount.size() == 1) mLocalsSize++;
            mRootLocalsLookUp[symId] = index;
            // if it's in a function this overwrite a current local! mLocalsRegister[index] = Value();
        }
        return (size_t) index;
    }
} //namespace
