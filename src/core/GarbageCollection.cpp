//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// GarbageCollection Singleton
//-----------------------------------------------------------------------------
#include <unordered_map>
#include <cassert>
#include <cassert>
#include <algorithm>
#include "GarbageCollection.h"
#include "Value.h"
#include "ValueObject.h"


namespace DreiZehn {


// ------------------------------------------------------------------------
void GarbageCollection::addToGarbageCollection(ValueObject* obj) {

    auto& gcVector = mGarbageCollection;

    // garbage collection border check before new object !!
    mInsertCounter++;
    if (!mLocked && (mInsertCounter > DREIZEHN_GARBAGE_CHECK_BORDER)) {
        mInsertCounter = 0;
        doGarbageCollection(false);
    }


    if (gcVector.size() >= gcVector.capacity()) {
        size_t newCapacity = gcVector.capacity() * 2;
        gcVector.reserve(newCapacity);
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
void GarbageCollection::listGarbageObjects() {
    Tools::printf("---------------- Garbage Collection ------------------\n");
    size_t countNotAssigned = 0;
    int i = 0;
    for (auto* obj : mGarbageCollection) {
        int objtype = obj->mType;
        Tools::printf("#%d [%p] assigned: %d type:%d %s\n"
        , i, (void*)obj, obj->mAssigned
        , objtype, GetObjectTypeName(obj));

        if (obj->mAssigned == 0) countNotAssigned++;
        i++;
    }
    Tools::printf("*** count is: %zu, not assigned: %zu ***\n", GarbageCollection::size(), countNotAssigned);
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


        uint64_t curTime = Tools::time_seconds();
        for (int i = 0; i < 3; i++) {
            size_t startcount = mGarbageCollection.size();
            std::vector<ValueObject*> byebye;
            std::vector<ValueObject*> muststay;

            for (auto* obj : mGarbageCollection) {
                if (obj->mAssigned < 1 && (curTime -  obj->mCreationTime) > DREIZEHN_GARBAGE_AGE_SEC_BORDER ) {
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
