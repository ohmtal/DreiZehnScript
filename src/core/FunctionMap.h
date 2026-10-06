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
    // List of registed modules
    // -------------------------------------------------------------
    typedef void(*ModuleCallback)(Environment& env);

    class ModuleRegistry {
    private:
        struct ModuleDef {
            std::string name = "";
            uint32_t    symbolId  = 0;
            ModuleCallback callback = nullptr;
            bool isLoaded = false;

            bool load(Environment& env) {
                if (isLoaded) return true;
                if (!callback) return false;
                callback(env);
                isLoaded = true;
                return true;
            }
        };
        std::unordered_map<uint32_t, ModuleDef> modules;

        static ModuleRegistry& get() {
            static ModuleRegistry instance;
            return instance;
        }
    public:
        inline static uint32_t Register(const std::string& name, ModuleCallback callback) {
            return get().internalRegister(name, callback);
        }

        inline static bool Print() {
            return get().internalPrint();
        }

        inline static bool Load(uint32_t moduleSymbolId,  Environment& env) {
            return get().internalLoad(moduleSymbolId, env);
        }

        inline static bool LoadAll(Environment& env) {
            return get().internalLoadAll(env);
        }


        inline static bool isLoaded(uint32_t moduleSymbolId) {
            return get().internalIsLoaded(moduleSymbolId);
        }

    protected:
        uint32_t internalRegister(const std::string& name, ModuleCallback callback) {
            uint32_t symId = SymbolTable::insert(name);
            assert(!internalIsRegistered(symId) && "Module already registered!");

            ModuleDef def;
            std::string constantName = "Module::";
            constantName = constantName + name;
            RegisterConstants(constantName, symId);
            def.name = constantName;
            def.symbolId = symId;
            def.callback = callback;
            def.isLoaded = false;
            modules[symId] = def;
            return symId;
        }

        bool internalIsRegistered(uint32_t moduleSymbolId) {
            auto it = modules.find(moduleSymbolId);
            return  ( it != modules.end() );
        }

        bool internalIsLoaded(uint32_t moduleSymbolId) {
            auto it = modules.find(moduleSymbolId);
            return  (it != modules.end() && it->second.isLoaded);
        }

        bool internalPrint( ) {
            Tools::printf("Module Registry:\n");
            for ( auto& [key, value] : modules)  {
                Tools::printf("  [%10s] %s\n", value.isLoaded ? "imported": "available", value.name.c_str() );
            }
            return true;
        }

        bool internalLoadAll( Environment& env) {
            Tools::printf("Import all modules:\n");
            for ( auto& [key, value] : modules)  {
                Tools::printf("     - %s: %s\n", value.name.c_str(), value.load(env)? "OK": "FAIL");
            }
            return true;
        }
        bool internalLoad(uint32_t moduleSymbolId, Environment& env) {
            auto it = modules.find(moduleSymbolId);
            if (it != modules.end()) {
                bool result = it->second.load(env);
                Tools::printf("Import: %s: %s\n", it->second.name.c_str(), result ? "OK": "FAIL");
                return result;
            }
            return false;
        }
    };




} //namespace DreiZehn::FunctionMap


