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

enum class ValueType { Double, Int, Pointer, Undefined};

constexpr uint64_t QNAN_MASK = 0x7FF8000000000000ULL;
constexpr uint64_t TAG_INT   = 0x0001000000000000ULL; // Integer
constexpr uint64_t TAG_PTR   = 0x0002000000000000ULL; // Pointer

class Value {
private:
    uint64_t mBits; // 8 byte value

    explicit Value(uint64_t b) : mBits(b) {}

public:
    // Default constructor
    Value() : mBits(0) {}

    Value(double d) {
        mBits = std::bit_cast<uint64_t>(d);
    }

    Value(int32_t i) {
        // NaN Mask + Int-Tag + 32-Bit Integers
        mBits = QNAN_MASK | TAG_INT | static_cast<uint32_t>(i);
    }

    Value(ValueObject* obj) {
        uint64_t ptrBits = std::bit_cast<uint64_t>(obj);
        mBits = QNAN_MASK | TAG_PTR | (ptrBits & 0x0000FFFFFFFFFFFFULL);
    }

    // --- Typ-Check ---
    inline bool isDouble()  const { return (mBits & QNAN_MASK) != QNAN_MASK; }
    inline bool isInt()     const { return (mBits & (QNAN_MASK | TAG_INT)) == (QNAN_MASK | TAG_INT); }
    inline bool isPointer() const { return (mBits & (QNAN_MASK | TAG_PTR)) == (QNAN_MASK | TAG_PTR); }

    inline bool isNumber() {return  isInt() || isDouble();}

    // --- Getter (Unboxing) ---
    inline double asDouble() const {
        assert(isDouble());
        return std::bit_cast<double>(mBits);
    }

    inline int32_t asInt() const {
        assert(isInt());
        return static_cast<int32_t>(mBits & 0xFFFFFFFFULL);
    }

    inline void* asPointer() const {
        assert(isPointer());
        uint64_t ptrBits = mBits & 0x0000FFFFFFFFFFFFULL;
        return std::bit_cast<void*>(ptrBits);
    }

    inline ValueObject* asPointerObject() const {
        assert(isPointer());
        uint64_t ptrBits = mBits & 0x0000FFFFFFFFFFFFULL;
        return std::bit_cast<ValueObject*>(ptrBits);
    }

    // --- Getter checking type (Unboxing) ---
    inline double getDouble() const {
        if (!isDouble()) {
            if (isInt()) return (double) getInt();
            else if (isPointer()) return (double) (asPointer() != nullptr);
            else return 0;
        }
        return std::bit_cast<double>(mBits);
    }

    inline float getFloat() const {
        if (!isDouble()) {
            if (isInt()) return (float) getInt();
            else if (isPointer()) return (float) (asPointer() != nullptr);
            else return 0;
        }
        return (float)std::bit_cast<double>(mBits);
    }

    inline int32_t getInt() const {
        if (!isInt()) {
            if (isDouble()) return (int32_t) getDouble();
            else if (isPointer()) return (int32_t)(asPointer() != nullptr);
            else return 0;
        }
        return static_cast<int32_t>(mBits & 0xFFFFFFFFULL);
    }

    inline void* getPointer() const {
        if (!isPointer()) return nullptr;
        uint64_t ptrBits = mBits & 0x0000FFFFFFFFFFFFULL;
        return std::bit_cast<void*>(ptrBits);
    }

    inline StringValueObject* getStringObj() {
        if (this->isPointer()) {
            auto* obj = static_cast<ValueObject*>(this->asPointer());
            if (obj->mType == ValueObjectType::String) {
                auto* strObj = static_cast<StringValueObject*>(obj);
                return strObj;
            }
        }
        return nullptr;
    }

};

} //namespace
