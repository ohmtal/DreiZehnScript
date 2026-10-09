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
#define DREIZEHN_INITIAL_GARBAGE_SIZE 2048
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
    void addToGarbageCollection(ValueObject* obj);
    void removeFromGarbageCollection(ValueObject* obj);
    size_t getGarbageSize();
    void listGarbageObjects();
    void doGarbageCollection(bool isShutDown);

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
