//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Variable Frame
//-----------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <cassert>
#include <algorithm>

#include "Value.h"
#include "ValueObject.h"
#include "GarbageCollection.h"
#include "toolbox/Tools.h"

#ifndef DREIZEHN_INITIAL_GARBAGE_SIZE
#define DREIZEHN_INITIAL_GARBAGE_SIZE 2048
#endif

namespace DreiZehn {

// -----------------------------------------------------------------------------
// Global Access:
class VariableFrame;

inline VariableFrame* gCurrentFrame = nullptr;

inline VariableFrame* gMasterFrame = nullptr;
// -----------------------------------------------------------------------------
#ifndef DREIZEHN_INITIAL_VARIABLE_MASTERFRAME_SIZE
#define DREIZEHN_INITIAL_VARIABLE_MASTERFRAME_SIZE 256
#define DREIZEHN_INITIAL_VARIABLE_CHILDFRAME_SIZE 8
#endif
// // -----------------------------------------------------------------------------

class VariableFrame {
private:
    // variables stack
    std::unordered_map<uint32_t, Value> mVariables;
    VariableFrame* mParentFrame = nullptr;

    static inline uint32_t gFrameCounter = 0;
    uint32_t mFrameId = 0;
public:

    VariableFrame( VariableFrame* parentFrame ) {
        mFrameId = (gFrameCounter++);
        gCurrentFrame = this;
        if (parentFrame == nullptr) {
            gMasterFrame = this;
            mVariables.reserve(DREIZEHN_INITIAL_VARIABLE_MASTERFRAME_SIZE);
        } else {
            mVariables.reserve(DREIZEHN_INITIAL_VARIABLE_CHILDFRAME_SIZE);
        }
        mParentFrame = parentFrame;
    }
    ~VariableFrame() {
        for(auto v: mVariables) {
            if (v.second.isPointer()) {
                v.second.asPointerObject()->setAssigned(false);
            }
        }
        // NOT! GarbageCollection::run();
        gCurrentFrame = mParentFrame;
    }

    inline uint32_t getFrameId() {return mFrameId;}

private:
    void internalSetVariable(Value& pre, Value& post) {
        if (pre.isPointer())  pre.asPointerObject()->setAssigned(false);
        if (post.isPointer()) post.asPointerObject()->setAssigned(true);
        pre = post;
    }

    // -------------------------------------------------------------------------
    bool tryUpdateVariable(uint32_t id, Value val) {
        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            internalSetVariable(it->second , val);
            return true;
        }
        if (mParentFrame != nullptr) {
            return mParentFrame->tryUpdateVariable(id, val);
        }
        return false;
    }
public:
    // -------------------------------------------------------------------------
    inline VariableFrame* getParentFrame() {return mParentFrame;}
    // -------------------------------------------------------------------------
    inline void setVariable(uint32_t id, Value val, bool forceScope = false) {

        // if (Globals::gShowVariableDebug) Tools::printf("DEBUG: setVariable :: name: %s id: %d, floatval: %f\n", SymbolTable::getName(id).c_str(), id, val.getFloat());

        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            internalSetVariable(it->second , val);
            return;
        }
        if (forceScope) {
             internalSetVariable( mVariables[id] , val);
             return;
        }

        if (mParentFrame != nullptr) {
            if (mParentFrame->tryUpdateVariable(id, val)) {
                return;
            }
        }
        internalSetVariable( mVariables[id] , val);
    }

    // // -------------------------------------------------------------------------
   inline Value getVariable(uint32_t id) {
        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            return it->second;
        }

        if (mParentFrame != nullptr) {
            return mParentFrame->getVariable(id);
        }

        std::string varName = SymbolTable::getName(id);
        Tools::warnf("Variable not found: %s\n", varName.c_str());
        return Value();
    }

    inline Value* getVariablePtr(uint32_t id, bool isAssignVar = false) {
        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            return &it->second;
        }

        if (mParentFrame != nullptr) {
            return mParentFrame->getVariablePtr(id, isAssignVar);
        }
        // not found we set a new one !
        // warning is bad, when it's an assign!
        if (!isAssignVar) {
            std::string varName = SymbolTable::getName(id);
            Tools::warnf("Variable not found: %s\n", varName.c_str());
        }
        mVariables[id] = Value(0);
        return &mVariables[id];
    }

};
// -----------------------------------------------------------------------------
// TOOLS
// -----------------------------------------------------------------------------

inline bool isValidVariableName(const std::string& str) {
    if (str.empty()) return false; // Handle empty string if needed

    return std::all_of(str.begin(), str.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == ':';
    });
}

// -----------------------------------------------------------------------------
} // namespace DreiZehn
