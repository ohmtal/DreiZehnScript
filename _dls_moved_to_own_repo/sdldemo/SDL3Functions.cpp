//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// DLS basic SDL3 bindings
// NOTE WARNING null type safety!
//-----------------------------------------------------------------------------
// #pragma once
#include <SDL3/SDL.h>
#include <cstring>

#include "core/Value.h"
#include "core/FunctionMap.h"

namespace DLS {

    struct SDL_Event_Object {
        SDL_Event event;
        SDL_Event_Object() { std::memset(&event, 0, sizeof(SDL_Event)); }
    };
    // =================================================================================
    void RegisterSDL3Functions() {
        static bool registered = false; if (registered) return; registered = true;

        using namespace FunctionMap;
        REGISTER_CONST(SDLK_0);

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
        RegisterFunction("SDL_Init", [](std::span<Value> args, Value& ret) -> bool {
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
        RegisterFunction("SDL_Quit", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("usage: SDL_Quit()\n");
                return false;
            }
            SDL_Quit();
            return true;
        });

        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC SDL_Window * SDLCALL SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags);
        RegisterFunction("SDL_CreateWindow", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 4) {
                Tools::errorf("usage: SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags)\n");
                return false;
            }

            const char* title = args[0].getString().c_str();
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

            ret = Value((void*)win);
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC void SDLCALL SDL_DestroyWindow(SDL_Window *window);
        RegisterFunction("SDL_DestroyWindow", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_DestroyWindow(SDL_Window *window)\n");
                return false;
            }
            SDL_Window* obj = static_cast<SDL_Window*>(args[0].getPointer());
            if (!obj ) {
                Tools::errorf("SDL_DestroyWindow: Invalid SDL_Window Pointer!\n", SDL_GetError());
                return false;
            }
            SDL_DestroyWindow(obj);
            ret = Value(0);
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC SDL_Renderer * SDLCALL SDL_CreateRenderer(SDL_Window *window, const char *name);
        RegisterFunction("SDL_CreateRenderer", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() < 1 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_CreateRenderer(SDL_Window *window, const char *name)\n");
                return false;
            }
            SDL_Window* winObj = static_cast<SDL_Window*>(args[0].getPointer());
            if (!winObj ) {
                Tools::errorf("SDL_CreateRenderer: Invalid Pointer!\n");
                return false;
            }
            const char* name = (args.size() > 1 && args[1].isString()) ? args[1].getString().c_str() : nullptr;

            SDL_Renderer* rend = SDL_CreateRenderer(winObj, name);
            if (!rend) {
                Tools::errorf("SDL_CreateRenderer: Failed: %s\n", SDL_GetError());
                return false;
            }
            ret = Value((void*)rend);
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC void SDLCALL SDL_DestroyRenderer(SDL_Renderer *renderer);
        RegisterFunction("SDL_DestroyRenderer", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_DestroyRenderer(SDL_Window *window)\n");
                return false;
            }
            SDL_Renderer* obj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!obj ) {
                Tools::errorf("SDL_DestroyRenderer: Invalid  Pointer!\n", SDL_GetError());
                return false;
            }
            SDL_DestroyRenderer(obj);
            ret = Value(0);
            return true;
        });


        // ---------------------------------------------------------------------
        // Continue Render:
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
        RegisterFunction("SDL_SetRenderDrawColor", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() < 4 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_SetRenderDrawColor(SDL_Renderer *renderer, r, g, b, a)\n");
                return false;
            }
            SDL_Renderer* rendObj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!rendObj ) return false;

            bool success = SDL_SetRenderDrawColor(
                rendObj,
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
        RegisterFunction("SDL_RenderClear", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) return false;
            SDL_Renderer* rendObj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!rendObj ) return false;

            ret = Value(SDL_RenderClear(rendObj));
            return true;
        });

        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderPresent(SDL_Renderer *renderer);
        RegisterFunction("SDL_RenderPresent", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) return false;
            SDL_Renderer* rendObj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!rendObj) return false;

            ret = Value(SDL_RenderPresent(rendObj));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderDebugText(SDL_Renderer *renderer, float x, float y, const char *str);
        RegisterFunction("SDL_RenderDebugText", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 4 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_RenderDebugText(SDL_Renderer *renderer, float x, float y, const char *str)\n");
                return false;
            }
            SDL_Renderer* rendObj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!rendObj ) return false;

            float x = static_cast<float>(args[1].getFloat());
            float y = static_cast<float>(args[2].getFloat());
            const char* str = args[3].getString().c_str();

            ret = Value(SDL_RenderDebugText(rendObj, x, y, str));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_SetRenderScale(SDL_Renderer *renderer, float scaleX, float scaleY);
        RegisterFunction("SDL_SetRenderScale", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 3 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_SetRenderScale(SDL_Renderer *renderer, float scaleX, float scaleY)\n");
                return false;
            }
            SDL_Renderer* rendObj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!rendObj ) return false;

            float scaleX = static_cast<float>(args[1].getFloat());
            float scaleY = static_cast<float>(args[2].getFloat());

            ret = Value(SDL_SetRenderScale(rendObj, scaleX, scaleY));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderFillRect(SDL_Renderer *renderer, const SDL_FRect *rect);
        RegisterFunction("SDL_RenderFillRect", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 5 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_RenderFillRect(SDL_Renderer *renderer, float x, float y, float w, float h)\n");
                return false;
            }
            SDL_Renderer* rendObj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!rendObj ) return false;

            SDL_FRect rect;
            rect.x = static_cast<float>(args[1].getFloat());
            rect.y = static_cast<float>(args[2].getFloat());
            rect.w = static_cast<float>(args[3].getFloat());
            rect.h = static_cast<float>(args[4].getFloat());

            ret = Value(SDL_RenderFillRect(rendObj, &rect));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderRect(SDL_Renderer *renderer, const SDL_FRect *rect);
        RegisterFunction("SDL_RenderRect", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 5 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_RenderRect(SDL_Renderer *renderer, float x, float y, float w, float h)\n");
                return false;
            }
            SDL_Renderer* rendObj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!rendObj ) return false;

            SDL_FRect rect;
            rect.x = static_cast<float>(args[1].getFloat());
            rect.y = static_cast<float>(args[2].getFloat());
            rect.w = static_cast<float>(args[3].getFloat());
            rect.h = static_cast<float>(args[4].getFloat());

            ret = Value(SDL_RenderRect(rendObj, &rect));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC bool SDLCALL SDL_RenderLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2);
        RegisterFunction("SDL_RenderLine", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 5 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_RenderLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2)\n");
                return false;
            }
            SDL_Renderer* rendObj = static_cast<SDL_Renderer*>(args[0].getPointer());
            if (!rendObj ) return false;

            float x1 = static_cast<float>(args[1].getFloat());
            float y1 = static_cast<float>(args[2].getFloat());
            float x2 = static_cast<float>(args[3].getFloat());
            float y2 = static_cast<float>(args[4].getFloat());

            ret = Value(SDL_RenderLine(rendObj, x1, y1, x2, y2));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC Uint64 SDLCALL SDL_GetTicks(void);
        RegisterFunction("SDL_GetTicks", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 0) {
                Tools::errorf("usage: SDL_GetTicks()\n");
                return false;
            }
            ret = Value(static_cast<int>(SDL_GetTicks()));
            return true;
        });
        // ---------------------------------------------------------------------
        // extern SDL_DECLSPEC void SDLCALL SDL_Delay(Uint32 ms);
        RegisterFunction("SDL_Delay", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("usage: SDL_Delay(int ms)\n");
                return false;
            }
            Uint32 ms = static_cast<Uint32>(args[0].getInt());
            SDL_Delay(ms);
            return true;
        });
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        // NOTE: Tool function to create an empty event object
        // used by SDL_PollEvent
        RegisterFunction("SDL_CreateEvent", [](std::span<Value> args, Value& ret) -> bool {
            SDL_Event_Object* obj = new SDL_Event_Object();
            ret = Value(obj);

            return true;
        });
        RegisterFunction("SDL_DestroyEvent", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_DestroyEvent(Event *event)\n");
                return false;
            }
            SDL_Event_Object* obj = static_cast<SDL_Event_Object*>(args[0].getPointer());
            if (!obj ) {
                Tools::errorf("SDL_DestroyEvent: Invalid Event Pointer!\n", SDL_GetError());
                return false;
            }
            delete obj;
            ret = Value(0);
            return true;
        });
        // extern SDL_DECLSPEC bool SDLCALL SDL_PollEvent(SDL_Event *event);
        RegisterFunction("SDL_PollEvent", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) {
                Tools::errorf("usage: SDL_PollEvent(SDL_Event event)\n");
                return false;
            }
            SDL_Event_Object* evObj = static_cast<SDL_Event_Object*>(args[0].getPointer());
            if (!evObj) {
                Tools::errorf("SDL_PollEvent: Invalid SDL_Event Object!\n");
                return false;
            }
            ret = Value(SDL_PollEvent(&evObj->event));
            return true;
        });

        // NOTE Toolfunction for Event Type
        RegisterFunction("SDL_GetEventType", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) return false;
            SDL_Event_Object* evObj = static_cast<SDL_Event_Object*>(args[0].getPointer());
            if (!evObj) return false;

            ret = Value(static_cast<int>(evObj->event.type));
            return true;
        });

        // NOTE SDLK_ESCAPE = 27 SDLK_SPACE = 32 SDL_EVENT_QUIT = 0x100 (Dezimal: 256)
        // Tool Function for IsKeyDownEvent
        RegisterFunction("SDL_IsKeyDownEvent", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) return false;
            SDL_Event_Object* evObj = static_cast<SDL_Event_Object*>(args[0].getPointer());
            if (!evObj) return false;

            ret = Value(evObj->event.type == SDL_EVENT_KEY_DOWN);
            return true;
        });

        // Tools Function for Key Event
        RegisterFunction("SDL_GetEventKey", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointer()) return false;
            SDL_Event_Object* evObj = static_cast<SDL_Event_Object*>(args[0].getPointer());
            if (!evObj) return false;

            if (evObj->event.type == SDL_EVENT_KEY_DOWN || evObj->event.type == SDL_EVENT_KEY_UP) {
                // In SDL3 ist es evObj->value.key.scancode oder .key.key (Keycode)
                ret = Value(static_cast<int>(evObj->event.key.key));
            } else {
                ret = Value(0);
            }
            return true;
        });


    } //RegisterCoreFunctions

} //namespace
