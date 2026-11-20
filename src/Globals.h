//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
#pragma once

#include "toolbox/Tools.h"
#include <vector>
#include <span>
#include <algorithm>
#include <functional>
#include <cstdint>

#if defined(_MSC_VER)
// Microsoft Visual Studio Compiler
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
// GCC, Clang, and Intel Compiler (on Linux/macOS)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
// Fallback for other compilers
#define FORCE_INLINE inline
#endif

namespace DreiZehn {

    // static!
    #ifndef LOCALS_SIZE
    #define LOCALS_SIZE  65536
    #endif

    #ifndef ROOT_LOCALS_OFFSET
    #define ROOT_LOCALS_OFFSET  2048
    #endif
    // ------------------------------------------
    inline std::function<bool()> OnBreath = nullptr;
    inline bool BaseBreath() {
        if (OnBreath) {
            return OnBreath();
        }
        return true;
    }
    // ------------------------------------------

namespace Globals {
    inline bool ByteCodeMode = false;
    inline bool ByteCodeRunning = false;

    // ------
    inline bool shutDownRUNNING = false;
    // ------


#ifdef DREIZEHN_DEBUG
    inline  bool gDumpStateNodes = false; //Debug
    inline  bool gShowVariableDebug = false; //Debug
#endif
}


} //namespace DreiZehn
