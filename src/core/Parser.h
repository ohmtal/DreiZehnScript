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

    int getPrecedence(TokenType type) {
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
        if (isMathAssignOperatorType(type)) return 0;

        return -1;
    }

    // -------------------------------------------------------------------------
    std::unique_ptr<Expression> parsePrimary() {


        if (peek().mType == TokenType::Minus) {
            advance(); // eat '-'
            auto expr = parseMath(9);
            return std::make_unique<UnaryMinusExpression>(std::move(expr));

        }
        else if (peek().mType == TokenType::LParen) {
            advance();

            auto expr = parseMath();

            if (peek().mType == TokenType::RParen) {
                advance(); //  ')'
            } else {
                Tools::PrintParseError("Error: Missing closing bracket");
                // Tools::errorf("Error: Missing closing bracket %s %d\n", __FILE__, __LINE__);
            }
            return expr;
        }
        // else if (peek().mType == TokenType::LCurly) {
        //     advance();
        //
        //     // while (peek().mType != TokenType::RCurly || peek().mType == TokenType::EOFToken)
        //     // {
        //     // }
        //     std::vector<std::unique_ptr<Expression>> args;
        //     while (isContinueToken(peek())) {
        //         size_t lastPos = mPos;
        //         args.push_back(parseMath());
        //         if (lastPos == mPos) {
        //             Tools::PrintParseError("In Curly Brackets:");
        //             break;
        //         }
        //     }
        //
        //     if (peek().mType == TokenType::RCurly) {
        //         advance();
        //
        //         return std::make_unique<CurlyArgumentsExpression>(std::move(args));
        //     } else {
        //         Tools::PrintParseError("Error: Missing closing curly bracket\n");
        //     }
        //     return nullptr;
        // }
        else if (peek().mType == TokenType::Not) {
            advance(); //eat Not
            auto expr = parseMath();
            return std::make_unique<BinarySingleRightOpExpression>( TokenType::Not, std::move(expr));
        }
        else if (peek().mType == TokenType::Number || peek().mType == TokenType::StringLiteral) {
            Token t = advance();
            return std::make_unique<LiteralExpression>(t.mType, t.mValue);
        }
        else if (peek().mType == TokenType::Identifier) {
            std::unique_ptr<Expression> currentExpression = nullptr;
            Token nameToken = advance();
            uint32_t nameTokenSymbolId = SymbolTable::insert( nameToken.mValue);
            Value* constansPointer = FunctionMap::getConstants(nameTokenSymbolId);
            if (constansPointer != nullptr) {
                return std::make_unique<ValueExpression>((*constansPointer));
            }
            else if (FunctionMap::IsFunction(nameTokenSymbolId)) {
                std::vector<std::unique_ptr<Expression>> args;
                while (isContinueToken(peek())) {
                    size_t lastPos = mPos;
                    args.push_back(parseMath());
                    if (lastPos == mPos) {
                        Tools::PrintParseError("In function call:");
                        break;
                    }
                }
                return std::make_unique<CallExpression>(nameTokenSymbolId, std::move(args));
            } else {
                // we got a variable
                currentExpression = std::make_unique<VariableExpression>(nameTokenSymbolId);
            }

            while (true) {
                if (peek().mType  == TokenType::Dot && peekNext().mType == TokenType::Identifier ) {
                    advance(); // eat DOT
                    Token fieldToken = advance();
                    uint32_t fieldSymbolId = SymbolTable::insert(fieldToken.mValue);
                    currentExpression = std::make_unique<ObjectFieldExpression>(std::move(currentExpression), fieldSymbolId);
                }
                else  if (peek().mType  == TokenType::Arrow && peekNext().mType == TokenType::Identifier ) {
                    advance(); // eat ->
                    Token methodToken = advance();
                    uint32_t methodSymbolId = SymbolTable::insert(methodToken.mValue);

                    std::vector<std::unique_ptr<Expression>> args;
                    while (isContinueToken(peek())) {
                        size_t lastPos = mPos;
                        args.push_back(parseMath());
                        if (lastPos == mPos) {
                            Tools::PrintParseError("In method call:");
                            break;
                        }
                    }
                    currentExpression = std::make_unique<MethodExpression>(std::move(currentExpression), methodSymbolId, std::move(args));
                }
                else if (peek().mType == TokenType::Dot) {
                    Tools::PrintParseError("Field expected after  '.'");
                    advance();
                    break;
                }
                else if (peek().mType == TokenType::Arrow) {
                    Tools::PrintParseError("Method expected after '->'");
                    advance(); // eat '->'
                    break;
                }
                else if (peek().mType == TokenType::LSquare) {
                      advance(); // eat '['
                     std::unique_ptr<Expression> indexExpr = parseMath();

                     if (peek().mType != TokenType::RSquare) {
                         Tools::PrintParseError("expected ']' after array index!");
                         break;
                     }
                     advance(); // eat ']'
                     currentExpression = std::make_unique<ArrayVariableExpression>(
                         std::move(currentExpression), std::move(indexExpr));

                }
                else {
                    break;
                }
            }
            return currentExpression;

        } // TokenType::Identifier


        return nullptr;
    }

    // -------------------------------------------------------------------------
    // 0.6 added short if/then
    std::unique_ptr<Expression> parseMath(int minPrecedence = 0) {
        auto left = parsePrimary();

        while (isMathType(peek())) {
            Token op = peek();
            int precedence = getPrecedence(op.mType);

            if (precedence < minPrecedence) {
                break;
            }
            advance();

            if (op.mType == TokenType::Assign) {
                if (!left) {
                    Tools::PrintParseError("Assignment but no Variable Expression!");
                    return nullptr;
                }
                auto rhs = parseMath(0);
                if (!rhs) {
                    Tools::PrintParseError("Assignment but no Assign Expression!");
                    return nullptr;
                }
                left = std::make_unique<AssignStatement>(
                    std::move(left), std::move(rhs)
                );
                continue;
            }
            else  if (isMathAssignOperatorType(op.mType)) {
                if (!left) {
                    Tools::PrintParseError("OP Assignment but no Expression!");
                    return nullptr;
                }
                auto rhs = parseMath(0);
                if (!rhs) {
                    Tools::PrintParseError("Assignment but no Assign Expression!");
                    return nullptr;
                }
                left = std::make_unique<AssignOPStatement>(
                    std::move(left),op.mType, std::move(rhs)
                );
                continue;
            }
            else if (op.mType == TokenType::Question) {
                auto trueBranch = parseMath(0);

                if (peek().mType != TokenType::Colon) {
                    Tools::PrintParseError("Expect ':' after '?'");
                    return nullptr;
                }
                advance();
                auto falseBranch = parseMath(precedence);
                left = std::make_unique<TernaryExpression>(
                    std::move(left), std::move(trueBranch), std::move(falseBranch)
                );
                continue;
            }
            else if (isInlineMathType(op.mType)) {
                if (!left) {
                    Tools::PrintParseError("Inline OP but no Expression!");
                    return nullptr;
                }
                left = std::make_unique<BinaryInlineExpression>(
                    std::move(left),op.mType
                );
                continue;
            }

            auto right = parseMath(precedence + 1);

            if (isMathOperatorType(op)) {
                left = std::make_unique<BinaryOpExpression>(std::move(left), op.mType, std::move(right));
            } else {
                left = std::make_unique<BinaryExpression>(std::move(left), op.mType, std::move(right));
            }
        }
        return left;
    }

    // -------------------------------------------------------------------------
    std::unique_ptr<Expression> parseComparison() {
        auto left = parseMath();

        while (peek().mType == TokenType::Greater
            || peek().mType == TokenType::Less
            || peek().mType == TokenType::Equal
            || peek().mType == TokenType::NotEqual
            || peek().mType == TokenType::LowerEqual
            || peek().mType == TokenType::GreaterEqual
        ) {
            Token op = advance();
            auto right = parseMath();
            left = std::make_unique<BinaryExpression>(std::move(left), op.mType, std::move(right));
        }
        return left;
    }
    // -------------------------------------------------------------------------
    std::unique_ptr<Expression> parseExpression() {

        if (peek().mType == TokenType::LParen) {
            advance(); // skip '('
            auto expr = parseExpression(); // inner statement

            if (peek().mType == TokenType::RParen) {
                advance(); // skip ')'
            } else {
                Tools::errorf("Error: Missing closing bracket %s %d\n", __FILE__, __LINE__);
            }
            return expr;
        }

        if (peek().mType == TokenType::Number || peek().mType == TokenType::StringLiteral) {
            Token t = advance();
            return std::make_unique<LiteralExpression>(t.mType, t.mValue);
        }


        if (peek().mType == TokenType::Identifier) {
            Token nameToken = advance();
            uint32_t nameTokenSymbolId = SymbolTable::insert(nameToken.mValue);

            if (FunctionMap::IsFunction(nameTokenSymbolId)) {
                std::vector<std::unique_ptr<Expression>> args;

                while (isContinueToken(peek()))
                {
                    size_t lastPos = mPos;
                    args.push_back(parseMath());

                    if (lastPos == mPos) {
                        Tools::PrintParseError("In function call:");
                        break;
                    }
                }
                return std::make_unique<CallExpression>(nameTokenSymbolId, std::move(args));
            }

            return std::make_unique<VariableExpression>(nameTokenSymbolId);
        }

        return nullptr;
    }
    // -------------------------------------------------------------------------
public:
    Parser(std::vector<Token> t) : mTokens(std::move(t)) {}

    // -------------------------------------------------------------------------
    inline std::vector<std::unique_ptr<ASTNode>> parseStatements() {
        std::vector<std::unique_ptr<ASTNode>> statements;
        while (peek().mType != TokenType::EOFToken) {
            if (peek().mType == TokenType::Semicolon) {
                advance();
                continue;
            }

            if (Globals::gDumpStateNodes) {
                Tools::printf("---------- new statement ------ Pos:%d \n", (int)mPos);
                for(size_t i = mPos; i < mTokens.size() ; i++) {
                    Tools::printf("Token %d: %d :: %s\n", i, (int)mTokens[i].mType, tokenTypeToString(mTokens[i].mType));
                }
            }
            size_t startIndex = mPos;
            auto stmt = parseLine();
            if (Globals::gDumpStateNodes) {
                Tools::printf("---------- LINE parsed ------ Pos:%d next:%d :: %s\n", (int)mPos, (int)mTokens[mPos].mType, tokenTypeToString(mTokens[mPos].mType));
            }
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
            if (peek().mType == TokenType::Semicolon) {
                advance();
            }
            else if (mPos == startIndex) {
                Tools::errorf("Syntax-Error: Unexpected token '%s' (%s:%d)\n"
                , tokenTypeToString(mTokens[mPos].mType),Globals::currentScriptLine.c_str()
                , Globals::currentScriptLineNumber );
                advance();
            }
        }
        return statements;
    }

    // -------------------------------------------------------------------------
    inline std::unique_ptr<ASTNode> parseLine() {


        TokenType tokenType = peek().mType;
        switch (tokenType) {
            case TokenType::Fn: {
                advance(); //eat fn

                if (peek().mType != TokenType::Identifier) {
                    Tools::errorf("Syntax-Error: function name required after fn\n");
                    return nullptr;
                }

                std::string funcName = advance().mValue;
                uint32_t funcNameSymbolId = SymbolTable::insert(funcName);
                std::vector<std::string> params;

                while (peek().mType == TokenType::Identifier) {
                    params.push_back(advance().mValue);
                }
                FunctionMap::RegisteredScriptFunctions[funcNameSymbolId] = { params, {} };
                return std::make_unique<FunctionDefineStartNode>(funcNameSymbolId);
            }
            break;

            case TokenType::forRange: {
                advance(); //eat range

                if (peek().mType != TokenType::Identifier) {
                    Tools::errorf("Syntax-Error: variable name after range expected\n");
                    return nullptr;
                }
                std::string varName = advance().mValue;

                auto countExpr = parseMath();

                return std::make_unique<ForRangeStatement>(SymbolTable::insert(varName)
                , std::move(countExpr));
            }
            break;

            case TokenType::forEach: {
                advance(); //eat

                if (peek().mType != TokenType::Identifier) {
                    Tools::errorf("Syntax-Error: variable name after range expected\n");
                    return nullptr;
                }
                std::string varName = advance().mValue;
                auto varExpr = parseMath();

                return std::make_unique<ForEachStatement>(SymbolTable::insert(varName)
                , std::move(varExpr));
            }
            break;

            case TokenType::For: {
                advance();

                if (peek().mType != TokenType::Identifier) {
                    Tools::errorf("Syntax-Error: variable name after for expected\n");
                    return nullptr;
                }
                std::string varName = advance().mValue;

                auto start = parseMath();
                auto end = parseMath();

                return std::make_unique<ForStatement>(SymbolTable::insert(varName), std::move(start), std::move(end));
            }
            break;

            case TokenType::While:  {
                advance();
                auto condition = parseComparison();
                return std::make_unique<WhileStatement>(std::move(condition));
            }
            break;

            case TokenType::End:  {
                advance();
                return std::make_unique<FunctionDefineEndNode>();
            }
            break;

            case TokenType::If:  {
                advance(); // skip "if"
                auto condition = parseComparison();
                return std::make_unique<IfStatement>(std::move(condition));
            }
            break;

            case TokenType::ElIf:  {
                advance(); // skip "if"
                auto condition = parseComparison();
                return std::make_unique<ElIfStatement>(std::move(condition));
            }
            break;

            case TokenType::Else:  {
                advance(); // skip "else"
                return std::make_unique<ElseMarkerNode>();
            }
            break;

            case TokenType::Break:  {
                advance(); // eat 'break'
                return std::make_unique<BreakStatement>();
            }
            break;

            case TokenType::Continue: {
                advance(); // eat
                return std::make_unique<ContinueStatement>();
            }
            break;

            case TokenType::Return:  {
                advance(); // eat'return'

                std::unique_ptr<Expression> rhs = nullptr;

                if (peek().mType != TokenType::EOFToken &&
                    peek().mType != TokenType::End &&
                    peek().mType != TokenType::Semicolon &&
                    peek().mType != TokenType::RParen) {
                    rhs = parseComparison();
                    }

                    return std::make_unique<ReturnStatement>(std::move(rhs));
            }
            break;

            // -----------------------------------------------------------------
            case TokenType::Define: {
                advance(); //eat define

                if (peek().mType != TokenType::Identifier) {
                    Tools::errorf("Syntax-Error: variable name after define expected\n");
                    return nullptr;
                }
                std::string varName = advance().mValue;

                auto expr = parseMath();

                if (!expr) {
                    Tools::errorf("Syntax-Error: value after define expected\n");
                    return nullptr;
                }

                if (!Globals::gCurEnv) {
                    Tools::PrintParseError("System-Error: current enviroment not set!\n");
                    return nullptr;
                }

                // direct register no token
                Value value = expr->evaluate(*Globals::gCurEnv);
                FunctionMap::RegisterConstants(varName, value);

                return nullptr;
            }
            break;

            case TokenType::Import: {
                advance();

                if (peek().mType != TokenType::Identifier) {
                    Tools::PrintParseError("Syntax-Error: variable name after define expected\n");
                    return nullptr;
                }
                std::string modname = advance().mValue;

                if (!Globals::gCurEnv) {
                    Tools::PrintParseError("System-Error: current enviroment not set!\n");
                    return nullptr;
                }
                if (modname.empty()) {
                    Tools::PrintParseError("System-Error: module name empty.\n");
                    return nullptr;
                }
                // direct import
                if ( modname == "All") {
                    FunctionMap::ModuleRegistry::LoadAll(*Globals::gCurEnv);
                } else {
                    if (!FunctionMap::ModuleRegistry::Load(modname, *Globals::gCurEnv)) {
                        Tools::errorf("Failed to load Module %s\n", modname.c_str());
                    }
                }

                return nullptr;
            }
            break;
            // -----------------------------------------------------------------


            default: {
                 return parseComparison();
            }
            break;

        }

        return parseComparison();
    } //  inline std::unique_ptr<ASTNode> parseLine()

};
} //namespace
