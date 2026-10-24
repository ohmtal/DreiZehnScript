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



enum class ValueType : uint8_t {
    Double,
    Int,
    Pointer,
    ValuePointer,
    StringId,
    ANY,
    NONE
};




constexpr uint64_t QNAN_MASK        = 0x7FF8000000000000ULL;
constexpr uint64_t TAG_INT          = 0x0001000000000000ULL; // 1 (binary 001) =>  Integer
constexpr uint64_t TAG_PTR          = 0x0002000000000000ULL; // 2 (binary 010) =>  Pointer
constexpr uint64_t TAG_VALUE_PTR    = 0x0003000000000000ULL; // 3 (binary 011) =>  Pointer to a Value - will be used in byte code
constexpr uint64_t TAG_STRING_ID    = 0x0004000000000000ULL; // 4 (binary 100) =>  StringTable uint32_t Identifier

constexpr uint64_t TAG_DUMMY_5       = 0x0005000000000000ULL; // 5 (binary 101)
constexpr uint64_t TAG_DUMMY_6       = 0x0006000000000000ULL; // 6 (binary 110)
constexpr uint64_t TAG_DUMMY_7       = 0x0007000000000000ULL; // 7 (binary 111)
// ----
constexpr uint64_t TAG_MASK         = 0x0007000000000000ULL; // for type safty - last type

constexpr uint64_t QNAN_AND_TAG_MASK = 0x7FFF000000000000ULL; // ==> QNAN_MASK | TAG_MASK


struct Unsafe { };

class Value {
private:
    uint64_t mBits; // 8 byte value

    explicit Value(uint64_t b) : mBits(b) {}

    std::string toStringSlowPath() const;

public:
    // Default constructor
    Value() : mBits(0) {}
    // // // // // Value() : mBits(QNAN_MASK | TAG_INT) {}

    // -------------------------------------------------------------------------
    inline Value(Unsafe, uint64_t rawBits) noexcept : mBits(rawBits) {}
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
    inline bool isInt()     const { return (mBits & QNAN_AND_TAG_MASK) == (QNAN_MASK | TAG_INT); }
    inline bool isPointer() const { return (mBits & QNAN_AND_TAG_MASK) == (QNAN_MASK | TAG_PTR); }
    inline bool isValuePointer() const { return (mBits & QNAN_AND_TAG_MASK) == (QNAN_MASK | TAG_VALUE_PTR); }
    inline bool isString() const { return (mBits & QNAN_AND_TAG_MASK) == (QNAN_MASK | TAG_STRING_ID); }

    inline ValueType getType() const {
        if ((mBits & QNAN_MASK) != QNAN_MASK) {
            return ValueType::Double;
        }
        uint64_t tag = mBits & TAG_MASK;

        switch (tag) {
            case TAG_INT:          return ValueType::Int;
            case TAG_PTR:          return ValueType::Pointer;
            case TAG_VALUE_PTR:    return ValueType::ValuePointer;
            case TAG_STRING_ID:    return ValueType::StringId;
            default:
                return ValueType::NONE;
        }
    }


    // inline bool isString() const {
    //     return (isPointer() && asPointerObject()->mType == ValueObjectType::String);
    // }

    inline bool isNumber() {ValueType t = getType(); return (t == ValueType::Int || t == ValueType::Double);}

    // -------------------------------------------------------------------------
    // --- Getter (Unboxing) ---
    // -------------------------------------------------------------------------
    inline double asFastDouble() const {
        return std::bit_cast<double>(mBits);
    }
    // -------------------------------------------------------------------------
    inline double asDouble() const {
        assert(isDouble());
        return std::bit_cast<double>(mBits);
    }
    // -------------------------------------------------------------------------
    inline int32_t asFastInt() const {
        return static_cast<int32_t>(mBits & 0xFFFFFFFFULL);
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
    inline uint32_t asString() const {
        assert(isString());
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
    // -------------------------------------------------------------------------
    inline double getDouble() const {
        if (isDouble()) [[likely]] {
            return std::bit_cast<double>(mBits);
        }
        if (isInt())      return static_cast<double>(asFastInt());
        if (isPointer())  return static_cast<double>(asPointer() != nullptr);
        if (isString())   return static_cast<double>(!getStringRef().empty());
        return 0.0;
    }

    // -------------------------------------------------------------------------
    inline float getFloat() const {
        return static_cast<float>(getDouble());
    }

    // -------------------------------------------------------------------------
    inline int64_t getInt64() const {
        if (isInt()) [[likely]] {
            return static_cast<int64_t>(asFastInt());
        }
        if (isDouble())   return static_cast<int64_t>(asFastDouble());
        if (isPointer())  return reinterpret_cast<int64_t>(asPointer());
        if (isString())   return static_cast<int64_t>(!getStringRef().empty());
        return 0;
    }

    // -------------------------------------------------------------------------
    inline int32_t getInt() const {
        if (isInt()) [[likely]] {
            return static_cast<int32_t>(mBits & 0xFFFFFFFFULL);
        }
        if (isDouble())   return static_cast<int32_t>(asFastDouble());
        if (isPointer())  return static_cast<int32_t>(asPointer() != nullptr);
        if (isString())   return static_cast<int32_t>(!getStringRef().empty());
        return 0;
    }

    // -------------------------------------------------------------------------
    inline uint32_t getUInt() const {
        return static_cast<uint32_t>(getInt());
    }

    // -------------------------------------------------------------------------
    inline bool getBool() const {
        if (isInt()) [[likely]] {
            return (mBits & 0xFFFFFFFFULL) != 0;
        }
        if (isDouble())   return asFastDouble() != 0.0;
        if (isPointer())  return asPointer() != nullptr;
        if (isString())   return !getStringRef().empty();
        return false;
    }

    // // inline double getDouble() const {
    // //     if (!isDouble()) {
    // //         if (isInt()) return (double) asFastInt();
    // //         else if (isPointer()) return (double) (asPointer() != nullptr);
    // //         else if (isString()) return (double)( getStringRef().length() > 0  );
    // //         else return 0;
    // //     }
    // //     return std::bit_cast<double>(mBits);
    // // }
    // //
    // // // -------------------------------------------------------------------------
    // // inline float getFloat() const {
    // //     if (!isDouble()) {
    // //         if (isInt()) return (float) asFastInt();
    // //         else if (isPointer()) return (float) (asPointer() != nullptr);
    // //         else if (isString()) return (float)( getStringRef().length() > 0  );
    // //         else return 0;
    // //     }
    // //     return (float)std::bit_cast<double>(mBits);
    // // }
    // // // -------------------------------------------------------------------------
    // // inline int64_t getInt64() const {
    // //
    // //     if (isDouble()) return static_cast<int64_t>(asFastDouble());
    // //     else if (isInt()) return (int64_t)(asFastInt());
    // //     else if (isPointer()) return (int64_t)(asPointer());
    // //     else if (isString()) return (int64_t)( getStringRef().length() > 0  );
    // //     return 0;
    // // }
    // // // -------------------------------------------------------------------------
    // // inline int32_t getInt() const {
    // //     if (!isInt()) {
    // //         if (isDouble()) return (int32_t) asFastDouble();
    // //         else if (isPointer()) return (int32_t)(asPointer() != nullptr);
    // //         else if (isString()) return (int32_t)( getStringRef().length() > 0  );
    // //         else return 0;
    // //     }
    // //     return static_cast<int32_t>(mBits & 0xFFFFFFFFULL);
    // // }
    // // // -------------------------------------------------------------------------
    // // inline uint32_t getUInt() const {
    // //     if (!isInt()) {
    // //         if (isDouble()) return (uint32_t) asFastDouble();
    // //         else if (isPointer()) return (uint32_t)(asPointer() != nullptr);
    // //         else if (isString()) return (uint32_t)( getStringRef().length() > 0  );
    // //         else return 0;
    // //     }
    // //     return static_cast<uint32_t>(mBits & 0xFFFFFFFFULL);
    // // }
    // // // -------------------------------------------------------------------------
    // // inline bool getBool() const {
    // //     if (!isInt()) {
    // //         if (isDouble()) return ((uint32_t) asFastDouble() == 1);
    // //         else if (isPointer()) return (uint32_t)(asPointer() != nullptr);
    // //         else if (isString()) return (uint32_t)( getStringRef().length() > 0  );
    // //         else return 0;
    // //     }
    // //     return (static_cast<uint32_t>(mBits & 0xFFFFFFFFULL) == 1);
    // // }

    // -------------------------------------------------------------------------
    inline void* getPointer() const {
        if (!isPointer()) return nullptr;
        uint64_t ptrBits = mBits & 0x0000FFFFFFFFFFFFULL;
        return std::bit_cast<void*>(ptrBits);
    }

    // -------------------------------------------------------------------------
    // by reference
    inline const std::string& getStringRef() const {
        static const std::string empty = "";
        if (!isString()) return empty;
        return StringTable::get(asString());
    }
    inline const char * getStringChars() {
        if (!isString()) return "";
        return StringTable::get(asString()).c_str();
    }


    inline const std::string getString() {
        if (!isString()) return "";
        return StringTable::get(asString());
    }
    // -------------------------------------------------------------------------
    // Debug print
    // inline const std::string toString() const {
    //     char buffer[256];
    //
    //     if (isInt()) {
    //         snprintf(buffer, sizeof(buffer), "%d" , asInt());
    //         return std::string(buffer);
    //     }
    //     else if (isDouble()) {
    //         snprintf(buffer, sizeof(buffer), "%f" , asDouble());
    //         return std::string(buffer);
    //     }
    //     else if (isString()){
    //         return getStringRef();
    //     }
    //     else if (isPointer()) {
    //         ValueObject* obj = asPointerObject();
    //         return obj->toString();
    //     }
    //     else if(isValuePointer()) {
    //         snprintf(buffer, sizeof(buffer), "[%p]" , asValuePointer());
    //         return std::string(buffer);
    //     }
    //     return "unknown!";
    // }

    inline std::string toString() const {
        if (isString()) [[likely]] {
            return getStringRef();
        }
        return toStringSlowPath();
    }



    // -------------------------------------------------------------------------
    void const print(bool appendLineFeed = false);
    // Debug getType
    std::string const getTypeName();
    static Value handleGenericAdd(const Value& a, const Value& b);
};



    static const Value EmptyStringValue = Value(std::string(""));
    static const Value NumberNullValue = Value(0);



} //namespace
