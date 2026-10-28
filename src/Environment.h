//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Enviroment VM
#pragma once
//-----------------------------------------------------------------------------
#ifdef DREIZEHN_DEBUG
// #define DEBUG_TRACE_EXECUTION
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
// #include "bytecode/VMStructure.h"
// #include "bytecode/CompilerScope.h"
// #include "bytecode/VM.h"

namespace DreiZehn {



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
    // EXECUTE :D - currentEnv for function calls
    // second should be faster for cpu but issnt so maybe a mix of both ?
    // -------------------------------------------------------------------------
#define AST_IP_SMALL_EXECUTE
#ifdef AST_IP_SMALL_EXECUTE

    inline FlowStatus execute(ASTNode* node, Environment& currentEnv) {
        if (!node) return FlowStatus(FlowSignal::None);

        #ifdef DREIZEHN_DEBUG
        if (Globals::gDumpStateNodes) Tools::printf("EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
        #endif

        // switch(node->mNodeType) {
        //     case NodeType::VariableExpression: static_cast<VariableExpression*>(node)->evaluate(currentEnv); break;
        //     // case NodeType::IfStatement: return static_cast<IfStatement*>(node)->execute(currentEnv); break;
        //     // case NodeType::ArrayVariableExpression:
        //     case NodeType::ObjectFieldExpression:  static_cast<ObjectFieldExpression*>(node)->evaluate(currentEnv); break;
        //     // case NodeType::BinaryInlineExpression
        //     // case NodeType::BinaryOpExpression
        //     // case NodeType::BinaryExpression
        //     // case NodeType::MethodExpression
        //     // case NodeType::CallExpression
        //     // case NodeType::LiteralExpression
        //
        //     default:
                if (node->mFlags & NodeFlag_Expression) [[likely]] {
                    auto* expr = static_cast<Expression*>(node);
                    expr->evaluate(currentEnv);
                } else if (node->mFlags & NodeFlag_Flow) {
                    auto* flowStmt = static_cast<FlowBaseStatement*>(node);
                    return flowStmt->execute(currentEnv);

                } else [[unlikely]] {

                    Tools::errorf("UNKNOWN Expression EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
                }
        // }

        return FlowStatus(FlowSignal::None);

    }

#else // AST_IP_SMALL_EXECUTE
    inline FlowStatus execute(ASTNode* node, Environment& currentEnv) {
        if (!node) return FlowStatus(FlowSignal::None);

#ifdef DREIZEHN_DEBUG
        if (Globals::gDumpStateNodes) Tools::printf("EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
#endif
        switch(node->mNodeType) {
            // --- Break Statement ---
            case NodeType::BreakStatement:
            {
                auto* flowStmt = static_cast<BreakStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ContinueStatement:
            {
                auto* flowStmt = static_cast<ContinueStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ReturnStatement:
            {
                auto* flowStmt = static_cast<ReturnStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ElIfStatement:
            case NodeType::IfStatement:
            {
                auto* flowStmt = static_cast<IfStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::RangeStatement:
            {
                auto* flowStmt = static_cast<ForRangeStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ForEachStatement:
            {
                auto* flowStmt = static_cast<ForEachStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::ForStatement:
            {
                auto* flowStmt = static_cast<ForStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::WhileStatement:
            {
                auto* flowStmt = static_cast<WhileStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            case NodeType::BlockStatement:
            {
                auto* flowStmt = static_cast<BlockStatement*>(node);
                return flowStmt->execute(currentEnv);
            }
            // --- Assign ---
            case NodeType::AssignStatement: {
                if (auto* assign = static_cast<AssignStatement*>(node)) {
                    assign->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::AssignOPStatement:
            {
                if (auto* assign = static_cast<AssignOPStatement*>(node)) {
                    assign->evaluate(currentEnv);
                }
                break;
            }
            // -------------- OTHERS -------------------
            case NodeType::LiteralExpression: {
                if (auto* expr = static_cast<LiteralExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::ValueExpression: {
                if (auto* expr = static_cast<ValueExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::VariableExpression: {
                if (auto* expr = static_cast<VariableExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::CallExpression: {
                if (auto* expr = static_cast<CallExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::MethodExpression: {
                if (auto* expr = static_cast<MethodExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::BinaryExpression: {
                if (auto* expr = static_cast<BinaryExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::BinaryOpExpression: {
                if (auto* expr = static_cast<BinaryOpExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::BinaryInlineExpression: {
                if (auto* expr = static_cast<BinaryInlineExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::ObjectFieldExpression: {
                if (auto* expr = static_cast<ObjectFieldExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::ArrayVariableExpression: {
                if (auto* expr = static_cast<ArrayVariableExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }

            case NodeType::IncludeStatement: {
                if (auto* expr = static_cast<IncludeStatement*>(node)) {
                    return expr->execute(currentEnv);
                }
                break;
            }

            case NodeType::KeyValueExpression: {
                if (auto* expr = static_cast<KeyValueExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::StringConcatExpression: {
                if (auto* expr = static_cast<StringConcatExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }
            case NodeType::StringConcatAssignExpression: {
                if (auto* expr = static_cast<StringConcatAssignExpression*>(node)) {
                    expr->evaluate(currentEnv);
                }
                break;
            }

            // --- others ---
            default: {
                // if (auto* expr = static_cast<Expression*>(node)) {
                //     expr->evaluate(currentEnv);
                // }
                Tools::errorf("UNKNOWN Expression EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
                break;
            }

        } // ... SWITCH ...

        return FlowStatus(FlowSignal::None);
    }
#endif

    // -------------------------------------------------------------------------
    // main execute
    inline FlowStatus execute(ASTNode* node) {
         return execute(node, *this);
    }
    // -------------------------------------------------------------------------
}; //Class
} //Namespace
