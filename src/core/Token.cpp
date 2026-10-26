//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Token
//-----------------------------------------------------------------------------
#include "Token.h"

namespace DreiZehn {


const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Identifier:    return "Identifier";
        case TokenType::Number:        return "Number";
        case TokenType::Assign:        return "Assign";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::LParen:        return "LParen (";
        case TokenType::RParen:        return "RParen )";
        case TokenType::LSquare:        return "LSquare [";
        case TokenType::RSquare:        return "RSquare ]";
        // case TokenType::LCurly:        return "LCurly {";
        // case TokenType::RCurly:        return "RCurly }";

        // math
        case TokenType::Plus:          return "Plus";
        case TokenType::PlusPlus:      return "PlusPlus";
        case TokenType::MinusMinus:    return "MinusMinus";
        case TokenType::Minus:         return "Minus";
        case TokenType::Mul:           return "Mul";
        case TokenType::Div:           return "Div";
        case TokenType::Modulo:        return "Modulo";

        case TokenType::AssignPlus:    return "AssignPlus";
        case TokenType::AssignMinus:   return "AssignMinus";
        case TokenType::AssignMul:     return "AssignMul";
        case TokenType::AssignDiv:     return "AssignDiv";
        // if ...
        case TokenType::If:            return "If";
        case TokenType::Greater:       return "Greater";
        case TokenType::Less:          return "Less";
        case TokenType::Equal:         return "Equal";
        case TokenType::NotEqual:      return "Not Equal";
        case TokenType::Else:          return "Else";
        case TokenType::ElIf:          return "ElIf";

        // fn functions
        case TokenType::Fn:            return "Fn";
        case TokenType::End:           return "End";

        // for / while iter
        case TokenType::For:           return "For";
        case TokenType::While:         return "While";

        // break, return
        case TokenType::Break:         return "Break";
        case TokenType::Return:        return "Return";
        case TokenType::Continue:      return "Continue";

        case TokenType::Or:            return "OR ||";
        case TokenType::BitOr:         return "Bit |";
        case TokenType::BitXOr:           return "Bit ^";
        case TokenType::BitAnd:         return "Bit &";
        case TokenType::And:            return "AND &&";
        case TokenType::LowerEqual:     return "LowerEqual";
        case TokenType::GreaterEqual:   return "GreaterEqual";
        case TokenType::SHL:            return "Shift Left <<";
        case TokenType::SHR:            return "Shift Right >>";

        case TokenType::Semicolon:      return "Semicolon";

        case TokenType::Arrow:          return "Arrow Object Method call";
        case TokenType::Dot:            return "Dot Object field access";

        case TokenType::forRange:       return "forRange";
        case TokenType::forEach:        return "forEach";
        case TokenType::Not:            return "Not";

        case TokenType::Question:       return "Question";
        case TokenType::Colon:          return "Colon";

        case TokenType::Import:         return "Import";
        case TokenType::Include:        return "Include";
        case TokenType::Define:         return "Define";
        case TokenType::Global:         return "Global";
        case TokenType::Comma:          return "Comma";
        case TokenType::KeyValue:       return "KeyValue";
        case TokenType::DotDot:         return "DotDot"; //String concat
        case TokenType::DotAssign:      return "DotAssign"; //String concat

        case TokenType::EOFToken:       return "EOFToken";

        default:                        return "UnknownToken";
    }
}


bool isMathOperatorType(const Token& op) {
    return op.mType == TokenType::Plus
    || op.mType == TokenType::Minus
    || op.mType == TokenType::Mul
    || op.mType == TokenType::Div
    ;
}

// punkt vor strich :P orders of ...

bool isMathAssignOperatorType(TokenType tokenType) {
    return tokenType == TokenType::AssignPlus
    || tokenType == TokenType::AssignMinus
    || tokenType == TokenType::AssignMul
    || tokenType == TokenType::AssignDiv
    ;
}

bool isInlineMathType(const TokenType tokenType) {
    return
    tokenType == TokenType::PlusPlus
    || tokenType == TokenType::MinusMinus
    ;
}

bool isMathType(const Token& op) {
    return
    isMathOperatorType(op)
    || op.mType == TokenType::Greater
    || op.mType == TokenType::Less
    || op.mType == TokenType::Equal
    || op.mType == TokenType::NotEqual
    || op.mType == TokenType::Or
    || op.mType == TokenType::And
    || op.mType == TokenType::LowerEqual
    || op.mType == TokenType::GreaterEqual
    || op.mType == TokenType::BitAnd
    || op.mType == TokenType::BitOr
    || op.mType == TokenType::BitXOr
    || op.mType == TokenType::SHL
    || op.mType == TokenType::SHR
    || op.mType == TokenType::Modulo
    // || op.mType == TokenType::Not
    || op.mType == TokenType::Question
    || op.mType == TokenType::Dot
    || op.mType == TokenType::Arrow
    || op.mType == TokenType::KeyValue
    || op.mType == TokenType::DotDot
    || op.mType == TokenType::DotAssign

    || op.mType == TokenType::Assign
    || isInlineMathType(op.mType)
    || isMathAssignOperatorType(op.mType)
    ;
}



bool isContinueToken(const Token& op) {
    return op.mType != TokenType::EOFToken
    && op.mType != TokenType::RParen
    && op.mType != TokenType::Semicolon
    && op.mType != TokenType::End
    && op.mType != TokenType::Else
    && op.mType != TokenType::ElIf
    && op.mType != TokenType::RSquare
    && !isMathType(op)
    ;
}

} //namespace DreiZehn
