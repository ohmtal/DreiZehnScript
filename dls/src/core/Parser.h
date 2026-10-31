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
    Tools::errorf("[Parser-ERROR]: %s %s %s %s\n", message.c_str(),p1.c_str(),p2.c_str(),p3.c_str());
}
void PrintParseInfo(const std::string& message
, const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "") {
    Tools::printf("[Parser-INFO]: %s %s %s %s\n", message.c_str(),p1.c_str(),p2.c_str(),p3.c_str());
}


class Parser {
private:
    std::vector<Token> mTokens;
    VM* vm;
    size_t mPos = 0;
    U32 requestParam = 0;
    // ------------------------------
    Token peek() { return mTokens[mPos]; }
    size_t tell() { return mPos; }
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
    bool parsePrimary() {
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
                vm->emitCode(op_literal );
                vm->emitValue( evaluatedValue );
            }
            break;

            // ~~~~ String ~~~~
            case TokenType::StringLiteral: {
                Token t = advance();
                vm->emitCode(op_literal );
                vm->emitValue( Value(t.mValue) );
            }
            break;
            // ~~~~ LParen/RParen () ~~~~
            case TokenType::LParen: {
                advance();

                if (!getArg("inside Paren")) return false;

                if (peek().mType == TokenType::RParen) {
                    advance(); //  ')'
                } else {
                    PrintParseError("Error: Missing closing bracket");
                    return false;
                }
                // NOTE nothing else to do here ?!
            }
            break;
            // // ~~~~ Comma  ~~~~
            // case TokenType::Comma: {
            //     advance();
            // }
            // break;
            // ~~~~ Identifier ~~~~
            case TokenType::Identifier: {
                advance(); // eat
                // HACK fest - unable to save the tape when pushing the
                //             symId which is not always the same!
                U32 symId = SymbolTable::insert(t.mValue);
                Value* constansPointer = FunctionMap::getConstants(symId);
                if (constansPointer) {
                    vm->emitCode(op_literal );
                    vm->emitValue(*constansPointer);
                }
                else
                if (FunctionMap::IsCFunction(symId)) {
                    U32 argCount = 0;
                    while (isContinueToken(peek())) {
                        if (peek().mType == TokenType::Comma){
                            advance();
                            break;
                        }
                        requestParam++;
                        if (!getArg( "c function parameters" )) return false;
                        requestParam--;

                        argCount++;
                    }
                    vm->emitCode( op_cfunc );
                    vm->emitTapeChunk((size_t)symId);
                    vm->emitTapeChunk((size_t)argCount);

                    if (requestParam <= 0) vm->emitCode( op_pop_stack );
                }
                else
                if (peek().mType == TokenType::LSquare) {
                    advance(); //eat
                    if (!getArg("Square Paren")) return false;
                    if (peek().mType != TokenType::RSquare) {
                        PrintParseError("right square paren missing");
                        return false;
                    }
                    advance(); //RSquare
                    if (peek().mType == TokenType::Assign) {
                        advance(); //Assign
                        requestParam++;
                        if (!getArg("array assign")) return false;
                        vm->emitCode(op_save_array);
                        vm->emitTapeChunk(symId);
                    } else {
                        vm->emitCode(op_load_array);
                        vm->emitTapeChunk(symId);
                    }


                }
                else
                if (peek().mType == TokenType::Assign) {
                    advance(); // eat Assign
                   // FIXME only globals at the moment

                    requestParam++;

                    if (!getArg("Assign")) return false;
                    Variables.saveGlobal(symId, Value(0.0));
                    vm->emitCode(op_save_global);
                    vm->emitTapeChunk(symId);
                }
                else
                if (Variables.isGlobal(symId)) {
                    vm->emitCode(op_load_global);
                    vm->emitTapeChunk(symId);
                }
                else
                {
                    PrintLexerError("Unhandled Identifier:", t.mValue);
                    return false;
                }
            }
            break;
            case TokenType::Define: {
                advance(); //eat define
                if (peek().mType != TokenType::Identifier) {
                    PrintParseError("variable name after define expected");
                    return false;
                }
                U32 varSymId = SymbolTable::insert(advance().mValue);
                if (peek().mType == TokenType::Assign) advance(); //eat optional "="

                if (!getArg("Define")) return false;
                // HACK fest
                // I  push it directly !!!
                //  - with this I cant save a tape!
                //  - without i need to lookup the constants in op_code (slower)

                size_t tapePos = vm->tape.size() - 1;
                if (vm->tape[tapePos - 1 ].code == op_literal) {
                    Value a = vm->tape.back().value; vm->tape.pop_back();
                    vm->tape.pop_back();
                    FunctionMap::RegisterConstants(varSymId,a);
                } else {
                    PrintParseError("sorry, define only allows literals");
                    return false;
                }

            }
            break;



            // ~~~~ Others ~~~~
            default:
                PrintParseError("Unkandled Token:", tokenTypeToString(curType));
                return false;
                break;
        } //switch
        return true;
    }

    // -------------------------------------------------------------------------
    // Math
    // -------------------------------------------------------------------------
    bool parseMath(int minPrecedence) {
        if (!parsePrimary()) return false;

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

            if (!parseMath(precedence + 1)) return false;

            if (op.mType == TokenType::Plus)  vm->emitCode(op_add);
            if (op.mType == TokenType::Minus) vm->emitCode(op_sub );
            if (op.mType == TokenType::Mul)   vm->emitCode( op_mul );
            if (op.mType == TokenType::Div)   vm->emitCode( op_div );

            if (op.mType == TokenType::Greater)      vm->emitCode( op_greater );
            if (op.mType == TokenType::Less)         vm->emitCode( op_lower );
            if (op.mType == TokenType::Equal)        vm->emitCode( op_equal );
            if (op.mType == TokenType::NotEqual)     vm->emitCode( op_not_equal );
            if (op.mType == TokenType::LowerEqual)   vm->emitCode( op_lower_equal );
            if (op.mType == TokenType::GreaterEqual) vm->emitCode( op_greater_equal );

        } // while
        return true;
    }

    bool getArg(const std::string& Where) {
        size_t curPos = tell();
        if (!parseMath(0)) return false;
        if (tell() == curPos) {
            PrintParseError("Value expected at:", Where);
            return false;
        }
        return true;
    }
    // -------------------------------------------------------------------------
    //  Statements sub: While
    // -------------------------------------------------------------------------
    bool parseWhile() {
        advance();
        size_t loop_start_slot = vm->tell();

        if (!getArg("While")) return false;
        vm->emitCode(op_jump_if_false);
        size_t false_jump_slot = vm->tell();
        vm->emitJumpOffset(0); // false placeholder


        // break continue
        LoopContext context;
        context.continue_target = loop_start_slot;
        vm->mLoopStack.push_back(context);


        // main loop:
        while (peek().mType != TokenType::EOFToken &&
            peek().mType != TokenType::End)
        {
            if (!parseStatements()) return false;
        }
        if (peek().mType != TokenType::End) {
            PrintParseError("While missing end!");
            return false;
        }
        advance(); //end

        vm->emitCode(op_jump);
        vm->emitJumpOffset(loop_start_slot);
        vm->patchJumpOffset(false_jump_slot,vm->tell());


        size_t loop_end_slot = vm->tell();

        // handle breaks
        for (size_t break_slot : vm->mLoopStack.back().break_slots) {
            vm->patchJumpOffset(break_slot, loop_end_slot);
        }
        vm->mLoopStack.pop_back();

        return true;
    }


    // -------------------------------------------------------------------------
    // Statements sub :  If ElIf Else
    // -------------------------------------------------------------------------
    bool parseIfElifElse() {
        advance(); // eat 'if'

        // save jumps for
        std::vector<size_t> end_jump_slots;

        if (!getArg("If condition")) return false;

        vm->emitCode(op_jump_if_false);
        size_t false_jump_slot = vm->tell();
        vm->emitJumpOffset(0); // false placeholder

        // main loop:
        while (peek().mType != TokenType::EOFToken &&
            peek().mType != TokenType::End &&
            peek().mType != TokenType::ElIf &&
            peek().mType != TokenType::Else) {
            if (!parseStatements()) return false;
        }
        vm->emitCode(op_jump);
        end_jump_slots.push_back(vm->tell());
        vm->emitJumpOffset(0); // end placeholder

        // patch end jump
        vm->patchJumpOffset(false_jump_slot, vm->tell());

        // --- ELIF loop ---
        while (peek().mType == TokenType::ElIf) {
            advance(); // eat 'elif'

            if (!getArg("elif condition")) return false;
            vm->emitCode(op_jump_if_false);
            false_jump_slot = vm->tell();
            vm->emitJumpOffset(0); // placeholder nect elif

            while (peek().mType != TokenType::EOFToken &&
                peek().mType != TokenType::End &&
                peek().mType != TokenType::ElIf &&
                peek().mType != TokenType::Else) {
                if (!parseStatements()) return false;;
                }

                // elif jump to end
                vm->emitCode(op_jump);
            end_jump_slots.push_back(vm->tell());
            vm->emitJumpOffset(0);

            // previous false-Jump patch
            vm->patchJumpOffset(false_jump_slot, vm->tell());
        }

        // --- ELSE ---
        if (peek().mType == TokenType::Else) {
            advance(); // eat 'else'

            while (peek().mType != TokenType::EOFToken &&
                peek().mType != TokenType::End) {
                if (!parseStatements()) return false;
            }
        }

        // final end
        if (peek().mType == TokenType::End) {
            advance(); // eat 'end'
        } else {
            PrintParseError("Missing end!");
            return false;
        }

        // --- FINAL JUMPS PATCH ---
        size_t final_destination = vm->tell();
        for (size_t slot : end_jump_slots) {
            vm->patchJumpOffset(slot, final_destination);
        }
        return true;
    }

    // -------------------------------------------------------------------------
    // Statements
    // -------------------------------------------------------------------------
    bool parseStatements() {
        TokenType tokenType = peek().mType;
        // Tools::printf("PARSE STATEMENTS: %s\n", tokenTypeToString(tokenType));
        switch (tokenType) {
            case TokenType::EOLToken:
            case TokenType::Semicolon: {
                advance();
                break;
            }
            // ----- If / ElIf / Else ----
            case TokenType::If: {
                if (!parseIfElifElse()) return false;
            }
            break;
            case TokenType::While: {
                if (!parseWhile()) return false;
            }
            break;
            case TokenType::Continue: {
                advance(); // eat 'continue'
                if (vm->mLoopStack.empty()) {
                    PrintParseError("'continue' found outside of a loop!");
                    return false;
                }
                vm->emitCode(op_jump);
                vm->emitJumpOffset(vm->mLoopStack.back().continue_target);
            }
            break;
            case TokenType::Break: {
                advance(); // eat 'break'

                if (vm->mLoopStack.empty()) {
                    PrintParseError("'break' found outside of a loop!");
                    return false;
                }
                vm->emitCode(op_jump);
                vm->mLoopStack.back().break_slots.push_back(vm->tell());
                vm->emitJumpOffset(0); // placeholder patched in while
            }
            break;

            // ----- default ----
            default: if (!parseMath(0)) return false;; break;
        } //switch
        return true;
    }
    // -------------------------------------------------------------------------

public:
    Parser(std::vector<Token> t, VM* vm) : mTokens(std::move(t)), vm(vm) { }
    bool parse(){
        while (peek().mType != TokenType::EOFToken) {
            U32 startPos = tell();
            if (!parseStatements()) return false;

            if (tell() == startPos) {
                PrintParseError("Failed to parse: reason stuck!");
                return false;
            }
        }
        vm->emitCode(op_exit);
        return true;
    }

    // std::vector<std::unique_ptr<ASTNode>> parseStatements();
    // std::unique_ptr<ASTNode> parseLine();

};
} //namespace
