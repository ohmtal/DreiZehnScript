//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Basic SDL3 Bindings
//-----------------------------------------------------------------------------
#pragma once


namespace DreiZehn {
    class Environment;
    namespace Raylib {
        void RegisterRaylibFunctions(Environment& env);
    }
}
