//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Function Map , TODO lookup by std::string is not the fastest!
//-----------------------------------------------------------------------------
#pragma once

#include <functional>
#include <vector>
#include <string>
#include "Value.h"
#include "toolbox/SymbolTable.h"
#include "AstNode.h"

namespace DreiZehn { class Environment; }
namespace DreiZehn::FunctionMap {
    using CallBack =  std::function< bool ( std::vector<Value>&, Value& )>;
    using FuncLookupMap = std::unordered_map<uint32_t, CallBack>;

    // Functions
    inline FuncLookupMap RegisteredFunctions;

    inline void RegisterFunction(const std::string& name, CallBack cb) {
        RegisteredFunctions[SymbolTable::insert(name) ] = cb;
    }

    inline bool IsCFunction (uint32_t symbolId) {
        return RegisteredFunctions.find(symbolId) != RegisteredFunctions.end();
    }

    inline CallBack* GetCFunction (uint32_t symbolId) {
        auto it = RegisteredFunctions.find(symbolId);
        if (it != RegisteredFunctions.end())
            return &it->second;
        else
            return nullptr;
    }

    // --------------- SCRIPT FUNCTION ----------------------

    struct ScriptFunction {
        std::vector<std::string> parameterNames;
        std::vector<std::shared_ptr<ASTNode>> body;
    };

    inline std::unordered_map<uint32_t, ScriptFunction> RegisteredScriptFunctions;

    inline bool IsScriptFunction(uint32_t symbolId) {
        return RegisteredScriptFunctions.find(symbolId) != RegisteredScriptFunctions.end();
    }

    inline ScriptFunction* GetScriptFunction (uint32_t symbolId) {
        auto it = RegisteredScriptFunctions.find(symbolId);
        if (it != RegisteredScriptFunctions.end())
            return &it->second;
        else
            return nullptr;
    }

    Value CallScriptFunction(Environment& env, const ScriptFunction* sf,std::vector<Value>& args );

    // -------------------------------------------------------------
    // combined for parser
    inline bool IsFunction (uint32_t symbolId) {
        return IsCFunction(symbolId) || IsScriptFunction(symbolId);
    }
    // -------------------------------------------------------------
    // Constants
    // -------------------------------------------------------------
    using ConstantsLookupMap = std::unordered_map<uint32_t, Value>;
    inline ConstantsLookupMap RegisteredConstants;

    inline void RegisterConstants(const std::string& name, Value value) {
        RegisteredConstants[SymbolTable::insert( name )] = value;
    }
    inline Value* getConstants( uint32_t symbolId) {
        auto it = RegisteredConstants.find(symbolId);
        if (it != RegisteredConstants.end())
            return &it->second;
        else
            return nullptr;
    }
     // -------------------------------------------------------------

} //namespace


