//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Parser: Pratt (top-down operator precedence parser /Precedence Climbing Parser)
//-----------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

#include "Token.h"
#include "Lexer.h"
#include "VM.h"
#include "FunctionMap.h"

namespace DLS{

namespace ParserFrame {
    bool guessGlobalVarAndInsertLocalIfNot(uint32_t symId);
    void startFunc(uint32_t funcSymId);
    void endFunc(uint32_t funcSymId);
} //namespace

void PrintParseError(const std::string& message
, const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "") {
    //TODO tool printer
    std::cout << "[Parser-ERROR] " << message << p1 << p2 << p3 << "\n";
}
void PrintParseInfo(const std::string& message
, const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "") {
    //TODO tool printer
    std::cout << "[Parser-Info] " << message << p1 << p2 << p3 << "\n";
}


class Parser {
private:
    std::vector<Token> mTokens;
    VM* mVM;
    size_t mPos = 0;
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
    int getPrecedence(TokenType type){
        if (isInlineMathType(type)) return 100;
        if (type == TokenType::Mul || type == TokenType::Div ||
            type == TokenType::Modulo || type == TokenType::Not) return 90;
        if (type == TokenType::Plus || type == TokenType::Minus) return 80;
        if (type == TokenType::SHL || type == TokenType::SHR) return 70;

        if (type == TokenType::Less || type == TokenType::Greater ||
            type == TokenType::LowerEqual || type == TokenType::GreaterEqual) return 60;

        if (type == TokenType::Equal || type == TokenType::NotEqual) return 50;

        if (type == TokenType::BitAnd) return 40;
        if (type == TokenType::BitXOr) return 30;
        if (type == TokenType::BitOr)  return 20;

        if (type == TokenType::And || type == TokenType::Or) return 10;

        if (type == TokenType::Question) return 0;
        if (type == TokenType::Assign) return 0;
        if (type == TokenType::Dot) return 0;
        if (type == TokenType::Arrow) return 0;
        if (type == TokenType::KeyValue) return 0;
        if (type == TokenType::DotDot) return 0;
        if (type == TokenType::DotAssign) return 0;
        if (isMathAssignOperatorType(type)) return 0;

        return -1;
    }

    // -------------------------------------------------------------------------
    // parsePrimary
    // -------------------------------------------------------------------------
    void parsePrimary() {
        Token t = peek();
        TokenType curType = peek().mType;

        switch (curType) {
            // ~~~~ Number ~~~~
            case TokenType::Number: {
                Token t = advance();
                std::string rawValue = t.mValue;
                Value evaluatedValue;

                char* endptr = nullptr;
                bool  haveDot = rawValue.find('.') != std::string::npos;
                double resDouble = std::strtod(rawValue.c_str(), &endptr);
                if (rawValue.empty() || *endptr != '\0') {
                    evaluatedValue = Value(0);
                } else {
                    if (haveDot) evaluatedValue = Value(resDouble);
                    else evaluatedValue =  Value((S64)resDouble);
                }
                mVM->tape.push_back({.code = op_literal});
                mVM->tape.push_back({.value = evaluatedValue});
            }
            break;

            // ~~~~ String ~~~~
            case TokenType::StringLiteral: {
                Token t = advance();
                mVM->tape.push_back({.code = op_literal});
                mVM->tape.push_back({.value = Value(t.mValue)});
            }
            break;
            // ~~~~ LParen/RParen () ~~~~
            case TokenType::LParen: {
                advance();

                parseMath(0);

                if (peek().mType == TokenType::RParen) {
                    advance(); //  ')'
                } else {
                    PrintParseError("Error: Missing closing bracket");
                }
                // NOTE nothing else to do here ?!
            }
            break;
            // ~~~~ Comma () ~~~~
            case TokenType::Comma: {
                advance();
            }
            break;
            // ~~~~ Identifier ~~~~
            case TokenType::Identifier: {
                advance();
                U32 symId = SymbolTable::insert(t.mValue);
                if (FunctionMap::IsCFunction(symId)) {
                    U32 argCount = 0;
                    while (isContinueToken(peek())) {
                        size_t lastPos = mPos;
                        parseMath(0);
                        argCount++;
                        if (lastPos == mPos) {
                            PrintParseError("CFUNC CALL parameter stuck!");
                            break;
                        }
                    }
                    mVM->tape.push_back({.code = op_literal});
                    mVM->tape.push_back({.value = Value(symId)});
                    mVM->tape.push_back({.code = op_literal});
                    mVM->tape.push_back({.value = Value(argCount)});
                    mVM->tape.push_back({.code = op_cfunc});
                } else {
                    PrintLexerError("Unhandled Identifier:", t.mValue);
                }
            }
            break;
            // ~~~~ Others ~~~~
            default:
                PrintParseError("Unkandled Token:", tokenTypeToString(curType));
                break;
        } //switch

    }

    // -------------------------------------------------------------------------
    // Math
    // -------------------------------------------------------------------------
    void parseMath(int minPrecedence) {
        parsePrimary();

        while (isMathType(peek())) {
            Token op = peek();
            int precedence = getPrecedence(op.mType);

            if (precedence < minPrecedence) {
                break;
            }
            advance();

            // if (op.mType == TokenType::Assign) {
            //     parseAssignmentContext();
            //     continue;
            // }

            parseMath(precedence + 1);

            if (op.mType == TokenType::Plus)  mVM->tape.push_back({ .code = op_add });
            if (op.mType == TokenType::Minus) mVM->tape.push_back({ .code = op_sub });
            if (op.mType == TokenType::Mul)   mVM->tape.push_back({ .code = op_mul });
            if (op.mType == TokenType::Div)   mVM->tape.push_back({ .code = op_div });
        }
    }



    // -------------------------------------------------------------------------
    // std::unique_ptr<Expression> parsePrimary();
    //
    // -------------------------------------------------------------------------
    // //TEST
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
    //
    // // -------------------------------------------------------------------------
    // // 0.6 added short if/then
    // std::unique_ptr<Expression> parseMath(int minPrecedence = 0);
    //
    // // -------------------------------------------------------------------------
    // std::unique_ptr<Expression> parseComparison();
    // // -------------------------------------------------------------------------
    // std::unique_ptr<Expression> parseExpression();

public:
    Parser(std::vector<Token> t, VM* vm) : mTokens(std::move(t)), mVM(vm) { }
    bool parse(){
        while (peek().mType != TokenType::EOFToken) {
            // PrintParseInfo("TokenType: ", tokenTypeToString(peek().mType));

            U32 startPos = mPos;
            //FIXME
            parseMath(0);

            if (mPos == startPos) {
                PrintParseError("Failed to parse: reason stuck!");
                return false;
            }
        }
        return true;
    }

    // std::vector<std::unique_ptr<ASTNode>> parseStatements();
    // std::unique_ptr<ASTNode> parseLine();

};
} //namespace
