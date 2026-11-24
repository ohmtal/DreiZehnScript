//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// GarbageCollection Singleton
//-----------------------------------------------------------------------------
#include <unordered_map>
#include <cassert>
#include <cinttypes>
#include <algorithm>
#include "GarbageCollection.h"
#include "Value.h"
#include "ValueObject.h"

#ifndef DREIZEHN_GARBAGE_AGE_MSSEC_BORDER
#define DREIZEHN_GARBAGE_AGE_MSSEC_BORDER 50
#endif

namespace DreiZehn {


// ------------------------------------------------------------------------
void GarbageCollection::addToGarbageCollection(ValueObject* obj) {

    auto& gcVector = mGarbageCollection;

    // garbage collection border check before new object !!
    mInsertCounter++;
    if (!mLocked && (mInsertCounter > garbage_chech_border)) {
        mInsertCounter = 0;
        doGarbageCollection(false);
    }


    if (gcVector.size() >= gcVector.capacity()) {
        size_t newCapacity = gcVector.capacity() * 2;
        gcVector.reserve(newCapacity);
        garbage_chech_border = size_t( newCapacity / 2);
        // this should be high enough!
        // if (garbage_chech_border > 262144)
        //     garbage_chech_border = 262144;
    }

    gcVector.push_back(obj);
}
// ------------------------------------------------------------------------
void GarbageCollection::removeFromGarbageCollection(ValueObject* obj) {
    auto& gc = mGarbageCollection;
    gc.erase(std::remove(gc.begin(), gc.end(), obj), gc.end());
}
// ------------------------------------------------------------------------
size_t GarbageCollection::getGarbageSize() {
    return mGarbageCollection.size();
}
// ------------------------------------------------------------------------
ValueObject* GarbageCollection::getObjectValueByIndex(size_t index) {
    if (mLocked || index >= mGarbageCollection.size()) {
        Tools::errorf("GarbageCollection is locked or index out of bounds: %u", (U32)index);
        return nullptr;
    }
    return mGarbageCollection[index];
}
// ------------------------------------------------------------------------
void GarbageCollection::listGarbageObjects(bool onlyNotAssigned) {
    Tools::printf("---------------- Garbage Collection ------------------\n");
    size_t countNotAssigned = 0;
    uint64_t curTime = Tools::time_since_start_milliseconds();
    for (size_t index = 0; index < mGarbageCollection.size(); index++) {
        ValueObject* obj = mGarbageCollection[index];
        if (onlyNotAssigned && obj->mAssigned > 0) continue;
        int objtype = obj->mType;
        Tools::printf("#%05u [%p] assigned: %d type:%d %s age:%" PRIu64 "\n"
        , (U32)index, (void*)obj, obj->mAssigned
        , objtype
        , GetObjectTypeName(obj)
        , (curTime - obj->mCreationTime)

        );

        if (obj->mAssigned == 0) countNotAssigned++;
    }
    Tools::printf("*** count is: %zu, not assigned: %zu  ***\n"
            , GarbageCollection::size(), countNotAssigned);
    if (mLocked) Tools::warnf("\n ---- GarbageCollection is locked!!! ----\n");
    Tools::printf("------------------------------------------------------\n");
}
// ------------------------------------------------------------------------

void GarbageCollection::doGarbageCollection(bool isShutDown) {

    if (isShutDown) {
        Globals::shutDownRUNNING = true;
        for (auto* obj : mGarbageCollection) {
            delete obj;
        }

        mGarbageCollection.clear();
    } else {


        uint64_t curTime = Tools::time_since_start_milliseconds();
        for (int i = 0; i < 3; i++) {
            size_t startcount = mGarbageCollection.size();
            std::vector<ValueObject*> byebye;
            std::vector<ValueObject*> muststay;

            for (auto* obj : mGarbageCollection) {
                if (obj->mAssigned < 1 && (curTime -  obj->mCreationTime) > DREIZEHN_GARBAGE_AGE_MSSEC_BORDER ) {
                    byebye.push_back(obj);
                } else {
                    muststay.push_back(obj);
                }
            }
            mGarbageCollection = std::move(muststay);

            for (auto* obj : byebye) {
                delete obj;
            }
            if (mGarbageCollection.size() == startcount) return ;
        }
    }
}

} //namespace
