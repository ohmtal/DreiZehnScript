//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <fstream>
#include <istream>

namespace DreiZehn {
    class Environment;
    std::shared_ptr<BlockStatement> ParseScriptToAST(std::istream& stream);
    bool RunScriptStream(std::istream& stream, Environment& env);
    Value RunScriptStreamByteCode(std::istream& stream, Environment& env, bool isCodelet = false);

    bool RunScriptFile(const std::string& filename, Environment& env, bool useByteCode);

    std::shared_ptr<BlockStatement> IncludeScriptFile(const std::string& filename);
}
