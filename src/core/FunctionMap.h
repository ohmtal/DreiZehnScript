//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Function Map
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

    // ---------------------------------------------------------------------
    inline bool ArgsCheckString( const char* command,std::vector<Value>& args, uint32_t count) {
        if (args.size() < count ) {
            Tools::errorf("Usage: %s  [string value] %d times",command, count);
            return false;
        }

        for (int i = 0; i < count; i++) {
            if (!args[i].isStringId() ) {
                Tools::errorf("Usage: %s  [string value] %d times",command, count);
                return false;
            }
        }

        return true;
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
        if (value.isPointer()) {
            value.asPointerObject()->setAssigned(true);
        }
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
            ModuleCallback callback = nullptr;
            bool isLoaded = false;

            int load(Environment& env) {
                if (isLoaded) return 2;
                if (!callback) return 0;
                callback(env);
                isLoaded = true;
                return 1;
            }
        };
        std::unordered_map<std::string, ModuleDef> modules;

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

        inline static bool Load(std::string moduleName,  Environment& env) {
            return get().internalLoad(moduleName, env);
        }

        inline static bool LoadAll(Environment& env) {
            return get().internalLoadAll(env);
        }


        inline static bool isLoaded(std::string moduleName) {
            return get().internalIsLoaded(moduleName);
        }

    protected:
        bool internalRegister(const std::string& name, ModuleCallback callback) {
            uint32_t symId = SymbolTable::insert(name);
            if (internalIsRegistered(name)) {
                Tools::errorf("Module %s already registered!", name.c_str());
                return false;
            }

            ModuleDef def;
            def.name = name;
            def.callback = callback;
            def.isLoaded = false;
            modules[name] = def;
            return symId;
        }

        bool internalIsRegistered(std::string moduleName) {
            auto it = modules.find(moduleName);
            return  ( it != modules.end() );
        }

        bool internalIsLoaded(std::string moduleName) {
            auto it = modules.find(moduleName);
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
            // Tools::printf("Import all modules:\n");
            for ( auto& [key, value] : modules)  {
                int res = value.load(env);
                // if (res == 1) Tools::printf("     - %s: %s\n", value.name.c_str(), "OK");
                // else if (res == 0) Tools::printf("     - %s: %s\n", value.name.c_str(), "FAIL");
            }
            return true;
        }
        bool internalLoad(std::string moduleName, Environment& env) {
            auto it = modules.find(moduleName);
            if (it != modules.end()) {
                int res = it->second.load(env);
                // if (res == 1)  Tools::printf("Import: %s: %s\n", it->second.name.c_str(), "OK");
                // else if (res == 0) Tools::printf("Import: %s: %s\n", it->second.name.c_str(), "FAIL");
                return res != 0;
            }
            return false;
        }
    };

} //namespace DreiZehn::FunctionMap


