//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Dirty Little Script
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>

#include "core/VM.h"
#include "core/Lexer.h"
#include "core/Parser.h"
#include "toolbox/Tools.h"

#include <fstream>
#include <istream>

namespace DLS {
    bool DEBUG_TOKEN = false;

    // -------------------------------------------------------------------------
    // Run a Script
    // -------------------------------------------------------------------------
    std::string Run(std::string code) {
        Lexer lex(code);
        std::vector<Token> token = lex.tokenize();
        if (DEBUG_TOKEN) {
            for (auto& tok: token) {
                std::cout << tokenTypeToString(tok.mType) << "  ::  " << tok.mValue << "\n";
            }
        }

        VM vm;
        Parser parser(token,&vm);
        if (!parser.parse()) {
            return "ERROR!";
        }
        vm.run();
        std::string result = "";
        int iter = 0;
        for (auto val: vm.stack) {
            if (iter > 0) result = result  + ", ";
            result = result + val.toString();
            iter++;
        }
        return result;
    }
    // -------------------------------------------------------------------------
    // Load a Script file
    // -------------------------------------------------------------------------
    std::string loadScript(const std::string& scriptPath) {
        std::ifstream file(scriptPath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            Tools::errorf("Script file cant be opened: %s\n", scriptPath.c_str());
            return "";
        }
        return std::string((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    }
    // -------------------------------------------------------------------------
    // Init the base functions and constants
    // -------------------------------------------------------------------------
    void Init( ) {
        static bool registered = false; if (registered) return; registered = true;

        using namespace FunctionMap;

        // ---------------------------------------------------------------------
        // also push some CORE Constants here:
        RegisterConstants("true", Value(1));
        RegisterConstants("false", Value(0));

        // ---------------------------------------------------------------------

        RegisterConstants("type::float",    (int32_t)ValueType::Double);
        RegisterConstants("type::integer",  (int32_t)ValueType::Long);
        RegisterConstants("type::pointer",  (int32_t)ValueType::Pointer);
        RegisterConstants("type::array",    (int32_t)ValueType::Array);
        RegisterConstants("type::string",   (int32_t)ValueType::String);
        RegisterConstants("type::function", (int32_t)ValueType::Function);

        RegisterFunction("getType", [](std::span<Value> args, Value& ret ) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: getType value. return the type id of the expression. (Example: Type::float)");
                ret =  Value(-1);
                return true;
            }
            ret = Value((int) args[0].type);
            return true;
        } ,"Usage: getType value. return the type id of the expression. (Example: Type::float)");



        // ---------------------------------------------------------------------
        // -------- print --------------
        // ---------------------------------------------------------------------
        RegisterFunction("print", [](std::span<Value> args, Value& ret) -> bool {
            std::string line = "";
            for ( auto& value : args) {
                line = line + value.toString() + " ";
            }
            Tools::printf("%s\n", line.c_str());
            return true;
        }, "Print a line to console. @param value value value ...");

        RegisterFunction("printraw", [](std::span<Value> args, Value& ret) -> bool {
            std::string line = "";
            for ( auto& value : args) {
                line = line + value.toString() + " ";
            }
            Tools::printf("%s", line.c_str());
            return true;
        }, "Print a line without linefeed to console. @param value value value ...");

        RegisterFunction("error", [](std::span<Value> args, Value& ret) -> bool {
            std::string line = "";
            for ( auto& value : args) {
                line = line + value.toString() + " ";
            }
            Tools::errorf("%s\n", line.c_str());
            return true;
        }, "Print a error to console. @param value value value ...");
        // ---------------------------------------------------------------------
        // int
        // ---------------------------------------------------------------------
        RegisterFunction("int::cast", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: int::cast value\n");
                return false;
            }
            if (args[0].isInt()) ret =  args[0];
            else
                if (args[0].isDouble()) ret = Value(args[0].getInt());
                else
                if (args[0].isString()) ret = Value( std::atoi( args[0].getString().c_str()));
                else
                Tools::errorf("Cant cast to integer!\n");

            return true;
        }, "cast any type to integer.");


        // ---------------------------------------------------------------------
        // float
        // ---------------------------------------------------------------------
        RegisterFunction("float::cast", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: float::cast value\n");
                return false;
            }
            if (args[0].isDouble()) ret =  args[0];
            else
                if (args[0].isInt()) ret = Value(args[0].getDouble());
                else
                if (args[0].isString()) ret = Value( std::atof( args[0].getString().c_str()));
                else
                Tools::errorf("Cant cast to float!\n");

            return true;
        }, "cast any type to float");
    }

} // namespace DLS
