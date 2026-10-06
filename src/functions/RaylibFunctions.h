//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Raylib Function - Initial - i work also on a auto generator.
//                           - autogen miss the structs inside stucts do far
//-----------------------------------------------------------------------------
#pragma once
#include <stdio.h>
#include <stdint.h>

#include <vector>

#include "core/FunctionMap.h"
#include "core/VariableFrame.h"
#include "Globals.h"
#include "ArrayFunctions.h"

namespace DreiZehn::Raylib {
    #include "raylib-6.0/src/raylib.h"

    static int TypeRaylibColor = 0;

    static ValueObjectProperty prop_Color_r;
    static ValueObjectProperty prop_Color_g;
    static ValueObjectProperty prop_Color_b;
    static ValueObjectProperty prop_Color_a;



    struct ValueObjectColor : public ValueObject {
        Value r;
        Value g;
        Value b;
        Value a;

        ValueObjectColor(const Color& val): ValueObject(TypeRaylibColor) {
            mAssigned = 0;
            r = Value((double)val.r);
            g = Value((double)val.g);
            b = Value((double)val.b);
            a = Value((double)val.a);
        }

        Color toRaylib() const {
            Color res;
            res.r = (unsigned char)r.getDouble();
            res.g = (unsigned char)g.getDouble();
            res.b = (unsigned char)b.getDouble();
            res.a = (unsigned char)a.getDouble();
            return res;
        }

        inline virtual Value* onGetFieldPtr(uint32_t fieldSymbolId) override {
            if (fieldSymbolId == prop_Color_r.mSymbolId) return &r;
            if (fieldSymbolId == prop_Color_g.mSymbolId) return &g;
            if (fieldSymbolId == prop_Color_b.mSymbolId) return &b;
            if (fieldSymbolId == prop_Color_a.mSymbolId) return &a;
            Tools::errorf("Runtime Error: Field not found on Color.\n");
            return nullptr;
        }

        inline virtual bool onGetField(uint32_t fieldSymbolId, Value& ret) override {
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (ptr) { ret = *ptr; return true; }
            return false;
        }

        inline virtual bool onSetField(uint32_t fieldSymbolId, const Value& value) override {
            Value* ptr = onGetFieldPtr(fieldSymbolId);
            if (ptr) { *ptr = value; return true; }
            return false;
        }

    };
    // -------------------------------------------------------------------------
    void RegisterRaylibFunctions(Environment& env) {

        using namespace FunctionMap;

        TypeRaylibColor = RegisterUserObjectType("Color");

        prop_Color_r = ValueObjectProperty("r","", TypeRaylibColor);
        prop_Color_g = ValueObjectProperty("g","", TypeRaylibColor);
        prop_Color_b = ValueObjectProperty("b","", TypeRaylibColor);
        prop_Color_a = ValueObjectProperty("a","", TypeRaylibColor);

        RegisterConstants("rl::RAYLIB_VERSION", Value(std::string(RAYLIB_VERSION)));
        RegisterConstants("rl::DEG2RAD", Value((double)(PI/180.0f)));

        {
            ValueObjectColor* const_color = new ValueObjectColor(LIGHTGRAY);
            const_color->mAssigned = 1;
            RegisterConstants("rl::LIGHTGRAY", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(GRAY);
            const_color->mAssigned = 1;
            RegisterConstants("rl::GRAY", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(DARKGRAY);
            const_color->mAssigned = 1;
            RegisterConstants("rl::DARKGRAY", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(YELLOW);
            const_color->mAssigned = 1;
            RegisterConstants("rl::YELLOW", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(GOLD);
            const_color->mAssigned = 1;
            RegisterConstants("rl::GOLD", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(ORANGE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::ORANGE", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(PINK);
            const_color->mAssigned = 1;
            RegisterConstants("rl::PINK", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(RED);
            const_color->mAssigned = 1;
            RegisterConstants("rl::RED", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(MAROON);
            const_color->mAssigned = 1;
            RegisterConstants("rl::MAROON", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(GREEN);
            const_color->mAssigned = 1;
            RegisterConstants("rl::GREEN", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(LIME);
            const_color->mAssigned = 1;
            RegisterConstants("rl::LIME", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(DARKGREEN);
            const_color->mAssigned = 1;
            RegisterConstants("rl::DARKGREEN", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(SKYBLUE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::SKYBLUE", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(BLUE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::BLUE", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(DARKBLUE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::DARKBLUE", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(PURPLE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::PURPLE", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(VIOLET);
            const_color->mAssigned = 1;
            RegisterConstants("rl::VIOLET", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(DARKPURPLE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::DARKPURPLE", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(BEIGE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::BEIGE", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(BROWN);
            const_color->mAssigned = 1;
            RegisterConstants("rl::BROWN", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(DARKBROWN);
            const_color->mAssigned = 1;
            RegisterConstants("rl::DARKBROWN", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(WHITE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::WHITE", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(BLACK);
            const_color->mAssigned = 1;
            RegisterConstants("rl::BLACK", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(BLANK);
            const_color->mAssigned = 1;
            RegisterConstants("rl::BLANK", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(MAGENTA);
            const_color->mAssigned = 1;
            RegisterConstants("rl::MAGENTA", Value(const_color));
        }
        {
            ValueObjectColor* const_color = new ValueObjectColor(RAYWHITE);
            const_color->mAssigned = 1;
            RegisterConstants("rl::RAYWHITE", Value(const_color));
        }



        RegisterFunction("rl::Color", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 0 && args.size() != 4) {
                Tools::errorf("Usage: rl::Color or rl::Color unsigned char r, unsigned char g, unsigned char b, unsigned char a \n");
                return false;
            }

            Color temp_struct;
            if (args.size() == 0) {
                temp_struct = {};
            } else {
                temp_struct.r = (unsigned char)args[0].getDouble();
                temp_struct.g = (unsigned char)args[1].getDouble();
                temp_struct.b = (unsigned char)args[2].getDouble();
                temp_struct.a = (unsigned char)args[3].getDouble();
            }

            ValueObjectColor* wrapper_res = new ValueObjectColor(temp_struct);

            ret = Value(wrapper_res);
            return true;
        });

        RegisterFunction("rl::InitWindow", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 3) {
                Tools::errorf("Usage:raylib::InitWindow int width int height string title\n");
                ret = Value(0);
                return false;
            }
            InitWindow(args[0].getInt(),args[1].getInt(), args[2].getStringRef().c_str());
            ret = Value(1);
            return true;
        });

        RegisterFunction("rl::SetTargetFPS", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: raylib::SetTargetFPS(int fps)\n");
                return false;
            }

            int arg_fps = (int)args[0].getDouble();
            SetTargetFPS(arg_fps);
            ret = Value(1.0);
            return true;
        });

        RegisterFunction("rl::GetFrameTime", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("Usage: raylib::GetFrameTime()\n");
                return false;
            }

            float c_res = GetFrameTime();
            ret = Value((double)c_res);
            return true;
        });

        RegisterFunction("rl::WindowShouldClose", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("Usage: raylib::WindowShouldClose()\n");
                return false;
            }

            bool c_res = WindowShouldClose();
            ret = Value((double)c_res);
            return true;
        });

        RegisterFunction("rl::ClearBackground", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: raylib::ClearBackground(Color color)\n");
                return false;
            }

            if (! args[0].isPointer() ) return false;
            ValueObject* obj_color = args[0].asPointerObject();
            if (!obj_color || obj_color->mType != TypeRaylibColor) {
                Tools::errorf("Arg 0 must be of type Color");
                return false;
            }
            Color arg_color = static_cast<ValueObjectColor*>(obj_color)->toRaylib();
            ClearBackground(arg_color);
            ret = Value(1.0);
            return true;
        });


        RegisterFunction("rl::DrawText", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 5) {
                Tools::errorf("Usage: raylib::DrawText(const char * text, int posX, int posY, int fontSize, Color color)\n");
                return false;
            }

            const char* arg_text = args[0].getStringRef().c_str();
            int arg_posX = (int)args[1].getDouble();
            int arg_posY = (int)args[2].getDouble();
            int arg_fontSize = (int)args[3].getDouble();
            if (! args[4].isPointer() ) return false;
            ValueObject* obj_color = args[4].asPointerObject();
            if (!obj_color || obj_color->mType != TypeRaylibColor) {
                Tools::errorf("Arg 4 must be of type Color");
                return false;
            }
            Color arg_color = static_cast<ValueObjectColor*>(obj_color)->toRaylib();
            DrawText(arg_text, arg_posX, arg_posY, arg_fontSize, arg_color);
            ret = Value(1.0);
            return true;
        });

        RegisterFunction("rl::BeginDrawing", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("Usage: raylib::BeginDrawing()\n");
                return false;
            }

            BeginDrawing();
            ret = Value(1.0);
            return true;
        });

        RegisterFunction("rl::EndDrawing", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("Usage: raylib::EndDrawing()\n");
                return false;
            }

            EndDrawing();
            ret = Value(1.0);
            return true;
        });

        RegisterFunction("rl::CloseWindow", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("Usage: raylib::CloseWindow()\n");
                return false;
            }

            CloseWindow();
            ret = Value(1.0);
            return true;
        });

        RegisterFunction("rl::MeasureText", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 2) {
                Tools::errorf("Usage: raylib::MeasureText(const char * text, int fontSize)\n");
                return false;
            }

            const char* arg_text = args[0].getStringRef().c_str();
            int arg_fontSize = (int)args[1].getDouble();
            int c_res = MeasureText(arg_text, arg_fontSize);
            ret = Value((double)c_res);
            return true;
        });

        RegisterFunction("rl::DrawFPS", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 2) {
                Tools::errorf("Usage: raylib::DrawFPS(int posX, int posY)\n");
                return false;
            }

            int arg_posX = (int)args[0].getDouble();
            int arg_posY = (int)args[1].getDouble();
            DrawFPS(arg_posX, arg_posY);
            ret = Value(1.0);
            return true;
        });

    }
} //namespace DreiZehn
