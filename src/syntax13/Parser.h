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
    // -- Function Vars ---------------
    std::vector<uint32_t> mFunctionVariables;
    bool mBlockInFunction = false;
    std::vector<TokenType> mBlockOpenTypes;
    // -------------------------------------------------------------------------
    bool isFunctionVar(uint32_t symId);
    void insertFunctionVar(uint32_t symId);
    bool guessGlobalVarAndInsertLocalIfNot(Environment& env, uint32_t symId);
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

    //TEST THIS SUCKS <<  to use ()!!
    // inline bool checkCommaBreak(std::vector<std::unique_ptr<Expression>>& args) {
    //     if (peek().mType == TokenType::Comma) {
    //         advance();
    //
    //         // NOTE:
    //         // i got it working with the nullptr args hack but if you cant use it to
    //         // call a function with parameter inside a function call so you still need
    //         // to encapsulate with ()
    //         /*
    //          a = 47; print core::date, a, "jiji"
    //         */
    //         if (args.empty()) return false; // never ?!
    //         Expression* last = args.back().get();
    //         if (!last) return true;
    //     }
    //     return false;
    // }

    // -------------------------------------------------------------------------
    // 0.6 added short if/then
    std::unique_ptr<Expression> parseMath(int minPrecedence = 0);

    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseIdentifier();


    std::istream* mCurrentCodeStream;
public:
    U32 lineNumber = 0;

    Parser(Environment& env, TokenVector t, std::istream* in) : mTokens(std::move(t)) {
        mCurrentCodeStream = in;
        mEnvPtr = &env;
    }

    // std::vector<std::unique_ptr<ASTNode>> parse();
    std::shared_ptr<BlockStatement> parse();
    std::unique_ptr<ASTNode> parseStatement();


    std::string getLine(int byLine = -1);
    void PrintParseError(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");
    void PrintParseInfo(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");
};
} //namespace
