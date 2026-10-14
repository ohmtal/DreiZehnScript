//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Parser
//-----------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

#include "Lexer.h"
#include "AstNode.h"

namespace DreiZehn{

class Parser {
private:
    std::vector<Token> mTokens;
    size_t mPos = 0;

    Token peek() { return mTokens[mPos]; }
    Token peekNext() { if (mPos + 1 < mTokens.size()) return mTokens[mPos+1]; else return Token(TokenType::EOFToken); }
    Token peekNextNext() { if (mPos + 2 < mTokens.size()) return mTokens[mPos+2]; else return Token(TokenType::EOFToken); }
    Token peekOffset(int32_t offset) {
        int32_t tmpPeek = mPos + offset;
        if (tmpPeek < mTokens.size() && tmpPeek >= 0 )
            return mTokens[tmpPeek]; else return Token(TokenType::EOFToken);
    }
    Token peekPrev() { if (mPos > 1) return mTokens[mPos-1]; else return Token(TokenType::NoToken); }
    Token advance() { if (mPos + 1 < mTokens.size()) return mTokens[mPos++]; else return Token(TokenType::EOFToken);}

    // -------------------------------------------------------------------------

    int getPrecedence(TokenType type);

    // -------------------------------------------------------------------------
    std::unique_ptr<Expression> parsePrimary();

    //TEST
    inline bool checkCommaBreak(std::vector<std::unique_ptr<Expression>>& args) {
        if (peek().mType == TokenType::Comma) {
            advance();

            // NOTE:
            // i got it working with the nullptr args hack but if you cant use it to
            // call a function with parameter inside a function call so you still need
            // to encapsulate with ()
            /*
             a = 47; print core::date, a, "jiji"
            */
            if (args.empty()) return false; // never ?!
            Expression* last = args.back().get();
            if (!last) return true;
        }
        return false;
    }

    // -------------------------------------------------------------------------
    // 0.6 added short if/then
    std::unique_ptr<Expression> parseMath(int minPrecedence = 0);

    // -------------------------------------------------------------------------
    std::unique_ptr<Expression> parseComparison();
    // -------------------------------------------------------------------------
    std::unique_ptr<Expression> parseExpression();
    // -------------------------------------------------------------------------
public:
    Parser(std::vector<Token> t) : mTokens(std::move(t)) {}

    std::vector<std::unique_ptr<ASTNode>> parseStatements();
    std::unique_ptr<ASTNode> parseLine();

};
} //namespace
