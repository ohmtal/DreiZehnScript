//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Enviroment VM
// TODO add cpp!
//-----------------------------------------------------------------------------
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
#include "bytecode/VM.h"


namespace DreiZehn {


class Environment {
private:

    Environment* mParentEnv = nullptr;
    VariableFrame* mVariableFrame = nullptr;


    // -- Ast only locals Register --
    std::vector<size_t> mLocalVarCount;
    U32 mLocalsFrameId = 0;
    size_t mLocalOffset = 0;
    ValueVector mLocalsRegister;


    // ----
    // bad idea here ?!
    VM mVM;


public:
    // -----
    std::unordered_map<uint32_t, size_t> mGlobalsLookUp;
    ValueVector mGlobalsRegister;
    // -----

    Environment() : mParentEnv(nullptr) {
        // mVariableFrame = new VariableFrame(nullptr);
    }
    Environment(Environment* parentEnv) : mParentEnv(parentEnv) {
        // mVariableFrame = new VariableFrame(parentEnv->mVariableFrame);
    }
    ~Environment() {
        // if (mVariableFrame) {
        //     delete(mVariableFrame);
        //     mVariableFrame = nullptr;
        // }
    }

    // inline VariableFrame* getVariableFrame() {
    //     // assert(mVariableFrame && "FATAL ERROR: Enviroment require a VariableFrame!");
    //     return mVariableFrame;
    // }

    inline void PrintRuntimeError(const std::string& message, const std::string& p2 = "", const std::string& p3 = ""){
        Tools::errorf("RunTime Error [ENV:%p]: %s %s %s", (void*) this, message.c_str(), p2.c_str(),p3.c_str());
    }

    inline VM& getVM() { return mVM; } //const ?! ..


    // -------------------------------------------------------------------------
    // GlobalsRegister
    // -------------------------------------------------------------------------
    inline S64 getGlobalRegister(U32 symId) {
        auto it = mGlobalsLookUp.find(symId);
        if (it != mGlobalsLookUp.end()) [[likely]] {
            return it->second;
        }
        return -1;
    }

    inline size_t reserveGlobal(U32 symId) {
        S64 index = getGlobalRegister(symId);
        if (index < 0 ) {
            index = mGlobalsRegister.size();
            mGlobalsLookUp[symId] = index;
            mGlobalsRegister.push_back(Value());
        }
        return (size_t) index;
    }

    inline void saveGlobal(size_t index, const Value& value) {
        #ifdef DLS_DEBUG
        assert (index < mGlobalsRegister.size() && "SAVE: GLOBALS REGISTER OUT OF BOUNDS");
        #endif
        mGlobalsRegister[index] = value;
    }

    inline Value& getGlobal(size_t index) {
        #ifdef DLS_DEBUG
        assert (index < mGlobalsRegister.size() && "LOAD: GLOBALS REGISTER OUT OF BOUNDS");
        #endif
        return mGlobalsRegister[index];
    }

    inline bool getGlobalVariableValueRef(const std::string& varName, Value& ret) {
        S64 index = getGlobalRegister(SymbolTable::insert(varName));
        if (index < 0 ) return false;
        ret = getGlobal((size_t)index);
        return true;
    }
    // -------------------------------------------------------------------------
    // LocalRegister
    // -------------------------------------------------------------------------
    inline U32 getLocalsFrameId() {return  mLocalsFrameId;}

    inline void pushLocalStack(size_t varCount) {
        mLocalVarCount.push_back(varCount);
        mLocalOffset = mLocalsRegister.size();
        mLocalsRegister.resize(mLocalOffset  + varCount);
        mLocalsFrameId++;
    }
    inline void popLocalStack() {
        size_t newSize = mLocalsRegister.size() - mLocalVarCount.back();
        mLocalVarCount.pop_back();
        mLocalsRegister.resize(newSize);
        if (mLocalVarCount.size() > 0 ) mLocalOffset -= mLocalVarCount.back();
        else mLocalOffset = 0;
    }

    inline void saveLocal(size_t index, const Value& value) {
        // #ifdef DREIZEHN_DEBUG
        // Tools::printf("saveLocal [%zu]:%s\n", index, value.toString().c_str() );
        // #endif
        assert ((index + mLocalOffset) < mLocalsRegister.size() && "SAVE: LOCALS REGISTER OUT OF BOUNDS");
        mLocalsRegister[(index + mLocalOffset)] = value;
    }

    inline Value& getLocal(size_t index) {
        #ifdef DREIZEHN_DEBUG
        if ((index + mLocalOffset) >= mLocalsRegister.size()){
            Tools::errorf("LOAD: LOCALS REGISTER OUT OF BOUNDS");
            assert(true);
        }
        Value& value = mLocalsRegister[(index + mLocalOffset)];
        // Tools::printf("LoadLocal [%zu]:%s\n", index, value.toString().c_str() );
        return value;
        #endif
        // #ifdef DLS_DEBUG
        assert ((index + mLocalOffset) < mLocalsRegister.size() && "LOAD: LOCALS REGISTER OUT OF BOUNDS");
        // #endif
        return mLocalsRegister[(index + mLocalOffset)];
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

        switch(node->mNodeType) {
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
            case NodeType::BlockStatement:
            {
                auto* flowStmt = static_cast<BlockStatement*>(node);
                currentEnv.pushLocalStack(flowStmt->VarCount);
                FlowStatus result =  flowStmt->execute(currentEnv);
                popLocalStack();
                return result;
            }

            default:
                if (node->mFlags & NodeFlag_Expression) [[likely]] {
                    auto* expr = static_cast<Expression*>(node);
                    expr->evaluate(currentEnv);
                } else if (node->mFlags & NodeFlag_Flow) {
                    auto* flowStmt = static_cast<FlowBaseStatement*>(node);
                    return flowStmt->execute(currentEnv);
                } /*I use it for functionstart node so no error! ==> else [[unlikely]] {
                    Tools::errorf("UNKNOWN Expression EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
                }*/
        }

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
