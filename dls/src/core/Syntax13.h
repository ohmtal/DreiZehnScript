//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Lexer / Tokenizer
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <algorithm>

namespace DreiZehn::Syntax13 {

    // TODO make it more modular
    // -----------------------------------------------------------------------------
    // TOOLS
    // -----------------------------------------------------------------------------

    inline bool isValidVariableName(const std::string& str) {
        if (str.empty()) return false; // Handle empty string if needed

        return std::all_of(str.begin(), str.end(), [](unsigned char c) {
            return std::isalnum(c) || c == '_' || c == ':';
        });
    }

}
