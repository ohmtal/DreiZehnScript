//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Variable Frame
//
// it may be rewritten to register based Vector but since i prefer the bytecode
// a slower recursive call is not a big problem.
//-----------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <memory>

#include "Value.h"
#include "ValueObject.h"
#include "GarbageCollection.h"
#include "toolbox/Tools.h"

namespace DreiZehn {

// -----------------------------------------------------------------------------
// Global Access:
class VariableFrame;

// -----------------------------------------------------------------------------
#ifndef DREIZEHN_INITIAL_VARIABLE_MASTERFRAME_SIZE
#define DREIZEHN_INITIAL_VARIABLE_MASTERFRAME_SIZE 256
#define DREIZEHN_INITIAL_VARIABLE_CHILDFRAME_SIZE 8
#endif
// // -----------------------------------------------------------------------------

class VariableFrame {
private:
    // variables stack
    std::unordered_map<uint32_t, std::unique_ptr<Value>> mVariables;
    VariableFrame* mParentFrame = nullptr;

    static inline uint32_t gFrameCounter = 0;
    uint32_t mFrameId = 0;
public:

    VariableFrame( VariableFrame* parentFrame ) {
        mFrameId = (gFrameCounter++);
        if (parentFrame == nullptr) {
            Globals::MasterVariableFrame = this;
            mVariables.reserve(DREIZEHN_INITIAL_VARIABLE_MASTERFRAME_SIZE);
        } else {
            mVariables.reserve(DREIZEHN_INITIAL_VARIABLE_CHILDFRAME_SIZE);
        }
        mParentFrame = parentFrame;
    }
    ~VariableFrame() {
        if (!Globals::shutDownRUNNING) {
            for(auto& [key,ptr]: mVariables) {
                if (ptr && ptr->isPointer()) {
                    ptr->asPointerObject()->setAssigned(false);
                }
            }
        }

    }

    inline uint32_t getFrameId() {return mFrameId;}

private:
    void internalSetVariable(Value& pre, Value& post) {
        if (pre.isPointer())  pre.asPointerObject()->setAssigned(false);
        if (post.isPointer()) post.asPointerObject()->setAssigned(true);
        pre = post;
    }

    // -------------------------------------------------------------------------
    // bool tryUpdateVariable(uint32_t id, Value val) {
    //     auto it = mVariables.find(id);
    //     if (it != mVariables.end()) {
    //         internalSetVariable(it->second , val);
    //         return true;
    //     }
    //     if (mParentFrame != nullptr) {
    //         return mParentFrame->tryUpdateVariable(id, val);
    //     }
    //     return false;
    // }
public:
    // -------------------------------------------------------------------------
    inline VariableFrame* getParentFrame() {return mParentFrame;}
    // -------------------------------------------------------------------------

    //NOTE: only used at iter and fn definition at the moment
    //      maybe renamed to add Variable!!!
    inline void setVariable(uint32_t id, Value val, bool forceScope = false) {

        #ifdef DREIZEHN_DEBUG
        if (Globals::gShowVariableDebug) Tools::printf("DEBUG: setVariable :: name: %s id: %d, floatval: %f\n", SymbolTable::getName(id).c_str(), id, val.getFloat());
        #endif

        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            internalSetVariable(*it->second , val);
            return;
        }
        if (forceScope) {
            if (val.isPointer()) val.asPointerObject()->setAssigned(true);
            mVariables[id] =  std::make_unique<Value>(val);
            // internalSetVariable( *mVariables[id] , val);
            return;
        }

        // fallback
        if (Globals::MasterVariableFrame) {
            Globals::MasterVariableFrame->setVariable(id, val, true);
        }
        Tools::errorf("\n\tVariable fatal error ! can NOT set the variable !!!!\n\n");
        // if (mParentFrame != nullptr) {
        //     if (mParentFrame->tryUpdateVariable(id, val)) {
        //         return;
        //     }
        // }

        // internalSetVariable( mVariables[id] , val);
    }

    // // -------------------------------------------------------------------------
   inline Value getVariable(uint32_t id) {
       Value* varPtr = getVariablePtr(id);
       if (varPtr) return *varPtr;

        // std::string varName = SymbolTable::getName(id);
        // Tools::warnf("Variable not found: %s\n", varName.c_str());
        return Value();
    }

    // // -------------------------------------------------------------------------
    inline Value* getVariablePtr(uint32_t id, bool isAssignVar = false) {

        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            return it->second.get();
        }

        if (this != Globals::MasterVariableFrame &&  Globals::isGlobalVar(id)) {
            return Globals::MasterVariableFrame->getVariablePtr(id,isAssignVar);
        }

        if (!isAssignVar) {
            std::string varName = SymbolTable::getName(id);
            Tools::warnf("Variable not found: %s\n", varName.c_str());
            return nullptr;
        }

        mVariables[id] = std::make_unique<Value>(NumberNullValue);
        return mVariables[id].get();

    }


};


// -----------------------------------------------------------------------------
} // namespace DreiZehn
