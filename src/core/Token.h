//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Token
//-----------------------------------------------------------------------------
#pragma once

#include <string>

namespace DreiZehn{

    enum class TokenType {
        Identifier
        , Number
        , Assign
        , StringLiteral
        , LParen
        , RParen
        , LSquare, RSquare
        , LCurly, RCurly
        // math
        , Plus , Minus , Mul , Div

        , PlusPlus, MinusMinus

        , AssignPlus, AssignMinus, AssignMul, AssignDiv

        // if ...
        , If,  Else , ElIf

        , Greater, Less, Equal, NotEqual
        // fn functions
        , Fn, End


        // for iter
        ,For

        // break, return
        , Break, Return
        , Continue

        , Semicolon

        , While
        , Or, And, BitOr, BitAnd
        , BitXOr
        , LowerEqual, GreaterEqual
        , SHL, SHR

        , Arrow, Dot

        , forRange, forEach
        , Not
        , Modulo

        , Question, Colon //short if then

        , Define, Import, Include

        , Global

        , Comma

        , DotDot
        , DotAssign

        , KeyValue // =>

        , NoToken // for peekPrev pos < 1
        , EOFToken
    };

    const char* tokenTypeToString(TokenType type);

    struct Token {
        TokenType mType;
        std::string mValue;
    };

    bool isMathOperatorType(const Token& op);
    bool isMathAssignOperatorType(TokenType tokenType);
    bool isInlineMathType(const TokenType tokenType);
    bool isMathType(const Token& op);
    bool isContinueToken(const Token& op);

}//namespace
