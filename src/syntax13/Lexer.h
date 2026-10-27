//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Lexer / Tokenizer
//-----------------------------------------------------------------------------
#pragma once

#include <string>

#include <vector>
#include <cctype>
#include "toolbox/Tools.h"
#include "core/Token.h"

namespace DreiZehn {

class Lexer {
private:
    std::string mSrc;
    int32_t mPos = 0;

    char peek() { return mPos < mSrc.size() ? mSrc[mPos] : '\0'; }
    char peekNext() { return mPos+1 < mSrc.size() ? mSrc[mPos + 1] : '\0'; }
    char peekOffset(int32_t offset) {
        int32_t tmpPeek = mPos + offset;
        if (tmpPeek < mSrc.size() && tmpPeek >= 0 )
            return mSrc[tmpPeek]; else return '\0';
    }
    char peekPrev() { return mPos-1 > 0  ? mSrc[mPos - 1] : '\0'; }
    char advance() { return mPos < mSrc.size() ? mSrc[mPos++] : '\0'; }

public:
    Lexer(std::string source) : mSrc(std::move(source)) {}

    std::vector<Token> tokenize();


};
} //Namespace
