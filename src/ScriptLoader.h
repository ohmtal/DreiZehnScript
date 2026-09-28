//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

namespace DreiZehn {
    class Environment;
    bool RunScriptStream(std::istream& stream, Environment& env);
    bool RunScriptFile(const std::string& filename, Environment& env);
}
