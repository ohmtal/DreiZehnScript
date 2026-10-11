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
    int i = 0;
    for (auto* obj : mGarbageCollection) {
        int objtype = obj->mType;
        Tools::printf("#%d [%p] assigned: %d type:%d %s\n"
        , i, (void*)obj, obj->mAssigned
        , objtype, GetObjectTypeName(obj));

        i++;
    }
    if (mLocked) Tools::warnf("\n ---- GarbageCollection is locked!!! ----\n");
}
// ------------------------------------------------------------------------

void GarbageCollection::doGarbageCollection(bool isShutDown) {

    if (isShutDown) {
        for (auto* obj : mGarbageCollection) {
            delete obj;
        }
        mGarbageCollection.clear();
    } else {

        auto it = std::remove_if(mGarbageCollection.begin()
        , mGarbageCollection.end(), [](auto* obj) {
            if (obj->mAssigned < 1) {
                delete obj;
                return true; // mark for delete
            }
            return false;
        });

        mGarbageCollection.erase(it, mGarbageCollection.end());
    }
}

} //namespace
