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

size_t getNextGcPrimeSize(size_t currentCapacity) {
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
// ------------------------------------------------------------------------
void GarbageCollection::addToGarbageCollection(ValueObject* obj) {

    auto& gcVector = mGarbageCollection;

    // garbage collection border check before new object !!
    mInsertCounter++;
    if (!mLocked && mInsertCounter > (gcVector.capacity() / 2)) {
        mInsertCounter = 0;
        doGarbageCollection(false);
    }


    if (gcVector.size() >= gcVector.capacity()) {
        size_t newCapacity = getNextGcPrimeSize(gcVector.capacity());
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
        for (int i = 0; i < 3; i++) {
            if (mGarbageCollection.size() < 256) return ;
            std::vector<ValueObject*> byebye;
            std::vector<ValueObject*> muststay;

            for (auto* obj : mGarbageCollection) {
                if (obj->mAssigned < 1) {
                    byebye.push_back(obj);
                } else {
                    muststay.push_back(obj);
                }
            }
            mGarbageCollection = std::move(muststay);

            for (auto* obj : byebye) {
                delete obj;
            }
        }
    }
}

} //namespace
