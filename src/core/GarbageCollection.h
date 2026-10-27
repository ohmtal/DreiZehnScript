//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// GarbageCollection Singleton
//-----------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <vector>

#ifndef DREIZEHN_INITIAL_GARBAGE_SIZE
#define DREIZEHN_INITIAL_GARBAGE_SIZE 256
#endif

#ifndef DREIZEHN_GARBAGE_CHECK_BORDER
#define DREIZEHN_GARBAGE_CHECK_BORDER 128
#endif

#ifndef DREIZEHN_GARBAGE_AGE_SEC_BORDER
#define DREIZEHN_GARBAGE_AGE_SEC_BORDER 2
#endif

namespace DreiZehn {

// -----------------------------------------------------------------------------
struct ValueObject;
// -----------------------------------------------------------------------------

class GarbageCollection {

    std::vector<ValueObject*> mGarbageCollection;
    bool mLocked = false;
    size_t mInsertCounter = 0;

public:

    inline static void insert(ValueObject* obj) {
        return get().addToGarbageCollection(obj);
    }

    inline static void run(bool isShutDown = false) {
        return get().doGarbageCollection(isShutDown);
    }

    inline static size_t size() {
        return get().getGarbageSize();
    }
    inline static void print() {
        return get().listGarbageObjects();
    }

    inline static void lock() {
       get().mLocked = true;
    }

    inline static void unlock() {
        get().mLocked = false;
    }

    inline static const bool isLocked() {
        return get().mLocked ;
    }

    GarbageCollection() {
         mGarbageCollection.reserve(DREIZEHN_INITIAL_GARBAGE_SIZE);
    }
    ~GarbageCollection() {
        doGarbageCollection(true);
    }

protected:
    // ------------------------------------------------------------------------
    void doGarbageCollection(bool isShutDown);
    void addToGarbageCollection(ValueObject* obj);
    void removeFromGarbageCollection(ValueObject* obj);
    size_t getGarbageSize();
    void listGarbageObjects();

private:

    static GarbageCollection& get() {
        static GarbageCollection instance;
        return instance;
    }

}; //GarbageCollection


struct GarbageCollectionLockGuard {
    GarbageCollectionLockGuard() { GarbageCollection::lock(); }
    ~GarbageCollectionLockGuard() { GarbageCollection::unlock(); }
};

} // namespace DreiZehn
