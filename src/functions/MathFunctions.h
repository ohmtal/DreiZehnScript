//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Math
//-----------------------------------------------------------------------------
#pragma once
#include <ctime>
#include <cmath>
#include "core/FunctionMap.h"
#include "ScriptLoader.h"

namespace DreiZehn {
    void RegisterMathFunctions(Environment& env) {
         static bool registered = false; if (registered) return; registered = true;

        std::srand(std::time(nullptr)); // setup random seed

        using namespace FunctionMap;

        // ---------------------------------------------------------------------
        constexpr double PI = 3.14159265358979323846;
        RegisterConstants("math::pi", Value(PI));
        // ---------------------------------------------------------------------
        RegisterFunction("math::randomf", [](std::vector<Value>& args, Value& ret) -> bool {
            ret = Value(static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX));
            return true;
        }, "return a random float value.");

        // ---------------------------------------------------------------------
        RegisterFunction("math::random", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 2) {
                Tools::errorf("usage: math::random min max\n");
                return false;
            }
            int min = args[0].getInt();
            int max = args[1].getInt();
            if (min > max) std::swap(min, max);

            int range = max - min + 1;
            ret = Value((min + (std::rand() % range)));
            return true;
        }, "integer random number between value1 and value2");
        // ---------------------------------------------------------------------
        RegisterFunction("math::rand", [](std::vector<Value>& args, Value& ret) -> bool {
            ret = Value(std::rand());
            return true;
        }, "integer random number without a parameter like c std::rand");
        // ---------------------------------------------------------------------
        RegisterFunction("math::sin", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::sin(args[0].getDouble()));
            return true;
        }, "return sine of value using std::sin");

        // ---------------------------------------------------------------------
        RegisterFunction("math::cos", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::cos(args[0].getDouble()));
            return true;
        }, "return cosine of value using std::cos");

        // ---------------------------------------------------------------------
        RegisterFunction("math::tan", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::tan(args[0].getDouble()));
            return true;
        }, "return arc tangent of the radian value using std::tan");

        // ---------------------------------------------------------------------
        RegisterFunction("math::rad", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;

            ret = Value(args[0].getDouble() * (PI / 180.0));
            return true;
        }, "return the radian of an angle");

        // ---------------------------------------------------------------------
        RegisterFunction("math::deg", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;

            ret = Value(args[0].getDouble() * (180.0 / PI));
            return true;
        }, "return the angle of a radian value");

        // ---------------------------------------------------------------------
        RegisterFunction("math::clamp", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 3) {
                Tools::errorf("usage: math::clamp val min max\n");
                return false;
            }
            double val = args[0].getDouble();
            double min = args[1].getDouble();
            double max = args[2].getDouble();

            ret = Value(std::clamp(val, min, max));
            return true;
        },"Return the given value if in range of min and max. If it's lower than min, return min. Otherwise return max.");

        // ---------------------------------------------------------------------
        RegisterFunction("math::abs", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::abs(args[0].getDouble()));
            return true;
        }, "Return the absolute number of a value.");

        // ---------------------------------------------------------------------
        RegisterFunction("math::floor", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::floor(args[0].getDouble()));
            return true;
        }, "Return the next lowest full number.");

        // ---------------------------------------------------------------------
        RegisterFunction("math::ceil", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            ret = Value(std::ceil(args[0].getDouble()));
            return true;
        }, "Return the next highest full number.");
        // ---------------------------------------------------------------------
        RegisterFunction("math::sqrt", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            double val = args[0].getDouble();
            if (val < 0) {
                Tools::errorf("math::sqrt: Cannot calculate square root of a negative number.\n");
                return false;
            }
            ret = Value(std::sqrt(val));
            return true;
        }, "Returns the square root of of value");
        // ---------------------------------------------------------------------
        RegisterFunction("math::lerp", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 3) return false;
            // a + t * (b - a);
            ret = Value(std::lerp(args[0].getDouble(),args[1].getDouble(),args[2].getDouble()));
            return true;
        }, "Return linear interpolation between two values based on a given fraction");
        // ---------------------------------------------------------------------
        RegisterFunction("math::pow", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 2) return false;
            double i1 = args[0].getDouble();
            double i2 = args[1].getDouble();
            double res = std::pow(i1, i2);
            if (std::isnan(res)) {
                Tools::errorf("Error: pow(%f, %f) produced NaN.\n", i1,i2);
                return false;
            }
            ret = Value(res);
            return true;
        }, "Return the value of a base raised to a specified exponent");
        // ---------------------------------------------------------------------
        RegisterFunction("math::fmod", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 2) return false;

            ret = Value(std::fmod(args[0].getDouble(),args[1].getDouble()));
            return true;
        }, "Return the float modulo of two values.");
        // ---------------------------------------------------------------------
        RegisterFunction("math::min", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 2) return false;

            ret = Value(std::min(args[0].getDouble(),args[1].getDouble()));
            return true;
        }, "Return the smaller of two values or the first element in case of equality.");
        // ---------------------------------------------------------------------
        RegisterFunction("math::max", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 2) return false;

            ret = Value(std::max(args[0].getDouble(),args[1].getDouble()));
            return true;
        }, "Return the higher of two values or the first element in case of equality.");
        // ---------------------------------------------------------------------
    }

} //namespace
