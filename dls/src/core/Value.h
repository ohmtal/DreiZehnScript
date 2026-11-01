//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value (union)
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <memory>
#include <format>
#include <vector>
#include "Types.h"
#include "toolbox/SymbolTable.h"

namespace DLS {

    // ------------------------------------------
    // Value
    // ------------------------------------------
    enum class ValueType: uint8_t {
        None,
        Double,
        Long,
        Pointer,
        String,
        Function,
        Array
    };
    //
    struct alignas(16) Value {
        union {
            F64 d;
            S64 l;
            void* p;
            U32 strId;

        } ;
        ValueType type; // 1 byte
        U8  padding[7]; // 7 bytes padding for future use

        // ------------------
        Value() { type = ValueType::None;  }
        Value(F64 val) { type = ValueType::Double; d = val; }
        Value(S32 val) { type = ValueType::Long; l = (S64)val; }
        Value(U32 val) { type = ValueType::Long; l = (S64)val; }
        Value(S64 val) { type = ValueType::Long; l = val; }
        Value(std::string val) { type = ValueType::String; strId = StringTable::insert(val); }
        Value(void* val) { type = ValueType::Pointer; p = val; }
        // ------------------
        inline bool isDouble() { return type == ValueType::Double; }
        inline bool isFloat() { return type == ValueType::Double; }
        inline bool isLong() { return type == ValueType::Long; }
        inline bool isInt() { return type == ValueType::Long; }
        inline bool isPointer() { return type == ValueType::Pointer; }
        inline bool isString() { return type == ValueType::String; }
        inline bool isFunction() { return type == ValueType::Function; }
        inline bool isArray() { return type == ValueType::Array; }
        // ------------------
        inline F64 getDouble() {
            if (type == ValueType::Double) [[likely]] return d;
            else if (type == ValueType::Long) return static_cast<F64>(l);
            else if (type == ValueType::Array) return static_cast<F64>(l);
            else if (type == ValueType::Pointer) return (p != nullptr) ? 1.0 : 0.0;
            else return 0.0;
        }
        // ------------------
       inline S64 getLong() {
            if (type == ValueType::Long) [[likely]] return l;
            else if (type == ValueType::Array)  return l;
            else if (type == ValueType::Double)  return static_cast<S64>(d);
            else if (type == ValueType::Pointer) return (p != nullptr) ? 1 : 0;
            else return 0;
        }
        // ------------------
        inline F32 getFloat() { return (F32) getDouble(); }
        inline U32 getUInt()  { return (U32) getLong(); }
        inline S32 getInt()   { return (S32) getLong(); }
        inline bool getBool() { return  getLong() != 0; }
        // ------------------
        inline const std::string& getString() const {
            if (type == ValueType::String) [[likely]] return StringTable::get(strId);
            static const std::string empty = "";
            return empty;
        }
        // ------------------
        inline void * getPointer() const {
            if (type == ValueType::Pointer) [[likely]] return p;
            else return nullptr;
        }

        // ------------------
        inline const std::string toString() const {
            switch (type) {
                case ValueType::Double: return std::to_string(d);
                case ValueType::Long: return std::to_string(l);
                case ValueType::String: return getString();
                case ValueType::Pointer: return  std::format("{:p}", static_cast<void*>(p));
                case ValueType::Array: return  "*Array*";
                case ValueType::Function: return  "*Function*";
                default:
                    static const std::string empty = "";
                    return empty;
            }
        }


    }; //Value

} //namespace

