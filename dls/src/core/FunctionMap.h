//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Function Map
//-----------------------------------------------------------------------------
#pragma once

#include <functional>
#include <vector>
#include <span>
#include <string>
#include "Value.h"
#include "toolbox/SymbolTable.h"
#include "toolbox/Tools.h"

namespace DLS::FunctionMap {
    using CallBack =  std::function< bool ( std::span<Value>, Value& )>;
    using FuncLookupMap = std::unordered_map<uint32_t, CallBack>;

    // symbolId, stringId
    using DescLookupMap = std::unordered_map<uint32_t, uint32_t>;


    // Functions
    inline FuncLookupMap RegisteredFunctions;
    inline DescLookupMap RegisteredFunctionsDesc;

    inline void RegisterFunction(const std::string& name, CallBack cb, std::string desc = "") {
        uint32_t symId = SymbolTable::insert(name);
        RegisteredFunctions[symId ] = cb;
        if (!desc.empty()) {
            uint32_t strId = StringTable::insert(desc);
            RegisteredFunctionsDesc[symId] = strId;
        }
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

    inline uint32_t getFunctionDescStrId(uint32_t symbolId) {
        auto it = RegisteredFunctionsDesc.find(symbolId);
        if (it != RegisteredFunctionsDesc.end()) {
            return it->second;
        }
        return 0;
    }


    // -------------------------------------------------------------
    // combined for parser
    inline bool IsFunction (uint32_t symbolId) {
        return IsCFunction(symbolId) ;
    }
    // -------------------------------------------------------------
    // Constants
    // -------------------------------------------------------------
    using ConstantsLookupMap = std::unordered_map<uint32_t, Value>;
    inline ConstantsLookupMap RegisteredConstants;

    inline Value* getConstants( uint32_t symbolId) {
        auto it = RegisteredConstants.find(symbolId);
        if (it != RegisteredConstants.end())
            return &it->second;
        else
            return nullptr;
    }
    inline void RegisterConstants(const std::string& name, Value value) {
        U32 symId = SymbolTable::insert( name );
        Value* curConst = getConstants(symId);

        // FIXME if i add also objects but not sure.
        // if (curConst && curConst->isPointer()) {
        //     curConst->asPointerObject()->setAssigned(false);
        // }
        // if (value.isPointer()) {
        //     value.asPointerObject()->setAssigned(true);
        // }
        RegisteredConstants[symId] = value;
    }

    #define REGISTER_CONST(constName) RegisterConstants(#constName, Value(constName))


} //namespace DLS::FunctionMap


