//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// NaN Boxing Value
//-----------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <cstdint>
#include <bit>
#include <cassert>

#include "ValueObject.h"
#include "Globals.h"

namespace DreiZehn{


constexpr uint64_t QNAN_MASK        = 0x7FF8000000000000ULL;
constexpr uint64_t TAG_INT          = 0x0001000000000000ULL; // 1 (binär 001) =>  Integer
constexpr uint64_t TAG_PTR          = 0x0002000000000000ULL; // 2 (binär 010) =>  Pointer
constexpr uint64_t TAG_VALUE_PTR    = 0x0003000000000000ULL; // 3 (binär 011) =>  Pointer to a Value - will be used in byte code
constexpr uint64_t TAG_STRING_ID    = 0x0004000000000000ULL; // 4 (binär 100) =>  StringTable uint32_t Identifier

constexpr uint64_t TAG_DUMMY_5       = 0x0005000000000000ULL; // 5 (binär 101)
constexpr uint64_t TAG_DUMMY_6       = 0x0006000000000000ULL; // 6 (binär 110)
constexpr uint64_t TAG_DUMMY_7       = 0x0007000000000000ULL; // 7 (binär 111)
// ----
constexpr uint64_t TAG_MASK         = 0x0007000000000000ULL; // for type safty - last type

class Value {
private:
    uint64_t mBits; // 8 byte value

    explicit Value(uint64_t b) : mBits(b) {}

public:
    // Default constructor
    Value() : mBits(0) {}

    // -------------------------------------------------------------------------
    Value(double d) {
        mBits = std::bit_cast<uint64_t>(d);
    }
    // -------------------------------------------------------------------------
    Value(bool b) {
        // NaN Mask + Int-Tag + 32-Bit Integers
        mBits = QNAN_MASK | TAG_INT | static_cast<uint32_t>(b);
    }
    // -------------------------------------------------------------------------
    Value(int32_t i) {
        // NaN Mask + Int-Tag + 32-Bit Integers
        mBits = QNAN_MASK | TAG_INT | static_cast<uint32_t>(i);
    }
    // -------------------------------------------------------------------------
    Value(uint32_t i) {
        // NaN Mask + Int-Tag + 32-Bit Integers
        mBits = QNAN_MASK | TAG_INT | i;
    }
    // -------------------------------------------------------------------------
    Value(std::string str) {
        uint32_t strId = StringTable::insert(str);
        mBits = QNAN_MASK | TAG_STRING_ID | strId;
    }
    // -------------------------------------------------------------------------
    Value(ValueObject* obj) {
        uint64_t ptrBits = std::bit_cast<uint64_t>(obj);
        mBits = QNAN_MASK | TAG_PTR | (ptrBits & 0x0000FFFFFFFFFFFFULL);
    }
    // -------------------------------------------------------------------------
    Value(Value* valuePtr) {
        uint64_t ptrBits = std::bit_cast<uint64_t>(valuePtr);
        mBits = QNAN_MASK | TAG_VALUE_PTR| (ptrBits & 0x0000FFFFFFFFFFFFULL);
    }


    // -------------------------------------------------------------------------
    // --- Typ-Check ---
    // -------------------------------------------------------------------------
    inline bool isDouble()  const { return (mBits & QNAN_MASK) != QNAN_MASK; }
    inline bool isInt()     const { return (mBits & (QNAN_MASK | TAG_MASK)) == (QNAN_MASK | TAG_INT); }
    inline bool isPointer() const { return (mBits & (QNAN_MASK | TAG_MASK)) == (QNAN_MASK | TAG_PTR); }
    inline bool isValuePointer() const { return (mBits & (QNAN_MASK | TAG_MASK)) == (QNAN_MASK | TAG_VALUE_PTR); }
    inline bool isStringId() const { return (mBits & (QNAN_MASK | TAG_MASK)) == (QNAN_MASK | TAG_STRING_ID); }


    // inline bool isString() const {
    //     return (isPointer() && asPointerObject()->mType == ValueObjectType::String);
    // }

    inline bool isNumber() {return  isInt() || isDouble();}

    // -------------------------------------------------------------------------
    // --- Getter (Unboxing) ---
    // -------------------------------------------------------------------------
    inline double asDouble() const {
        assert(isDouble());
        return std::bit_cast<double>(mBits);
    }

    // -------------------------------------------------------------------------
    inline int32_t asInt() const {
        assert(isInt());
        return static_cast<int32_t>(mBits & 0xFFFFFFFFULL);
    }
    // -------------------------------------------------------------------------
    inline uint32_t asUInt() const {
        assert(isInt());
        return static_cast<uint32_t>(mBits & 0xFFFFFFFFULL);
    }
    // -------------------------------------------------------------------------
    inline uint32_t asStringId() const {
        assert(isStringId());
        return static_cast<uint32_t>(mBits & 0xFFFFFFFFULL);
    }
    // -------------------------------------------------------------------------
    inline void* asPointer() const {
        assert(isPointer());
        uint64_t ptrBits = mBits & 0x0000FFFFFFFFFFFFULL;
        return std::bit_cast<void*>(ptrBits);
    }

    // -------------------------------------------------------------------------
    inline ValueObject* asPointerObject() const {
        assert(isPointer());
        uint64_t ptrBits = mBits & 0x0000FFFFFFFFFFFFULL;
        return std::bit_cast<ValueObject*>(ptrBits);
    }


    inline Value* asValuePointer() const {
        assert(isValuePointer());
        uint64_t ptrBits = mBits & 0x0000FFFFFFFFFFFFULL;
        return std::bit_cast<Value*>(ptrBits);
    }

    // -------------------------------------------------------------------------
    // --- Getter checking type (Unboxing) ---
    // -------------------------------------------------------------------------
    inline double getDouble() const {
        if (!isDouble()) {
            if (isInt()) return (double) getInt();
            else if (isPointer()) return (double) (asPointer() != nullptr);
            else return 0;
        }
        return std::bit_cast<double>(mBits);
    }

    // -------------------------------------------------------------------------
    inline float getFloat() const {
        if (!isDouble()) {
            if (isInt()) return (float) getInt();
            else if (isPointer()) return (float) (asPointer() != nullptr);
            else return 0;
        }
        return (float)std::bit_cast<double>(mBits);
    }

    // -------------------------------------------------------------------------
    inline int32_t getInt() const {
        if (!isInt()) {
            if (isDouble()) return (int32_t) getDouble();
            else if (isPointer()) return (int32_t)(asPointer() != nullptr);
            else return 0;
        }
        return static_cast<int32_t>(mBits & 0xFFFFFFFFULL);
    }
    // -------------------------------------------------------------------------
    inline uint32_t getUInt() const {
        if (!isInt()) {
            if (isDouble()) return (uint32_t) getDouble();
            else if (isPointer()) return (uint32_t)(asPointer() != nullptr);
            else return 0;
        }
        return static_cast<uint32_t>(mBits & 0xFFFFFFFFULL);
    }
    // -------------------------------------------------------------------------
    inline void* getPointer() const {
        if (!isPointer()) return nullptr;
        uint64_t ptrBits = mBits & 0x0000FFFFFFFFFFFFULL;
        return std::bit_cast<void*>(ptrBits);
    }

    // -------------------------------------------------------------------------
    // by reference
    inline const std::string& getStringRef() {
        static const std::string empty = "";
        if (!isStringId()) return empty;
        return StringTable::get(asStringId());
    }

    inline const std::string getString() {
        if (!isStringId()) return "";
        return StringTable::get(asStringId());
    }
    // -------------------------------------------------------------------------
    // Debug print
    inline void const print(bool appendLineFeed = false) {
        if (isInt()) Tools::printf("%d ", asInt());
        else if (isDouble()) Tools::printf("%f ", asDouble());
        else if (isStringId()) Tools::printf("%s ", getStringRef().c_str());
        else if (isPointer()) {
            ValueObject* obj = asPointerObject();
            Tools::printf("%s [%p] ",  GetObjectTypeName(obj), (void*)obj);
        }
        else if(isValuePointer()) {
            Tools::printf("ValuePtr [%p] ",  asValuePointer());
        }
        if (appendLineFeed) Tools::printf("\n");
    }
    // -------------------------------------------------------------------------

};

} //namespace
