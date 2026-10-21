//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Enviroment VM
#pragma once
//-----------------------------------------------------------------------------
#ifdef DREIZEHN_DEBUG
#define DEBUG_TRACE_EXECUTION
#endif
//-----------------------------------------------------------------------------
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
#include "bytecode/VMStructure.h"
#include "bytecode/CompilerScope.h"
#include "bytecode/VM.h"

namespace DreiZehn {

    enum class BlockType { Function, ForLoop, WhileLoop, IfBlock };

    struct OpenBlock {
        BlockType mType;
        uint32_t mFuncNameSymbolId;
        BlockStatement* mBlockNodePointer;
    };


class Environment {
private:

    Environment* mParentEnv = nullptr;
    VariableFrame* mVariableFrame = nullptr;
public:
    Environment() : mParentEnv(nullptr) {
        Globals::gCurEnv = this;
        Globals::gMainEnv = this;
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

    inline VariableFrame* getVariableFrame() {
        // assert(mVariableFrame && "FATAL ERROR: Enviroment require a VariableFrame!");
        return mVariableFrame;
    }

    // -------------------------------------------------------------------------
    // Compile and CompileExec
    // -------------------------------------------------------------------------

    inline Value CompileExec(ASTNode* node) {
        if (!node) return NumberNullValue;
        BytecodeChunk chunk;
        CompilerScope scope;
        if (!Compile(node, *this, chunk, scope)) return NumberNullValue;

        Value res = runDirectThreadedVM(chunk, scope.getLocalCount());
        #ifdef DEBUG_TRACE_EXECUTION
        Tools::printf("*** CompileExec VALUE IS: %s\n", res.toString().c_str());
        #endif
        return res;
    }

    inline bool Compile( ASTNode* node, Environment& env, BytecodeChunk& chunk, CompilerScope& scope)
    {
        if (!node) return false;

// FIXME FIXME
//  PRECOMPILE CANT SEE THE GLOBALS !!!!, when i put it on the back the others cant see the functions ?!
//  I NEED TO PUT THEM INSIDE IN CORRECT ORDER
// FIXME FIXME

        // chunk.emit(OP_JUMP);
        // U32 globalStartPatch = chunk.tell();
        // chunk.emit(0);
        // for (auto& [symbolId, func] : FunctionMap::RegisteredScriptFunctions) {
        //     func.bytecodeAddress = chunk.tell();
        //
        //     for (const auto& paramName : func.parameterNames) {
        //         scope.insert(SymbolTable::insert(paramName));
        //     }
        //
        //     for (auto& statement : func.body) {
        //         statement->compile(env, chunk, scope, ValueType::NONE);
        //     }
        //
        //     chunk.emit(OP_PUSH_VALUE);
        //     chunk.emitValue(NumberNullValue);
        //     chunk.emit(OP_RETURN);
        // }
        // U32 mainScriptStartIP = chunk.tell();
        // chunk.patch(globalStartPatch, mainScriptStartIP);

        // CompilerScope globalScope;

        mFunctionPatches.clear();

        switch(node->mNodeType) {
            case NodeType::BlockStatement:
            {
                auto* flowStmt = dynamic_cast<BlockStatement*>(node);
                if (!flowStmt->compile(env, chunk, scope, ValueType::NONE)) return false;
                break;
            }
            default:
                Tools::errorf("UNKNOWN Expression Compile: %s\n", NodeTypeToString(node->mNodeType));
                return false;
        }


        chunk.emit(OP_EXIT);

        /// TEST added scripts on back of byte code after EXIT
        // LOL ON THE BACK I DONT HAVE THE **bytecodeAddress** on a function call!
        for (auto& [symbolId, func] : FunctionMap::RegisteredScriptFunctions) {
            func.bytecodeAddress = chunk.tell();

            CompilerScope funcScope;

            for (const auto& paramName : func.parameterNames) {
                funcScope.insert(SymbolTable::insert(paramName));
            }

            for (auto& statement : func.body) {
                statement->compile(env, chunk, funcScope, ValueType::NONE);
            }

            chunk.emit(OP_PUSH_VALUE);
            chunk.emitValue(NumberNullValue);
            chunk.emit(OP_RETURN);

            func.localVarCount = funcScope.getLocalCount();
        }

        // Apply function patches - what a HACK fest
        for (auto& patch: mFunctionPatches) {
            if (patch.mPatchType == FuncPatchType::Adress)
                chunk.patch(patch.mPatchByteCodePos, patch.mFuncPtr->bytecodeAddress);
            else if (patch.mPatchType == FuncPatchType::VarCount)
                chunk.patch(patch.mPatchByteCodePos, patch.mFuncPtr->localVarCount);
            else
                assert(true && "FunctionPatches unknown PathType!!!");
        }
        mFunctionPatches.clear();

        return true;
    }

    // -------------------------------------------------------------------------
    // EXECUTE :D - currentEnv for function calls
    // -------------------------------------------------------------------------
    inline FlowStatus execute(ASTNode* node, Environment& currentEnv) {
        if (!node) return FlowStatus(FlowSignal::None);

        if (Globals::gDumpStateNodes) Tools::printf("EXECUTE: %s\n", NodeTypeToString(node->mNodeType));

        switch(node->mNodeType) {
            // --- Break Statement ---
            case NodeType::BreakStatement:
            {
                auto* flowStmt = dynamic_cast<BreakStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ContinueStatement:
            {
                auto* flowStmt = dynamic_cast<ContinueStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ReturnStatement:
            {
                auto* flowStmt = dynamic_cast<ReturnStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ElIfStatement:
            case NodeType::IfStatement:
            {
                auto* flowStmt = dynamic_cast<IfStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::RangeStatement:
            {
                auto* flowStmt = dynamic_cast<ForRangeStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ForEachStatement:
            {
                auto* flowStmt = dynamic_cast<ForEachStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ForStatement:
            {
                auto* flowStmt = dynamic_cast<ForStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::WhileStatement:
            {
                auto* flowStmt = dynamic_cast<WhileStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::BlockStatement:
            {
                auto* flowStmt = dynamic_cast<BlockStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            // --- Assign ---
            case NodeType::AssignStatement: {
                if (auto* assign = dynamic_cast<AssignStatement*>(node)) {
                    assign->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::AssignOPStatement:
            {
                if (auto* assign = dynamic_cast<AssignOPStatement*>(node)) {
                    assign->evaluate(currentEnv);
                }
                break;
            }
            // -------------- OTHERS -------------------
            case NodeType::LiteralExpression: {
                if (auto* expr = dynamic_cast<LiteralExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::ValueExpression: {
                if (auto* expr = dynamic_cast<ValueExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::VariableExpression: {
                if (auto* expr = dynamic_cast<VariableExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::CallExpression: {
                if (auto* expr = dynamic_cast<CallExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::MethodExpression: {
                if (auto* expr = dynamic_cast<MethodExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::BinaryExpression: {
                if (auto* expr = dynamic_cast<BinaryExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::BinaryOpExpression: {
                if (auto* expr = dynamic_cast<BinaryOpExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::BinaryInlineExpression: {
                if (auto* expr = dynamic_cast<BinaryInlineExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::ObjectFieldExpression: {
                if (auto* expr = dynamic_cast<ObjectFieldExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::ArrayVariableExpression: {
                if (auto* expr = dynamic_cast<ArrayVariableExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }

            case NodeType::IncludeStatement: {
                if (auto* expr = dynamic_cast<IncludeStatement*>(node)) {
                    return expr->execute(currentEnv);
                }
                break;
            }

            case NodeType::KeyValueExpression: {
                if (auto* expr = dynamic_cast<KeyValueExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }


            // --- others ---
            default: {
                // if (auto* expr = dynamic_cast<Expression*>(node)) {
                //     expr->evaluate(currentEnv);
                // }
                Tools::errorf("UNKNOWN Expression EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
                break;
            }

        } // ... SWITCH ...

        return FlowStatus(FlowSignal::None);
    }


    // -------------------------------------------------------------------------
    // main execute
    inline FlowStatus execute(ASTNode* node) {
         return execute(node, *this);
    }
    // -------------------------------------------------------------------------
    void shutDown() {
        // done be destructor: doGarbageCollection();
    }
}; //Class
} //Namespace
