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

namespace DreiZehn{

enum class TokenType {
    Identifier
    , Number
    , Assign
    , StringLiteral
    , LParen
    , RParen
    // math
    , Plus , Minus , Mul , Div

    , PlusPlus, MinusMinus

    , AssignPlus, AssignMinus, AssignMul, AssignDiv

    // if ...
    , If, Greater, Less, Equal, NotEqual, Else
    // fn functions
    , Fn, End


    // for iter
     ,For

    // break, return
    , Break, Return

    , Semicolon

    , While
    , Or, And, BitOr, BitAnd
    , LowerEqual, GreaterEqual
    , SHL, SHR

    , Arrow, Dot

    , NoToken // for peekPrev pos < 1
    , EOFToken
};

inline const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Identifier:    return "Identifier";
        case TokenType::Number:        return "Number";
        case TokenType::Assign:        return "Assign";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::LParen:        return "LParen";
        case TokenType::RParen:        return "RParen";

        // math
        case TokenType::Plus:          return "Plus";
        case TokenType::PlusPlus:      return "PlusPlus";
        case TokenType::MinusMinus:    return "MinusMinus";
        case TokenType::Minus:         return "Minus";
        case TokenType::Mul:           return "Mul";
        case TokenType::Div:           return "Div";

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

        // fn functions
        case TokenType::Fn:            return "Fn";
        case TokenType::End:           return "End";

        // for / while iter
        case TokenType::For:           return "For";
        case TokenType::While:         return "While";

        // break, return
        case TokenType::Break:         return "Break";
        case TokenType::Return:        return "Return";

        case TokenType::Or:            return "OR ||";
        case TokenType::BitOr:         return "Bit |";
        case TokenType::BitAnd:         return "Bit &";
        case TokenType::And:            return "AND &&";
        case TokenType::LowerEqual:     return "LowerEqual";
        case TokenType::GreaterEqual:   return "GreaterEqual";
        case TokenType::SHL:            return "Shift Left <<";
        case TokenType::SHR:            return "Shift Right >>";

        case TokenType::Semicolon:     return "Semicolon";

        case TokenType::Arrow:          return "Arrow Object Method call";
        case TokenType::Dot:            return "Dot Object field access";

        case TokenType::EOFToken:      return "EOFToken";

        default:                       return "UnknownToken";
    }
}

struct Token {
    TokenType mType;
    std::string mValue;
};

class Lexer {
private:
    std::string mSrc;
    int32_t mPos = 0;

    char peek() { return mPos < mSrc.size() ? mSrc[mPos] : '\0'; }
    char peekNext() { return mPos+1 < mSrc.size() ? mSrc[mPos + 1] : '\0'; }
    char peekPrev() { return mPos-1 > 0  ? mSrc[mPos - 1] : '\0'; }
    char advance() { return mPos < mSrc.size() ? mSrc[mPos++] : '\0'; }

public:
    Lexer(std::string source) : mSrc(std::move(source)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (peek() != '\0') {
            // // ----------------------------------------------------------------
            if (std::isspace(peek())) { advance(); continue; }
            // ----------------------------------------------------------------
            if (peek() == '>' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::GreaterEqual, ">="}); continue; }
            if (peek() == '<' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::LowerEqual, "<="}); continue; }

           if (peek() == '+' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::AssignPlus, "+="}); continue; }
           if (peek() == '-' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::AssignMinus, "-="}); continue; }
           if (peek() == '*' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::AssignMul, "*="}); continue; }
           if (peek() == '/' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::AssignDiv, "/="}); continue; }

            if (peek() == '=') {
                advance();
                if (peek() == '=') { // "=="
                    advance();
                    tokens.push_back({TokenType::Equal, "=="});
                } else {
                    tokens.push_back({TokenType::Assign, "="});
                }
                continue;
            }
            if (peek() == '!' && peekNext() == '=') {
                advance();
                tokens.push_back({TokenType::NotEqual, "!="});
            }

            if (peek() == '+' && peekNext() == '+') { advance();advance(); tokens.push_back({TokenType::PlusPlus, "++"}); continue; }
            if (peek() == '-' && peekNext() == '-') { advance();advance(); tokens.push_back({TokenType::MinusMinus, "--"}); continue; }

            if (peek() == '(') { advance(); tokens.push_back({TokenType::LParen, "("}); continue; }
            if (peek() == ')') { advance(); tokens.push_back({TokenType::RParen, ")"}); continue; }
            if (peek() == '+') { advance(); tokens.push_back({TokenType::Plus, "+"}); continue; }
            if (peek() == '*') { advance(); tokens.push_back({TokenType::Mul,   "*"}); continue; }
            if (peek() == '/') { advance(); tokens.push_back({TokenType::Div,   "/"}); continue; }

            // ----------------------------------------------------------------
            if (peek() == '-' && peekNext() == '>') {
                advance();advance();
                tokens.push_back({TokenType::Arrow, "->"});
                continue;
            }

            if (peek() == '>' && peekNext() == '>') { advance();advance(); tokens.push_back({TokenType::SHR, ">>"}); continue; }
            if (peek() == '<' && peekNext() == '<') { advance();advance(); tokens.push_back({TokenType::SHL, "<<"}); continue; }

            if (peek() == '>') { advance(); tokens.push_back({TokenType::Greater, ">"}); continue; }
            if (peek() == '<') { advance(); tokens.push_back({TokenType::Less, "<"}); continue; }
            // ----------------------------------------------------------------

            if (peek() == '|' && peekNext() != '|') { advance(); tokens.push_back({TokenType::BitOr, "|"}); continue; }
            if (peek() == '&' && peekNext() != '&') { advance(); tokens.push_back({TokenType::BitAnd, "&"}); continue; }

            if (peek() == '|' && peekNext() == '|') { advance();advance(); tokens.push_back({TokenType::Or, "||"}); continue; }
            if (peek() == '&' && peekNext() == '&') { advance();advance(); tokens.push_back({TokenType::And, "&&"}); continue; }

            // ----------------------------------------------------------------
            // Numbers
            //FIXME this can be optimized!
            if ( std::isdigit(peek())
                || (!std::isdigit(peekPrev()) && peek() == '-' && std::isdigit(peekNext()))
                || (std::isdigit(peekPrev()) && peek() == '.' && std::isdigit(peekNext()))
            ){
                std::string num;
                while (std::isdigit(peek()) || peek() == '.' || peek() == '-') {
                    num += advance();
                }
                tokens.push_back({TokenType::Number, num});
                continue;
            }

            // minus after number
            if (peek() == '-') { advance(); tokens.push_back({TokenType::Minus, "-"}); continue; }

            if (peek() == '.') { advance(); tokens.push_back({TokenType::Dot, "."}); continue; }

            // ----------------------------------------------------------------
            // Identifier and Keywords scan
            if (std::isalpha(peek()) || peek() == '_' ||/* peek() == '.' ||*/ peek() == ':') {
                std::string id;

                while (std::isalnum(peek()) || peek() == '_' ||/* peek() == '.' ||*/ peek() == ':') {
                    id += advance();
                }

                if (id == "if") { tokens.push_back({TokenType::If, "if"}); }
                else if (id == "else") { tokens.push_back({TokenType::Else, "else"}); }
                else if (id == "fn") { tokens.push_back({TokenType::Fn, "fn"});  }
                else if (id == "end") { tokens.push_back({TokenType::End, "end"}); }
                else if (id == "for") { tokens.push_back({TokenType::For, "for"});  }
                else if (id == "break") { tokens.push_back({TokenType::Break, "break"});  }
                else if (id == "return") { tokens.push_back({TokenType::Return, "return"});  }
                else if (id == "while") { tokens.push_back({TokenType::While, "while"});  }
                else {
                    tokens.push_back({TokenType::Identifier, id});
                }
                continue;
            }


            // ----------------------------------------------------------------
            // StringLiteral

            if (peek() == '"') {
                advance(); // eat '"'
                std::string strValue;

                while (peek() != '"' && peek() != '\0') {
                    if (peek() == '\\') {
                        advance();
                        if (peek() == 'n') { strValue += '\n'; advance(); }
                        else if (peek() == '"') { strValue += '"'; advance(); }
                        else { strValue += '\\'; }
                    } else {
                        strValue += advance();
                    }
                }

                if (peek() == '"') {
                    advance(); //eat '"'
                } else {
                    Tools::errorf("[Lexer-ERROR] String not closed!\n");
                    mPos = mSrc.size();
                    continue;
                }

                tokens.push_back({TokenType::StringLiteral, strValue});
                continue;
            }


            if (peek() == ';') { advance(); tokens.push_back({TokenType::Semicolon, ";"}); continue; }

            advance(); // skip unknown
        }
        tokens.push_back({TokenType::EOFToken, ""});
        return tokens;
    }
};
} //Namespace
