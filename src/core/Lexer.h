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
    , LSquare, RSquare
    // , LCurly, RCurly
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

    , Define, Import

    , NoToken // for peekPrev pos < 1
    , EOFToken
};

inline const char* tokenTypeToString(TokenType type) {
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
        case TokenType::Define:         return "Define";

        case TokenType::EOFToken:       return "EOFToken";

        default:                        return "UnknownToken";
    }
}

struct Token {
    TokenType mType;
    std::string mValue;
};

inline bool isMathOperatorType(const Token& op) {
    return op.mType == TokenType::Plus
    || op.mType == TokenType::Minus
    || op.mType == TokenType::Mul
    || op.mType == TokenType::Div
    ;
}

// punkt vor strich :P orders of ...



inline bool isMathAssignOperatorType(TokenType tokenType) {
    return tokenType == TokenType::AssignPlus
    || tokenType == TokenType::AssignMinus
    || tokenType == TokenType::AssignMul
    || tokenType == TokenType::AssignDiv
    ;
}

inline bool isInlineMathType(const TokenType tokenType) {
    return
    tokenType == TokenType::PlusPlus
    || tokenType == TokenType::MinusMinus
    ;
}

inline bool isMathType(const Token& op) {
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

    || op.mType == TokenType::Assign
    || isInlineMathType(op.mType)
    || isMathAssignOperatorType(op.mType)
    ;
}



inline bool isContinueToken(const Token& op) {
    return op.mType != TokenType::EOFToken
    && op.mType != TokenType::RParen
    && op.mType != TokenType::Semicolon
    && op.mType != TokenType::End
    && op.mType != TokenType::Else
    && op.mType != TokenType::ElIf
    && op.mType != TokenType::RSquare
    // && op.mType != TokenType::RCurly
    && !isMathType(op)
    ;
}


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

            if (peek() == '!') { advance(); tokens.push_back({TokenType::Not,   "!"}); continue; }
            if (peek() == '%') { advance(); tokens.push_back({TokenType::Modulo,"%"}); continue; }

            if (peek() == '[') { advance(); tokens.push_back({TokenType::LSquare,"["}); continue; }
            if (peek() == ']') { advance(); tokens.push_back({TokenType::RSquare,"]"}); continue; }


            // if (peek() == '{') { advance(); tokens.push_back({TokenType::LCurly,"{"}); continue; }
            // if (peek() == '}') { advance(); tokens.push_back({TokenType::RCurly,"}"}); continue; }

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

            if (peek() == '^' ) { advance(); tokens.push_back({TokenType::BitXOr, "^"}); continue; }
            if (peek() == '|' && peekNext() != '|') { advance(); tokens.push_back({TokenType::BitOr, "|"}); continue; }
            if (peek() == '&' && peekNext() != '&') { advance(); tokens.push_back({TokenType::BitAnd, "&"}); continue; }

            if (peek() == '|' && peekNext() == '|') { advance();advance(); tokens.push_back({TokenType::Or, "||"}); continue; }
            if (peek() == '&' && peekNext() == '&') { advance();advance(); tokens.push_back({TokenType::And, "&&"}); continue; }

            // ----------------------------------------------------------------
            // Numbers
            //  - hex numbers  ( 0xFFFFFF / 0x1a2b)
            if (peek() == '0' && (peekNext() == 'x' || peekNext() == 'X') && std::isxdigit(peekOffset(+2)) ) {
                std::string hexNum;
                hexNum += advance(); hexNum += advance();
                while (std::isxdigit(peek())) { hexNum += advance(); }
                unsigned long long rawValue = std::stoull(hexNum, nullptr, 16);
                int32_t intVal = static_cast<int32_t>(rawValue);

                tokens.push_back({TokenType::Number, std::to_string(intVal)});
                continue;
            }

            // --- Normal number
            if (std::isdigit(peek()) || (peek() == '.' && std::isdigit(peekNext()))) {
                std::string num;
                bool hasDot = false;
                bool hasExponent = false;

                while (peek() != '\0') {
                    char c = peek();

                    if (std::isdigit(c)) {
                        num += advance();
                    }
                    else if (c == '.' && !hasDot && !hasExponent) {
                        hasDot = true;
                        num += advance();
                    }
                    else if ((c == 'e' || c == 'E') && !hasExponent) {
                        hasExponent = true;
                        num += advance();
                        if (peek() == '+' || peek() == '-') {
                            num += advance();
                        }
                    }
                    else {
                        break;
                    }
                }
                tokens.push_back({TokenType::Number, num});
                continue;
            }


            // if ( std::isdigit(peek())
            //     || (std::isdigit(peekPrev()) && peek() == '.' && std::isdigit(peekNext()))
            //     || (peek() == '-' && std::isdigit(peekNext()) && (tokens.size()>0 && isMathType(tokens.back())) )
            //     // || (!std::isdigit(peekPrev()) && peek() == '-' && std::isdigit(peekNext()))
            //
            //
            // ){
            //     std::string num;
            //     bool isFirst = true;
            //     while (std::isdigit(peek()) || (peek() == '.' && !isFirst) || (peek() == '-' && isFirst)) {
            //         num += advance();
            //         isFirst = false;
            //     }
            //     tokens.push_back({TokenType::Number, num});
            //     continue;
            // }

            // minus after number
            if (peek() == '-') { advance(); tokens.push_back({TokenType::Minus, "-"}); continue; }

            if (peek() == '.') { advance(); tokens.push_back({TokenType::Dot, "."}); continue; }

            // ----------------------------------------------------------------
            // Identifier and Keywords scan
            // 0.6c only allow "::" not more or less
            if (std::isalpha(peek()) || peek() == '_' || ( peek() == ':' && peekNext() == ':') ) {
                std::string id;

                if (peek() == ':'  ) {
                    if (peekNext() == ':') {
                        advance();
                    }
                }

                while (std::isalnum(peek()) || peek() == '_' ||/* peek() == '.' ||*/ peek() == ':') {
                    id += advance();
                }

                if (id == "if") { tokens.push_back({TokenType::If, "if"}); }
                else if (id == "else") { tokens.push_back({TokenType::Else, "else"}); }
                else if (id == "elif") { tokens.push_back({TokenType::ElIf, "elif"});  }
                else if (id == "fn") { tokens.push_back({TokenType::Fn, "fn"});  }
                else if (id == "end") { tokens.push_back({TokenType::End, "end"}); }
                else if (id == "for") { tokens.push_back({TokenType::For, "for"});  }
                else if (id == "break") { tokens.push_back({TokenType::Break, "break"});  }
                else if (id == "return") { tokens.push_back({TokenType::Return, "return"});  }
                else if (id == "continue") { tokens.push_back({TokenType::Continue, "continue"});  }
                else if (id == "while") { tokens.push_back({TokenType::While, "while"});  }
                else if (id == "forRange") { tokens.push_back({TokenType::forRange, "forRange"});  }
                else if (id == "forrange") { tokens.push_back({TokenType::forRange, "forrange"});  }
                else if (id == "forEach") { tokens.push_back({TokenType::forEach, "forEach"});  }
                else if (id == "foreach") { tokens.push_back({TokenType::forEach, "foreach"});  }
                else if (id == "not") { tokens.push_back({TokenType::Not, "not"});  }
                else if (id == "and") { tokens.push_back({TokenType::And, "and"});  }
                else if (id == "or") { tokens.push_back({TokenType::Or, "or"});  }
                else if (id == "import") { tokens.push_back({TokenType::Import, "import"});  }
                else if (id == "define") { tokens.push_back({TokenType::Define, "define"});  }
                else {
                    tokens.push_back({TokenType::Identifier, id});
                }
                continue;
            }

             if (peek() == '?') { advance(); tokens.push_back({TokenType::Question, "?"}); continue; }
             if (peek() == ':') { advance(); tokens.push_back({TokenType::Colon, ":"}); continue; }

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
                    // mPos = mSrc.size();
                    // continue;

                    // clear all token and exit .. nothing else to do here !!
                    tokens.clear();
                    return tokens;
                }

                tokens.push_back({TokenType::StringLiteral, strValue});
                continue;
            }

            // ---------- Comment ---------------
            if (peek() == '#') {
                advance(); // eat '#'
                while ( peek() != '\0') {
                    if (peek() == '\\' && peekNext() == 'n' ) {
                        advance();advance();
                        break; //end of line
                    }
                    if (peek() == '#') {
                        advance();
                        break;
                    }
                    advance();
                }
                continue;
            }


            if (peek() == ';' && peekNext() == ';') { advance(); tokens.push_back({TokenType::End, ";;"}); continue; }
            if (peek() == ';') { advance(); tokens.push_back({TokenType::Semicolon, ";"}); continue; }

            advance(); // skip unknown
        }
       tokens.push_back({TokenType::EOFToken, ""});
       return tokens;
    }
};
} //Namespace
