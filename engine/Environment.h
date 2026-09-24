//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Enviroment VM

// #define DREIZEHN_BYTECODE
// #define DREIZEHN_BYTECODE_PORTING
// #define DEBUG_TRACE_EXECUTION
//-----------------------------------------------------------------------------
#pragma once



#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

#include <unordered_map>
#include <iostream>
#include <functional>
#include <cassert>

#include "core/AstNode.h"
#include "core/FunctionMap.h"
#include "core/Value.h"
#include "core/VariableFrame.h"

#include "Globals.h"

#include "toolbox/SymbolTable.h"
#include "toolbox/Tools.h"

// Byte Code
#ifdef DREIZEHN_BYTECODE
#include "bytecode/VMStructure.h"
#include "bytecode/CompilerScope.h"
#include "bytecode/ASTCompiler.h"
#include "bytecode/VM.h"
#endif

namespace DreiZehn {


    enum class BlockType { Function, ForLoop, WhileLoop, IfBlock };

    struct OpenBlock {
        BlockType mType;
        uint32_t mFuncNameSymbolId;
        BlockStatement* mBlockNodePointer;
    };


    enum class FlowSignal {
        None,
        Break,
        Return
    };



class Environment {
private:

    Environment* mParentEnv = nullptr;
    VariableFrame* mVariableFrame = nullptr;
public:
    Environment() : mParentEnv(nullptr) {
        Globals::gCurEnv = this;
        mVariableFrame = new VariableFrame(nullptr);
    }
    Environment(Environment* parentEnv) : mParentEnv(parentEnv) {
        Globals::gCurEnv = this;
        mVariableFrame = new VariableFrame(parentEnv->mVariableFrame);
    }
    ~Environment() {
        if (mParentEnv) Globals::gCurEnv = mParentEnv;
        else Globals::gCurEnv = nullptr;

        if (mVariableFrame) {
            delete(mVariableFrame);
            mVariableFrame = nullptr;
        }
    }

    VariableFrame* getVariableFrame() {
        assert(mVariableFrame && "FATAL ERROR: Enviroment require a VariableFrame!");
        return mVariableFrame;
    }


    // -------------------------------------------------------------------------
    // EXECUTE :D - currentEnv for function calls
    // -------------------------------------------------------------------------
    inline FlowSignal execute(ASTNode* node, Environment& currentEnv) {
        if (!node) return FlowSignal::None;

        if (Globals::gDumpStateNodes) Tools::printf("EXECUTE: %s\n", NodeTypeToString(node->mNodeType));

        switch(node->mNodeType) {
            // --- Break Statement ---
            case NodeType::BreakStatement: {
                return FlowSignal::Break;
            }

            // --- Return Statement ---
            case NodeType::ReturnStatement: {
                auto* retStmt = dynamic_cast<ReturnStatement*>(node);
                if (retStmt->mExpression) {
                    Value retVal = retStmt->mExpression->evaluate(currentEnv);
                    currentEnv.mVariableFrame->setVariable(SymbolTable::insert("__return_value__"), retVal);
                }
                return FlowSignal::Return;
            }
            // --- Assign ---
            case NodeType::AssignStatement: {
                #ifdef DREIZEHN_BYTECODE
                if (auto* assign = dynamic_cast<AssignStatement*>(node)) {
                    // new chunk
                    BytecodeChunk chunk;
                    CompilerScope scope;

                    // compile
                    ASTCompiler::compileExpression(assign->mRhs.get(), chunk, scope);

                    chunk.emit(OP_EXIT);

                    // fire!
                    Value result = runDirectThreadedVM(chunk, scope.getLocalCount());
                    // save
                    currentEnv.mVariableFrame->setVariable(assign->mVarNameSymbolId, result);
                }
                #else
                if (auto* assign = dynamic_cast<AssignStatement*>(node)) {
                    currentEnv.mVariableFrame->setVariable(assign->mVarNameSymbolId, assign->mRhs->evaluate(currentEnv));
                }
                #endif
                break;
            }
            // ---- AssignOPStatement
            case NodeType::AssignOPStatement: {
                if (auto* assignOP = dynamic_cast<AssignOPStatement*>(node)) {
                    Value* valuePtr = currentEnv.mVariableFrame->getVariablePtr(assignOP->mVarNameSymbolId);
                    Value rightHand = assignOP->mRhs->evaluate(currentEnv);
                    if (valuePtr->isPointer() ) {
                        Tools::PrintRuntimeError("Operation with pointer not allowed!\n");
                        break;
                    }
                    if (valuePtr->isInt() && rightHand.isInt()) {
                        int32_t intval = valuePtr->asInt();
                        switch(assignOP->mOp) {
                            case TokenType::AssignPlus:  intval += rightHand.getInt(); break;
                            case TokenType::AssignMinus: intval -= rightHand.getInt(); break;
                            case TokenType::AssignMul:   intval *= rightHand.getInt(); break;
                            case TokenType::AssignDiv:  if (rightHand.getInt() != 0) {intval /= rightHand.getInt();} break;
                            default: break;
                        }
                        *valuePtr = Value(intval);
                    } else {
                        double doubleval = valuePtr->getDouble();
                        switch(assignOP->mOp) {
                            case TokenType::AssignPlus:  doubleval += rightHand.getDouble(); break;
                            case TokenType::AssignMinus: doubleval -= rightHand.getDouble(); break;
                            case TokenType::AssignMul:   doubleval *= rightHand.getDouble(); break;
                            case TokenType::AssignDiv:  if (rightHand.getDouble() != 0.0) {doubleval /= rightHand.getDouble();} break;
                            default: break;
                        }
                        *valuePtr = Value(doubleval);
                    }
                }
                break;
            }
            // --- If-Statement  ---
            case NodeType::IfStatement: {
                auto* ifStmt = dynamic_cast<IfStatement*>(node);
                Value condVal = ifStmt->mCondition->evaluate(currentEnv);

                double condNum = condVal.getDouble();
                const double EPSILON = 1e-9;
                bool isTrue = std::abs(condNum) > EPSILON;

                if (isTrue) {
                    for (auto& childNode : ifStmt->mBody) {
                        if (!childNode) continue;
                        FlowSignal sig = execute(childNode.get(), currentEnv);
                        if (sig != FlowSignal::None) return sig;
                    }
                } else {
                    for (auto& childNode : ifStmt->mElseBody) {
                        if (!childNode) continue;
                        FlowSignal sig = execute(childNode.get(), currentEnv);
                        if (sig != FlowSignal::None) return sig;
                    }
                }
                break;
            }

            // ---- for statement .....
            case NodeType::ForStatement: {
                #ifdef DREIZEHN_BYTECODE
                    auto* forStmt = dynamic_cast<ForStatement*>(node);
                    BytecodeChunk chunk;
                    CompilerScope scope;

                    ASTCompiler::compileExpression(forStmt, chunk, scope);

                    //TODO: ASTCompiler::compileForStatement(forStmt, chunk, scope);

                    chunk.emit(OP_EXIT);

                    runDirectThreadedVM(chunk, scope.getLocalCount());
                #else
                    auto* forStmt = dynamic_cast<ForStatement*>(node);
                    if (!forStmt->mStartExpr || !forStmt->mEndExpr ) {
                        Tools::errorf("Runtime Error: invalid for borders!\n");
                        return FlowSignal::None;
                    }
                    Value startVal = forStmt->mStartExpr->evaluate(currentEnv);
                    Value endVal = forStmt->mEndExpr->evaluate(currentEnv);

                    int start = startVal.getInt();
                    int end = endVal.getInt();

                    Environment loopEnv(&currentEnv);

                    if (start > end ) {
                        for (int i = start; i >= end; --i) {
                            loopEnv.mVariableFrame->setVariable(forStmt->mIteratorVarNameSymbolId, Value(i));

                            for (auto& statement : forStmt->mBody) {
                                FlowSignal sig = currentEnv.execute(statement.get(), loopEnv);

                                if (sig == FlowSignal::Break) {
                                    return FlowSignal::None;
                                }
                                if (sig == FlowSignal::Return) {
                                    return FlowSignal::Return;
                                }
                            }
                        }

                    } else {
                        for (int i = start; i <= end; ++i) {
                            loopEnv.mVariableFrame->setVariable(forStmt->mIteratorVarNameSymbolId, Value(i));

                            for (auto& statement : forStmt->mBody) {
                                FlowSignal sig = currentEnv.execute(statement.get(), loopEnv);

                                if (sig == FlowSignal::Break) {
                                    return FlowSignal::None;
                                }
                                if (sig == FlowSignal::Return) {
                                    return FlowSignal::Return;
                                }
                            }
                        }
                    }
                #endif
                break;
            }

            // ---- While statement .....
            case NodeType::WhileStatement: {
                auto* whileStmt = dynamic_cast<WhileStatement*>(node);
                Environment loopEnv(&currentEnv);

                auto checkCondition = [&]() -> bool {
                    Value condVal = whileStmt->mCondition->evaluate(loopEnv);
                    return (condVal.isInt() && condVal.asInt() != 0) ||
                    (condVal.isDouble() && condVal.asDouble() != 0.0);
                };

                while (checkCondition()) {
                    for (auto& statement : whileStmt->mBody) {
                        FlowSignal sig = currentEnv.execute(statement.get(), loopEnv);

                        if (sig == FlowSignal::Break) return FlowSignal::None;
                        if (sig == FlowSignal::Return) return FlowSignal::Return;
                    }
                }
                break;
            }
            // --- BlockStatement  ---
            case NodeType::BlockStatement: {
                auto* block = dynamic_cast<BlockStatement*>(node);
                for (auto& statement : block->mBody) {
                    if (!statement) continue;
                    FlowSignal sig = execute(statement.get(), *this);
                    if (sig != FlowSignal::None) return sig;
                }
                return FlowSignal::None;
            }

            // --- others ---
            default: {
                if (auto* expr = dynamic_cast<Expression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }

        } // ... SWITCH ...

        return FlowSignal::None;
    }


    // -------------------------------------------------------------------------
    // main execute
    inline FlowSignal execute(ASTNode* node) {
         return execute(node, *this);
    }
    // -------------------------------------------------------------------------
    void shutDown() {
        // done be destuctor: doGarbageCollection();
    }
}; //Class
} //Namespace
