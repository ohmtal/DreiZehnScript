//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Core Commands
//-----------------------------------------------------------------------------
#pragma once
#include <ctime>
#include <cmath>
#include "core/FunctionMap.h"
#include "ScriptLoader.h"

namespace DreiZehn {
    void RegisterMathFunctions() {
        std::srand(std::time(nullptr)); // setup random seed

        using namespace FunctionMap;

        // ---------------------------------------------------------------------
        constexpr double PI = 3.14159265358979323846;
        RegisterConstants("math:pi", Value(PI));
        // ---------------------------------------------------------------------
        RegisterFunction("math:randomf", [](std::vector<Value>& args, Value& ret) -> bool {
            ret = Value(static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:random", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 2) {
                Tools::errorf("usage: math:random min max\n");
                return false;
            }
            int min = static_cast<int>(args[0].getDouble());
            int max = static_cast<int>(args[1].getDouble());
            if (min > max) std::swap(min, max);

            int range = max - min + 1;
            ret = Value(static_cast<double>(min + (std::rand() % range)));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:sin", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::sin(args[0].getDouble()));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:cos", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::cos(args[0].getDouble()));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:tan", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::tan(args[0].getDouble()));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:rad", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;

            ret = Value(args[0].getDouble() * (PI / 180.0));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:deg", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;

            ret = Value(args[0].getDouble() * (180.0 / PI));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:clamp", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 3) {
                Tools::errorf("usage: math:clamp val min max\n");
                return false;
            }
            double val = args[0].getDouble();
            double min = args[1].getDouble();
            double max = args[2].getDouble();

            ret = Value(std::clamp(val, min, max));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:abs", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::abs(args[0].getDouble()));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:floor", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::floor(args[0].getDouble()));
            return true;
        });

        // ---------------------------------------------------------------------
        RegisterFunction("math:ceil", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::ceil(args[0].getDouble()));
            return true;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("math:sqrt", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            double val = args[0].getDouble();
            if (val < 0) {
                Tools::errorf("math:sqrt: Cannot calculate square root of a negative number.\n");
                return false;
            }
            ret = Value(std::sqrt(val));
            return true;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("math:lerp", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 3) return false;
            // a + t * (b - a);
            ret = Value(std::lerp(args[0].getDouble(),args[1].getDouble(),args[2].getDouble()));
            return true;
        });
        // ---------------------------------------------------------------------

    }

} //namespace
