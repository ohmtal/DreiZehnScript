//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Variable Frame
//-----------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <cassert>

#include "Value.h"
#include "ValueObject.h"
#include "toolbox/Tools.h"

namespace DreiZehn {

// -----------------------------------------------------------------------------
// Global Access:
class VariableFrame;
inline VariableFrame* gCurrentFrame = nullptr;
// all objects go into the gMasterFrame for
// GarbageCollection !!!
inline VariableFrame* gMasterFrame = nullptr;
// -----------------------------------------------------------------------------
class VariableFrame {
private:
    // variables stack
    std::unordered_map<uint32_t, Value> mVariables;
    // Garbage collection
    std::vector<ValueObject*> mGarbageCollection;
    int mGarbageCheckCounter = 0;


    VariableFrame* mParentFrame = nullptr;





public:
    VariableFrame( VariableFrame* parentFrame ) {
        gCurrentFrame = this;
        if (parentFrame == nullptr) {
            gMasterFrame = this;
            mGarbageCollection.reserve(512);
        }
        mParentFrame = parentFrame;
    }
    ~VariableFrame() {
        doGarbageCollection(true);
        gCurrentFrame = mParentFrame;
    }

    // -------------------------------------------------------------------------
    // Variable getter/setter
    // -------------------------------------------------------------------------
private:
    void internalSetVariable(Value& pre, Value& post) {
        if (pre.isPointer()) static_cast<ValueObject*>(pre.asPointer())->setAssigned(false);
        if (post.isPointer()) static_cast<ValueObject*>(post.asPointer())->setAssigned(true);
        pre = post;
    }
public:
    // -------------------------------------------------------------------------
    inline void setVariable(uint32_t id, Value val) {

        // if (Globals::gShowVariableDebug) Tools::printf("DEBUG: setVariable :: name: %s id: %d, floatval: %f\n", SymbolTable::getName(id).c_str(), id, val.getFloat());

        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            // it->second = val;
            internalSetVariable(it->second , val);
            return;
        }

        if (mParentFrame != nullptr) {
            if (mParentFrame->tryUpdateVariable(id, val)) {
                return;
            }
        }



        // mVariables[id] = val;
        internalSetVariable( mVariables[id] , val);
    }
    // -------------------------------------------------------------------------
    inline bool tryUpdateVariable(uint32_t id, Value val) {
        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            // it->second = val;
            internalSetVariable(it->second , val);
            return true;
        }
        if (mParentFrame != nullptr) {
            return mParentFrame->tryUpdateVariable(id, val);
        }
        return false;
    }
    // // -------------------------------------------------------------------------
   inline Value getVariable(uint32_t id) {
        // if (Globals::gShowVariableDebug) Tools::printf("DEBUG: getVariable :: name: %s id: %d\n", SymbolTable::getName(id).c_str(), id);

        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            return it->second;
        }

        if (mParentFrame != nullptr) {
            return mParentFrame->getVariable(id);
        }

        std::string varName = SymbolTable::getName(id);
        Tools::errorf("Variable not found: %s\n", varName.c_str());
        return Value();
    }

    inline Value* getVariablePtr(uint32_t id) {
        // if (Globals::gShowVariableDebug) Tools::printf("DEBUG: getVariable :: name: %s id: %d\n", SymbolTable::getName(id).c_str(), id);

        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            return &it->second;
        }

        if (mParentFrame != nullptr) {
            return mParentFrame->getVariablePtr(id);
        }

        // not found we set a new one !

        std::string varName = SymbolTable::getName(id);
        mVariables[id] = Value(0);
        Tools::errorf("Variable not found: %s\n", varName.c_str());
        return &mVariables[id];
    }
    // -------------------------------------------------------------------------
    // GarbageCollection
    // -------------------------------------------------------------------------
    inline void addToGarbageCollection(ValueObject* obj) {
        assert(gMasterFrame && "addToGarbageCollection but Frame have not MasterFrame!!!");
        gMasterFrame->mGarbageCollection.push_back(obj);
        mGarbageCheckCounter++;
        if (mGarbageCheckCounter > 500) {
            mGarbageCheckCounter = 0;
            doGarbageCollection(false);
        }

    }

    inline void listGarbageObjects() {
       assert(gMasterFrame && "listGarbageObjects but Frame have not MasterFrame!!!");
       int i = 0;
       for (auto* obj : gMasterFrame->mGarbageCollection) {
            // std::cout << "Object-Type: " << typeid(*obj).name() << "\n";
           int objtype = obj->mType;
           Tools::printf("#%d [%p] assigned: %d type:%d %s\n"
                         , i, (void*)obj, obj->mAssigned
                         , objtype, gUserObjectTypes[objtype].c_str());

           i++;
       }
    }

    inline void doGarbageCollection(bool calledOnDestructor) {
       const bool doMaster = (
           gMasterFrame
           && this == gMasterFrame
           && calledOnDestructor
       );
       if (doMaster) {
           for (auto* obj : mGarbageCollection) {
               delete obj;
           }
           mGarbageCollection.clear();
       } else {
           auto it = std::remove_if(mGarbageCollection.begin(), mGarbageCollection.end(), [](auto* obj) {
               if (obj->mAssigned < 1) {
                   delete obj;
                   return true; // mark for delete
               }
               return false;
           });

           mGarbageCollection.erase(it, mGarbageCollection.end());
       }

       //DEBUG: if (!calledOnDestructor) listGarbageObjects();
    }
    // -------------------------------------------------------------------------
};


} // namespace DreiZehn
