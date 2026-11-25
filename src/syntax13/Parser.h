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
#include "core/AstNode.h"

namespace DreiZehn{

struct ParserFrame {
    Environment* mEnvPtr = nullptr;
    // -- Function Vars ---------------
    std::vector<std::vector<uint32_t>> mLocalVariables;
    bool mBlockInFunction = false;
    std::vector<TokenType> mBlockOpenTypes;
    // -------------------------------------------------------------------------
    int64_t findLocalVar(uint32_t symId) const;
    U32 insertLocalVar(uint32_t symId);
    S32 guessGlobalVar(Environment& env, uint32_t symId);
    void startFunc(uint32_t funcSymId);
    void endFunc(uint32_t funcSymId);
}; // ParserFrame


class Parser {
private:
    TokenVector mTokens;
    size_t mPos = 0;
    Environment* mEnvPtr;

    ParserFrame parserFrame;

    // ------------------------------

    Token peek(int offset = 0) {
        S64 checkOffset = (S64)mPos + offset;
        if (checkOffset < 0) return Token(TokenType::NoToken);
        if (checkOffset >= mTokens.size()) return Token(TokenType::EOFToken);
        return mTokens[(size_t) checkOffset];
    }

    // Token peekNext() { if (mPos + 1 < mTokens.size()) return mTokens[mPos+1]; else return Token(TokenType::EOFToken); }
    Token peekNext() { return peek(1); }
    // Token peekNextNext() { if (mPos + 2 < mTokens.size()) return mTokens[mPos+2]; else return Token(TokenType::EOFToken); }
    Token peekNextNext() { return peek(2); }
    Token peekOffset(int32_t offset) { return peek(offset); }
    // Token peekPrev() { if (mPos > 1) return mTokens[mPos-1]; else return Token(TokenType::NoToken); }
    Token peekPrev() { return peek(-1); }
    Token advance() { if (mPos + 1 < mTokens.size()) return mTokens[mPos++]; else return Token(TokenType::EOFToken);}

    // -------------------------------------------------------------------------

    int getPrecedence(TokenType type);

    // -------------------------------------------------------------------------
    std::unique_ptr<Expression> parsePrimary();

    //TEST THIS SUCKS <<  to use ()!!
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

    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parsePostFix(std::unique_ptr<Expression> currentExpression); // [] -> .


    std::istream* mCurrentCodeStream;
public:
    U32 lineNumber = 0;

    Parser(Environment& env, TokenVector t, std::istream* in) : mTokens(std::move(t)) {
        mCurrentCodeStream = in;
        mEnvPtr = &env;
        parserFrame.mEnvPtr = mEnvPtr;
    }

    // std::vector<std::unique_ptr<ASTNode>> parse();
    std::shared_ptr<BlockStatement> parse();
    std::unique_ptr<ASTNode> parseStatement();


    std::string getLine(int byLine = -1);
    void PrintParseError(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");
    void PrintParseWarning(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");
    void PrintParseInfo(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");
};
} //namespace
