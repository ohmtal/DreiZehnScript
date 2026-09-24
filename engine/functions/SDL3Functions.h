//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Core Commands
//-----------------------------------------------------------------------------
#pragma once
#include <SDL3/SDL.h>
#include <cstring>

#include "core/FunctionMap.h"
#include "core/ValueObject.h"
#include "core/VariableFrame.h"

namespace DreiZehn {

    const int Type_SDL_Window_Object =  registerUserObjectType("SDL_Window");

    struct SDL_Window_Object : ValueObject {
        SDL_Window* value = nullptr;
        SDL_Window_Object(SDL_Window* win) : ValueObject(Type_SDL_Window_Object), value(win) {}
        ~SDL_Window_Object() {
            if (value) SDL_DestroyWindow(value);
            value = nullptr;
        }
    };

    const int Type_SDL_Renderer_Object =  registerUserObjectType("SDL_Renderer");
    struct SDL_Renderer_Object : ValueObject {
        SDL_Renderer* value = nullptr;
        SDL_Renderer_Object(SDL_Renderer* rend) : ValueObject(Type_SDL_Renderer_Object), value(rend) {}
        ~SDL_Renderer_Object() {
            if (value) SDL_DestroyRenderer(value);
            value = nullptr;
        }
    };

    const int Type_SDL_Event_Object =  registerUserObjectType("SDL_Event_Object");
    struct SDL_Event_Object : ValueObject {
        SDL_Event value;
        SDL_Event_Object() : ValueObject(Type_SDL_Event_Object) { std::memset(&value, 0, sizeof(SDL_Event)); }
    };


    void RegisterSDL3Functions( ) {

        using namespace FunctionMap;



        // ---------------------------------------------------------------------
            RegisterConstants("SDL_INIT_AUDIO",    Value(static_cast<int>(SDL_INIT_AUDIO)));    // 0x00000010
            RegisterConstants("SDL_INIT_VIDEO",    Value(static_cast<int>(SDL_INIT_VIDEO)));    // 0x00000020
            RegisterConstants("SDL_INIT_JOYSTICK", Value(static_cast<int>(SDL_INIT_JOYSTICK))); // 0x00000200
            RegisterConstants("SDL_INIT_GAMEPAD",  Value(static_cast<int>(SDL_INIT_GAMEPAD)));  // 0x00002000
            RegisterConstants("SDL_INIT_EVENTS",   Value(static_cast<int>(SDL_INIT_EVENTS)));   // 0x00004000

            RegisterConstants("SDL_WINDOW_FULLSCREEN", Value(static_cast<int>(SDL_WINDOW_FULLSCREEN))); // 0x00000001
            RegisterConstants("SDL_WINDOW_OPENGL",     Value(static_cast<int>(SDL_WINDOW_OPENGL)));     // 0x00000002
            RegisterConstants("SDL_WINDOW_HIDDEN",     Value(static_cast<int>(SDL_WINDOW_HIDDEN)));     // 0x00000008
            RegisterConstants("SDL_WINDOW_BORDERLESS", Value(static_cast<int>(SDL_WINDOW_BORDERLESS))); // 0x00000010
            RegisterConstants("SDL_WINDOW_RESIZABLE",  Value(static_cast<int>(SDL_WINDOW_RESIZABLE)));  // 0x00000020
            RegisterConstants("SDL_WINDOW_HIGH_PIXEL_DENSITY", Value(static_cast<int>(SDL_WINDOW_HIGH_PIXEL_DENSITY))); // Retina/4K

            RegisterConstants("SDL_EVENT_QUIT",           Value(static_cast<int>(SDL_EVENT_QUIT)));           // 0x100 (256)
            RegisterConstants("SDL_EVENT_KEY_DOWN",       Value(static_cast<int>(SDL_EVENT_KEY_DOWN)));       // 0x300
            RegisterConstants("SDL_EVENT_KEY_UP",         Value(static_cast<int>(SDL_EVENT_KEY_UP)));         // 0x301
            RegisterConstants("SDL_EVENT_MOUSE_MOTION",   Value(static_cast<int>(SDL_EVENT_MOUSE_MOTION)));   // 0x400
            RegisterConstants("SDL_EVENT_MOUSE_BUTTON_DOWN", Value(static_cast<int>(SDL_EVENT_MOUSE_BUTTON_DOWN))); // 0x401
            RegisterConstants("SDL_EVENT_MOUSE_BUTTON_UP",   Value(static_cast<int>(SDL_EVENT_MOUSE_BUTTON_UP)));   // 0x402

            RegisterConstants("SDLK_ESCAPE", Value(static_cast<int>(SDLK_ESCAPE))); // 27
            RegisterConstants("SDLK_SPACE",  Value(static_cast<int>(SDLK_SPACE)));  // 32
            RegisterConstants("SDLK_RETURN", Value(static_cast<int>(SDLK_RETURN))); // 13
            RegisterConstants("SDLK_UP",     Value(static_cast<int>(SDLK_UP)));     // Cursor Up
            RegisterConstants("SDLK_DOWN",   Value(static_cast<int>(SDLK_DOWN)));   // Cursor Down
            RegisterConstants("SDLK_LEFT",   Value(static_cast<int>(SDLK_LEFT)));   // Cursor Left
            RegisterConstants("SDLK_RIGHT",  Value(static_cast<int>(SDLK_RIGHT)));  // Cursor Right

            RegisterConstants("SDLK_W", Value(static_cast<int>(SDLK_W)));
            RegisterConstants("SDLK_A", Value(static_cast<int>(SDLK_A)));
            RegisterConstants("SDLK_S", Value(static_cast<int>(SDLK_S)));
            RegisterConstants("SDLK_D", Value(static_cast<int>(SDLK_D)));

        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        // SDL_INIT_AUDIO = 0x00000010 (Dezimal: 16)
        // SDL_INIT_VIDEO = 0x00000020 (Dezimal: 32)
        // SDL_INIT_EVENTS = 0x00004000 (Dezimal: 16384)
        // extern SDL_DECLSPEC bool SDLCALL SDL_Init(SDL_InitFlags flags);
        RegisterFunction("SDL_Init", [](std::vector<Value>& args, Value& ret) -> bool {
            // if (args.size() != 1) {
            //     Tools::errorf("usage: SDL_Init(int flags)\n");
            //     return false;
            // }

            Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO;
            if (args.size() > 0) flags = static_cast<Uint32>(args[0].getInt());

            bool success = SDL_Init(flags);
            if (!success) {
                Tools::errorf("SDL_Init Failed: %s\n", SDL_GetError());
            }

            ret = Value(success);
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC void SDLCALL SDL_Quit(void);
        RegisterFunction("SDL_Quit", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("usage: SDL_Quit()\n");
                return false;
            }
            SDL_Quit();
            return true;
        });

        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC SDL_Window * SDLCALL SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags);
        RegisterFunction("SDL_CreateWindow", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 4) {
                Tools::errorf("usage: SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags)\n");
                return false;
            }

            const char* title = (args[0].getStringObj()) ? args[0].getStringObj()->mValue.c_str() : "";
            SDL_Window* win = SDL_CreateWindow(
                title,
                args[1].getInt(),
                args[2].getInt(),
                args[3].getInt()
            );
            if (!win) {
                Tools::errorf("SDL_CreateWindow: Failed to create Window: %s\n", SDL_GetError());
                return false;
            }
            SDL_Window_Object* obj = new SDL_Window_Object(win);
            ret = Value(obj);
            if (gCurrentFrame) gCurrentFrame->addToGarbageCollection(obj);
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC void SDLCALL SDL_DestroyWindow(SDL_Window *window);
        RegisterFunction("SDL_DestroyWindow", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_DestroyWindow(SDL_Window *window)\n");
                return false;
            }
            SDL_Window_Object* obj = dynamic_cast<SDL_Window_Object*>(args[0].asPointerObject());
            if (!obj || !obj->value) {
                Tools::errorf("SDL_DestroyWindow: Invalid SDL_Window Pointer!\n", SDL_GetError());
                return false;
            }
            SDL_DestroyWindow(obj->value);
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC SDL_Renderer * SDLCALL SDL_CreateRenderer(SDL_Window *window, const char *name);
        RegisterFunction("SDL_CreateRenderer", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() < 1) {
                Tools::errorf("usage: SDL_CreateRenderer(SDL_Window *window, const char *name)\n");
                return false;
            }
            SDL_Window_Object* winObj = dynamic_cast<SDL_Window_Object*>(args[0].asPointerObject());
            if (!winObj || !winObj->value) {
                Tools::errorf("SDL_CreateRenderer: Invalid SDL_Window!\n");
                return false;
            }
            const char* name = (args.size() > 1 && args[1].getStringObj()) ? args[1].getStringObj()->mValue.c_str() : nullptr;

            SDL_Renderer* rend = SDL_CreateRenderer(winObj->value, name);
            if (!rend) {
                Tools::errorf("SDL_CreateRenderer: Failed: %s\n", SDL_GetError());
                return false;
            }
            SDL_Renderer_Object* obj = new SDL_Renderer_Object(rend);
            ret = Value(obj);
            if (gCurrentFrame) gCurrentFrame->addToGarbageCollection(obj);
            return true;
        });
        // ---------------------------------------------------------------------
        // Events
        // ---------------------------------------------------------------------
        // NOTE: Tool function to create an empty event object
        // used by SDL_PollEvent
        RegisterFunction("SDL_CreateEvent", [](std::vector<Value>& args, Value& ret) -> bool {
            SDL_Event_Object* obj = new SDL_Event_Object();
            ret = Value(obj);

            if (gCurrentFrame) gCurrentFrame->addToGarbageCollection(obj);
            return true;
        });
        // extern SDL_DECLSPEC bool SDLCALL SDL_PollEvent(SDL_Event *event);
        RegisterFunction("SDL_PollEvent", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("usage: SDL_PollEvent(SDL_Event event)\n");
                return false;
            }
            SDL_Event_Object* evObj = dynamic_cast<SDL_Event_Object*>(args[0].asPointerObject());
            if (!evObj) {
                Tools::errorf("SDL_PollEvent: Invalid SDL_Event Object!\n");
                return false;
            }
            ret = Value(SDL_PollEvent(&evObj->value));
            return true;
        });

        // NOTE Toolfunction for Event Type
        RegisterFunction("SDL_GetEventType", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            SDL_Event_Object* evObj = dynamic_cast<SDL_Event_Object*>(args[0].asPointerObject());
            if (!evObj) return false;

            ret = Value(static_cast<int>(evObj->value.type));
            return true;
        });

        // NOTE SDLK_ESCAPE = 27 SDLK_SPACE = 32 SDL_EVENT_QUIT = 0x100 (Dezimal: 256)
        // Tool Function for IsKeyDownEvent
        RegisterFunction("SDL_IsKeyDownEvent", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            SDL_Event_Object* evObj = dynamic_cast<SDL_Event_Object*>(args[0].asPointerObject());
            if (!evObj) return false;

            ret = Value(evObj->value.type == SDL_EVENT_KEY_DOWN);
            return true;
        });

        // Tools Function for Key Event
        RegisterFunction("SDL_GetEventKey", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            SDL_Event_Object* evObj = dynamic_cast<SDL_Event_Object*>(args[0].asPointerObject());
            if (!evObj) return false;

            if (evObj->value.type == SDL_EVENT_KEY_DOWN || evObj->value.type == SDL_EVENT_KEY_UP) {
                // In SDL3 ist es evObj->value.key.scancode oder .key.key (Keycode)
                ret = Value(static_cast<int>(evObj->value.key.key));
            } else {
                ret = Value(0.0);
            }
            return true;
        });

        // ---------------------------------------------------------------------
        // Continue Render:
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
        RegisterFunction("SDL_SetRenderDrawColor", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() < 4) {
                Tools::errorf("usage: SDL_SetRenderDrawColor(SDL_Renderer *renderer, r, g, b, a)\n");
                return false;
            }
            SDL_Renderer_Object* rendObj = dynamic_cast<SDL_Renderer_Object*>(args[0].asPointerObject());
            if (!rendObj || !rendObj->value) return false;

            bool success = SDL_SetRenderDrawColor(
                rendObj->value,
                static_cast<Uint8>(args[1].getInt()),
                        static_cast<Uint8>(args[2].getInt()),
                        static_cast<Uint8>(args[3].getInt()),
                        (args.size() >= 4) ?  static_cast<Uint8>(args[4].getInt()) : 255
            );
            ret = Value(success);
            return true;
        });

        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderClear(SDL_Renderer *renderer);
        RegisterFunction("SDL_RenderClear", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            SDL_Renderer_Object* rendObj = dynamic_cast<SDL_Renderer_Object*>(args[0].asPointerObject());
            if (!rendObj || !rendObj->value) return false;

            ret = Value(SDL_RenderClear(rendObj->value));
            return true;
        });

        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderPresent(SDL_Renderer *renderer);
        RegisterFunction("SDL_RenderPresent", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) return false;
            SDL_Renderer_Object* rendObj = dynamic_cast<SDL_Renderer_Object*>(args[0].asPointerObject());
            if (!rendObj || !rendObj->value) return false;

            ret = Value(SDL_RenderPresent(rendObj->value));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderDebugText(SDL_Renderer *renderer, float x, float y, const char *str);
        RegisterFunction("SDL_RenderDebugText", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 4) {
                Tools::errorf("usage: SDL_RenderDebugText(SDL_Renderer *renderer, float x, float y, const char *str)\n");
                return false;
            }
            SDL_Renderer_Object* rendObj = dynamic_cast<SDL_Renderer_Object*>(args[0].asPointerObject());
            if (!rendObj || !rendObj->value) return false;

            float x = static_cast<float>(args[1].getFloat());
            float y = static_cast<float>(args[2].getFloat());
            const char* str = (args[3].getStringObj()) ? args[3].getStringObj()->mValue.c_str() : "";

            ret = Value(SDL_RenderDebugText(rendObj->value, x, y, str));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_SetRenderScale(SDL_Renderer *renderer, float scaleX, float scaleY);
        RegisterFunction("SDL_SetRenderScale", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 3) {
                Tools::errorf("usage: SDL_SetRenderScale(SDL_Renderer *renderer, float scaleX, float scaleY)\n");
                return false;
            }
            SDL_Renderer_Object* rendObj = dynamic_cast<SDL_Renderer_Object*>(args[0].asPointerObject());
            if (!rendObj || !rendObj->value) return false;

            float scaleX = static_cast<float>(args[1].getFloat());
            float scaleY = static_cast<float>(args[2].getFloat());

            ret = Value(SDL_SetRenderScale(rendObj->value, scaleX, scaleY));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderFillRect(SDL_Renderer *renderer, const SDL_FRect *rect);
        RegisterFunction("SDL_RenderFillRect", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 5) {
                Tools::errorf("usage: SDL_RenderFillRect(SDL_Renderer *renderer, float x, float y, float w, float h)\n");
                return false;
            }
            SDL_Renderer_Object* rendObj = dynamic_cast<SDL_Renderer_Object*>(args[0].asPointerObject());
            if (!rendObj || !rendObj->value) return false;

            SDL_FRect rect;
            rect.x = static_cast<float>(args[1].getFloat());
            rect.y = static_cast<float>(args[2].getFloat());
            rect.w = static_cast<float>(args[3].getFloat());
            rect.h = static_cast<float>(args[4].getFloat());

            ret = Value(SDL_RenderFillRect(rendObj->value, &rect));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderRect(SDL_Renderer *renderer, const SDL_FRect *rect);
        RegisterFunction("SDL_RenderRect", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 5) {
                Tools::errorf("usage: SDL_RenderRect(SDL_Renderer *renderer, float x, float y, float w, float h)\n");
                return false;
            }
            SDL_Renderer_Object* rendObj = dynamic_cast<SDL_Renderer_Object*>(args[0].asPointerObject());
            if (!rendObj || !rendObj->value) return false;

            SDL_FRect rect;
            rect.x = static_cast<float>(args[1].getFloat());
            rect.y = static_cast<float>(args[2].getFloat());
            rect.w = static_cast<float>(args[3].getFloat());
            rect.h = static_cast<float>(args[4].getFloat());

            ret = Value(SDL_RenderRect(rendObj->value, &rect));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2);
        RegisterFunction("SDL_RenderLine", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 5) {
                Tools::errorf("usage: SDL_RenderLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2)\n");
                return false;
            }
            SDL_Renderer_Object* rendObj = dynamic_cast<SDL_Renderer_Object*>(args[0].asPointerObject());
            if (!rendObj || !rendObj->value) return false;

            float x1 = static_cast<float>(args[1].getFloat());
            float y1 = static_cast<float>(args[2].getFloat());
            float x2 = static_cast<float>(args[3].getFloat());
            float y2 = static_cast<float>(args[4].getFloat());

            ret = Value(SDL_RenderLine(rendObj->value, x1, y1, x2, y2));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC Uint64 SDLCALL SDL_GetTicks(void);
        RegisterFunction("SDL_GetTicks", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("usage: SDL_GetTicks()\n");
                return false;
            }
            ret = Value(static_cast<int>(SDL_GetTicks()));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC void SDLCALL SDL_Delay(Uint32 ms);
        RegisterFunction("SDL_Delay", [](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("usage: SDL_Delay(int ms)\n");
                return false;
            }
            Uint32 ms = static_cast<Uint32>(args[0].getInt());
            SDL_Delay(ms);
            return true;
        });
        // ---------------------------------------------------------------------


    } //RegisterCoreFunctions

} //namespace
