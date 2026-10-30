//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value (union)
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "Types.h"
#include "toolbox/SymbolTable.h"

namespace DLS {

    // ------------------------------------------
    // Value
    // ------------------------------------------
    enum class ValueType{
        None,
        Double,
        Long,
        Ptr,
        String
    };
    //
    // alignas(16)
    struct Value {
        union {
            F64 d;
            S64 l;
            void* p;
            U32 strId;

        } ;
        ValueType type;
        U8 padding[3];
        // ------------------
        Value() { type = ValueType::None; l = 0; }
        Value(F64 val) { type = ValueType::Double; d = val; }
        Value(S32 val) { type = ValueType::Long; l = (S64)val; }
        Value(U32 val) { type = ValueType::Long; l = (S64)val; }
        Value(S64 val) { type = ValueType::Long; l = val; }
        Value(std::string val) { type = ValueType::String; strId = StringTable::insert(val); }
        // ------------------
        inline bool isPointer() {
            return type == ValueType::Ptr;
        }
        // ------------------
        inline F64 getDouble() {
            if (type == ValueType::Double) [[likely]] return d;
            else if (type == ValueType::Long) return static_cast<F64>(l);
            else return 0.0;
        }
        // ------------------
       inline S64 getLong() {
            if (type == ValueType::Long) [[likely]] return l;
            if (type == ValueType::Double)  return static_cast<S64>(d);
            else return 0;
        }
        // ------------------
        inline U32 getUInt() {
            if (type == ValueType::Long) [[likely]] return static_cast<U32>(l);
            if (type == ValueType::Double)  return static_cast<U32>(d);
            else return 0;
        }
        // ------------------
        inline S32 getInt() {
            if (type == ValueType::Long) [[likely]] return static_cast<S32>(l);
            if (type == ValueType::Double)  return static_cast<S32>(d);
            else return 0;
        }
        // ------------------
        inline const std::string& getString() {
            if (type == ValueType::String) [[likely]] return StringTable::get(strId);
            static const std::string empty = "";
            return empty;
        }

        // ------------------
        inline const std::string toString() {
            switch (type) {
                case ValueType::Double: return std::to_string(d);
                case ValueType::Long: return std::to_string(l);
                case ValueType::String: return getString();
                default:
                    static const std::string empty = "";
                    return empty;
            }
        }

    }; //Value

} //namespace

