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
#include "toolbox/Tools.h"

#ifndef DREIZEHN_INITIAL_GARBAGE_SIZE
#define DREIZEHN_INITIAL_GARBAGE_SIZE 2048
#endif

namespace DreiZehn {


// -----------------------------------------------------------------------------
// Global Access:
class VariableFrame;
inline VariableFrame* gCurrentFrame = nullptr;
// all objects go into the gMasterFrame for
// GarbageCollection !!!
inline VariableFrame* gMasterFrame = nullptr;
inline size_t _GarbageCheckCounter = 0;
inline bool GarbageCollectionLocked = false;
// -----------------------------------------------------------------------------
class VariableFrame {
private:
    // variables stack
    std::unordered_map<uint32_t, Value> mVariables;
    // Garbage collection
    std::vector<ValueObject*> mGarbageCollection;

    VariableFrame* mParentFrame = nullptr;

public:
    VariableFrame( VariableFrame* parentFrame ) {
        gCurrentFrame = this;
        if (parentFrame == nullptr) {
            gMasterFrame = this;
            mGarbageCollection.reserve(DREIZEHN_INITIAL_GARBAGE_SIZE);
        }
        mParentFrame = parentFrame;
    }
    ~VariableFrame() {

        //unassign objects
        if (this != gMasterFrame) {
            for(auto v: mVariables) {
                if (v.second.isPointer()) {
                    v.second.asPointerObject()->setAssigned(false);
                }
            }
        }

        doGarbageCollection(true);
        gCurrentFrame = mParentFrame;
    }

    // -------------------------------------------------------------------------
    // Variable getter/setter
    // -------------------------------------------------------------------------
private:
    void internalSetVariable(Value& pre, Value& post) {
        if (pre.isPointer())  pre.asPointerObject()->setAssigned(false);
        if (post.isPointer()) post.asPointerObject()->setAssigned(true);
        pre = post;
    }
public:
    // -------------------------------------------------------------------------
    inline VariableFrame* getParentFrame() {return mParentFrame;}
    // -------------------------------------------------------------------------
    inline void setVariable(uint32_t id, Value val, bool forceScope = false) {

        // if (Globals::gShowVariableDebug) Tools::printf("DEBUG: setVariable :: name: %s id: %d, floatval: %f\n", SymbolTable::getName(id).c_str(), id, val.getFloat());

        auto it = mVariables.find(id);
        if (it != mVariables.end()) {
            // it->second = val;
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



        // mVariables[id] = val;
        internalSetVariable( mVariables[id] , val);
    }
    // -------------------------------------------------------------------------
    inline bool tryUpdateVariable(uint32_t id, Value val) {
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
        mVariables[id] = Value(0);
        return &mVariables[id];
    }
    // -------------------------------------------------------------------------
    // GarbageCollection
    // -------------------------------------------------------------------------
    // inline void addToGarbageCollection(ValueObject* obj) {
    //     assert(gMasterFrame && "addToGarbageCollection but Frame have not MasterFrame!!!");
    //     gMasterFrame->mGarbageCollection.push_back(obj);
    //     _GarbageCheckCounter++;
    //     if (!GarbageCollectionLocked && _GarbageCheckCounter > DREIZEHN_INITIAL_GARBAGE_SIZE / 2) {
    //         _GarbageCheckCounter = 0;
    //         doGarbageCollection(false);
    //     }
    //
    // }
    inline size_t getNextGcPrimeSize(size_t currentCapacity) {
        static const size_t primeSizes[] = {
            2048, 4099, 8209, 16411, 32771, 65537, 131101,
            262147, 524309, 1048583, 2097169, 4194319, 8388617
        };
        const size_t numPrimes = sizeof(primeSizes) / sizeof(primeSizes[0]);

        for (size_t i = 0; i < numPrimes; ++i) {
            if (primeSizes[i] > currentCapacity) {
                return primeSizes[i];
            }
        }
        return currentCapacity * 2; // Fallback
    }
    inline void addToGarbageCollection(ValueObject* obj) {
        assert(gMasterFrame && "addToGarbageCollection but Frame have not MasterFrame!!!");

        auto& gcVector = gMasterFrame->mGarbageCollection;

        if (gcVector.size() >= gcVector.capacity()) {
            size_t newCapacity = getNextGcPrimeSize(gcVector.capacity());
            gcVector.reserve(newCapacity);
        }

        gcVector.push_back(obj);
        _GarbageCheckCounter++;

        if (!GarbageCollectionLocked && _GarbageCheckCounter > (gcVector.capacity() / 2)) {
            _GarbageCheckCounter = 0;
            doGarbageCollection(false);
        }
    }


    inline void removeFromGarbageCollection(ValueObject* obj) {
        assert(gMasterFrame && "removeFromGarbageCollection but no MasterFrame!!!");
        auto& gc = gMasterFrame->mGarbageCollection;
        gc.erase(std::remove(gc.begin(), gc.end(), obj), gc.end());
    }

    inline size_t getGarbageSize() {
       assert(gMasterFrame && "getGarbageSize but Frame have not MasterFrame!!!");
       return gMasterFrame->mGarbageCollection.size();
    }
    inline void listGarbageObjects() {
       assert(gMasterFrame && "listGarbageObjects but Frame have not MasterFrame!!!");
       int i = 0;
       for (auto* obj : gMasterFrame->mGarbageCollection) {
            // std::cout << "Object-Type: " << typeid(*obj).name() << "\n";
           int objtype = obj->mType;
           Tools::printf("#%d [%p] assigned: %d type:%d %s\n"
                         , i, (void*)obj, obj->mAssigned
                         , objtype, GetObjectTypeName(obj));

           i++;
       }
       if (GarbageCollectionLocked) Tools::warnf("\n ---- GarbageCollection is locked!!! ----\n");
    }

    inline void doGarbageCollection(bool calledOnDestructor) {
       assert(gMasterFrame && "doGarbageCollection but Frame have not MasterFrame!!!");
       const bool doMaster = (
           gMasterFrame
           && this == gMasterFrame
           && calledOnDestructor
       );

       if (doMaster) {
           for (auto* obj : gMasterFrame->mGarbageCollection) {
               delete obj;
           }
           gMasterFrame->mGarbageCollection.clear();
       } else {

           auto it = std::remove_if(gMasterFrame->mGarbageCollection.begin()
                    , gMasterFrame->mGarbageCollection.end(), [](auto* obj) {
               if (obj->mAssigned < 1) {
                   delete obj;
                   return true; // mark for delete
               }
               return false;
           });

           gMasterFrame->mGarbageCollection.erase(it, gMasterFrame->mGarbageCollection.end());
       }

       // Tools::printf("DEBUG; gc:%zu\n", mGarbageCollection.size());
       //DEBUG: if (!calledOnDestructor) listGarbageObjects();
    }
    // -------------------------------------------------------------------------
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
