//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <span>
#include <cinttypes>

namespace DreiZehn {
    // ------------------------------------------
    // lazy types
    // ------------------------------------------
    typedef uint8_t  U8;
    typedef uint16_t U16;
    typedef uint32_t U32;
    typedef int32_t  S32;
    typedef float    F32;
    typedef int64_t  S64;
    typedef double   F64;

    struct Value;
    using ValueVector = std::vector<Value>;
    using ValueSpan   = std::span<Value>;

    struct Token;
    using TokenVector = std::vector<Token>;

    // ------------------------------------------

} //namespace DreiZehn
