//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// The power machine :)
//-----------------------------------------------------------------------------
#pragma once
#include "VariableFrame.h"
#include "VMStructure.h"
#include <memory>

namespace DreiZehn {

class ASTCompiler {
public:


    static void compileExpression(ASTNode* node, BytecodeChunk& chunk, CompilerScope& scope) {
        if (!node) return;

        // ---------------------------------------------------------------------
        // --- literal
        if (auto* literal = dynamic_cast<LiteralExpression*>(node)) {
            Value finalValue;

            if (literal->mType == TokenType::Number) {
                char* endptr = nullptr;
                double resDouble = std::strtod(literal->mRawValue.c_str(), &endptr);

                if (literal->mRawValue.find('.') != std::string::npos) {
                    finalValue = Value(resDouble);
                } else {
                    finalValue = Value(static_cast<int>(resDouble));
                }
            }

            else if (literal->mType == TokenType::StringLiteral) {
                auto* strObj = new StringValueObject(literal->mRawValue);
                if (gCurrentFrame) gCurrentFrame->addToGarbageCollection(strObj);
                finalValue = Value(strObj);
            }

            uint32_t constIdx = chunk.addConstant(finalValue);
            chunk.emit(OP_CONST);
            chunk.emit(constIdx);
        }

        //TODO ValueExpression

        // ---------------------------------------------------------------------
        // --- variable
        else if (auto* varExpr = dynamic_cast<VariableExpression*>(node)) {
                U32 localSlot = scope.insert(varExpr->mVariableNameSymbolId);

#ifdef DREIZEHN_BYTECODE_PORTING
                chunk.emit(OP_LOAD_VAR);
                chunk.emit(varExpr->mVariableNameSymbolId);
                chunk.emit(OP_SET_LOCAL);
                chunk.emit(localSlot);
#endif

                chunk.emit(OP_GET_LOCAL);
                chunk.emit(localSlot);
        }

        // TODO  MethodExpression
        // TODO  CallExpression

        // AssignStatement --------------------------------------------------------
        else if (auto* assignStmt = dynamic_cast<AssignStatement*>(node)) {
            compileExpression(assignStmt->mRhs.get(), chunk, scope);
            U32 localSlot = scope.insert(assignStmt->mVarNameSymbolId);
            chunk.emit(OP_SET_LOCAL);
            chunk.emit(localSlot);
        }
        // --- operation
        else if (auto* binary = dynamic_cast<BinaryOpExpression*>(node)) {
            compileExpression(binary->mLeft.get(), chunk, scope);
            compileExpression(binary->mRight.get(), chunk, scope);

            switch (binary->mOp) {
                case TokenType::Plus    : chunk.emit(OP_ADD); break;
                case TokenType::Minus   : chunk.emit(OP_SUB); break;
                case TokenType::Mul     : chunk.emit(OP_MUL); break;
                case TokenType::Div     : chunk.emit(OP_DIV); break;
                default: Tools::errorf("COMPILE ERROR BinaryOpExpression : unknown TokenType: %d", (int)binary->mOp);
            }
        }
        // --- .....
        //TODO BinaryExpression

        //TODO IfStatement
        //TODO ElseMarkerNode
        //TODO FunctionDefineStartNode
        //TODO FunctionDefineEndNode

        // ForStatement --------------------------------------------------------

        else if (auto* forStmt = dynamic_cast<ForStatement*>(node)) {
            if (!forStmt->mStartExpr || !forStmt->mEndExpr ) {
                Tools::errorf("COMPILE ERROR: invalid for borders\n");
                return;
            }

            // add iter values to registers
            U32 iteratorSlot = scope.insert(forStmt->mIteratorVarNameSymbolId);
            U32 endValueTmpSlot = scope.allocateTemporarySlot();

            // start to register
            compileExpression(forStmt->mStartExpr.get(), chunk, scope);
            chunk.emit(OP_SET_LOCAL);
            chunk.emit(iteratorSlot);

            // end to register
            compileExpression(forStmt->mEndExpr.get(), chunk, scope);
            chunk.emit(OP_SET_LOCAL);
            chunk.emit(endValueTmpSlot);

//FIXME this is not very performant !! and cant do down step

            U32 loopConditionAddr = (U32)chunk.mByteCodes.size();

            // ITER
            chunk.emit(OP_GET_LOCAL);
            chunk.emit(iteratorSlot);
            chunk.emit(OP_GET_LOCAL);
            chunk.emit(endValueTmpSlot);
            chunk.emit(OP_LESS_EQUAL);

            chunk.emit(OP_JUMP_IF_FALSE);
            U32 exitJumpPlaceholder = (U32)chunk.mByteCodes.size();
            chunk.emit(0); // space for backpatching

            for (auto& statement : forStmt->mBody) {
                compileExpression(statement.get(), chunk, scope);
            }

            chunk.emit(OP_INC_LOCAL);
            chunk.emit(iteratorSlot);
            chunk.emit(1); //for step :D

            // // chunk.emit(OP_GET_LOCAL);
            // // chunk.emit(iteratorSlot);
            // // chunk.emit(OP_CONST);
            // // //NOTE this is the iter increment value for future changes step / neg
            // // chunk.emit(chunk.addConstant(Value(1)));
            // // chunk.emit(OP_ADD);
            // // chunk.emit(OP_SET_LOCAL);
            // // chunk.emit(iteratorSlot);

            chunk.emit(OP_JUMP);
            chunk.emit(loopConditionAddr);

            // patch jmp
            U32 loopEndAddr = (U32)chunk.mByteCodes.size();
            chunk.mByteCodes[exitJumpPlaceholder] = loopEndAddr;

            scope.freeTemporarySlot();
        }


        //TODO BreakStatement
        //TODO ReturnStatement
        //TODO WhileStatement


    } // compileExpression
}; //ASTCompiler
} // namespace
