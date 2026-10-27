//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// BaseFlux13 Main source
// HACK using Eval for events! this enable both: Interpreter and ByteCode
//-----------------------------------------------------------------------------
#include <cstdio>
#include <string>

// ---- DreiZehn
#include "DreiZehn.h"
#include "ScriptLoader.h"
#include "toolbox/fluxStr.h"
#include "core/BaseObjects.h"

// ----  BaseFlux
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui.h"

#include "BaseFlux/Main.h"
#include "spice/gui/ImConsole.h"



BaseFlux::Main app;
ImConsole gConsole;
SDL_Point gMousePos = {0,0};

bool gDoUpdateCall; // = Con::isFunction("OnUpdate");
bool gDoRenderCall; // = Con::isFunction("OnRender");

bool gShowConsole = false;

std::string gDirectory = "assets/";
std::string gScriptFile = "assets/main.13";


using namespace DreiZehn;
extern void RegisterBaseFluxFunctions(Environment& env);

//-----------------------------------------------------------------------------
// console redirect ....
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    U32 count = FluxStr::getWordCount(message, '\n');

    if (count < 2) {
        gConsole.AddLog("%s", message);
    } else {
        for (U32 i = 0; i < count; i++) {
            gConsole.AddLog("%s", FluxStr::getWord(message, i, '\n').c_str());
        }
    }
    printf("%s\n", message);


}
//------------------------------------------------------------------------------
// typedef void(*LogCallback)(LogLevel level, const char* message);
void MyLogger(Tools::LogLevel level, const char *consoleLine)
{
    switch (level) {
        case Tools::LogLevel::warning: SDL_Log("[warn] %s",  consoleLine); break;
        case Tools::LogLevel::error: SDL_Log("[error] %s",  consoleLine); break;
        default: SDL_Log("%s",  consoleLine); break;
    }

}
// ----------------------------------------------------------------------------
void initConsole(Environment& env) {
    SDL_SetLogOutputFunction(ConsoleLogFunction, nullptr);

    //FIXME
    // gConsole.OnTabCompletion = [&](ImConsole* console, ImGuiInputTextCallbackData* data, bool forward) {
    //     char buffer[256] = {0};
    //     snprintf(buffer, sizeof(buffer), "%s", data->Buf);
    //     U32 newCursorPos = Con::tabComplete(buffer, data->CursorPos, sizeof(buffer) - 1, forward);
    //     // Log("TESTTAB: %s (%d)", buffer, newCursorPos);
    //
    //     data->DeleteChars(0, data->BufTextLen);
    //     data->InsertChars(0, buffer);
    //     data->CursorPos = newCursorPos;
    //     data->SelectionStart = newCursorPos;
    //     data->SelectionEnd = newCursorPos;
    //     data->BufDirty = true;
    // };
    //
    gConsole.OnCommand = [&env](ImConsole* console, const char* command_line) {
        Core::Eval(command_line, env);
    };

}
// ----------------------------------------------------------------------------
int argParser(int argc, char* argv[]) {

    int minargs = 1;
    for (int i = 1; i < argc; i++) {
        std::string strArg = argv[i];
        if (strArg == "-ip") {
            Globals::ByteCodeMode = false;
            minargs++;
            break;
        } else if (strArg == "-bc") {
            Globals::ByteCodeMode = true;
            minargs++;
            break;
        }
    }


    if (argc > minargs) {
        gScriptFile = argv[minargs];
    }

    return 0;
}
// ----------------------------------------------------------------------------
//  MAIN
// ----------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    DreiZehn::Environment env;
    DreiZehn::Init(env, argc, argv);
    FunctionMap::ModuleRegistry::LoadAll(env);
    Tools::LogHandler = MyLogger; //FIXME add to init
    RegisterBaseFluxFunctions(env);
    DreiZehn::FunctionMap::RegisterScriptFunctionStub("flux::Load");
    DreiZehn::FunctionMap::RegisterScriptFunctionStub("flux::Init");
    DreiZehn::FunctionMap::RegisterScriptFunctionStub("flux::Render");
    DreiZehn::FunctionMap::RegisterScriptFunctionStub("flux::ShutDown");
    DreiZehn::FunctionMap::RegisterScriptFunctionStub("flux::OnKey"
        ,{"keycode","isDown", "mod"});
    DreiZehn::FunctionMap::RegisterScriptFunctionStub("flux::OnMouse"
        ,{"x","y","button","isDown"});
    argParser(argc, argv);

    initConsole(env);

    // inject Flux Object
    BaseValueObject* FluxObject = new BaseValueObject();
    Globals::MasterVariableFrame->setVariable(SymbolTable::insert("Flux"), Value(FluxObject), true);
    // FluxObject->mClassName ="Flux"; // mhh does not do anything!

    Tools::printf("Console ready. Mode: %s\n", Globals::ByteCodeMode ? "ByteCode" : "Interpreter");

    if (gDirectory.empty()) gDirectory = BaseFlux::Tools::getBasePath().c_str();
    if (!gDirectory.ends_with("/")) gDirectory += "/";
    app.mBasePath = gDirectory.c_str();


    bool success = RunScriptFile(gScriptFile, env, Globals::ByteCodeMode);
    if (!success) app.TerminateApplication();

    app.getSettings() = {
        .ScreenSize = { 800, 450},
        .Company = "Ohmtal",
        .Caption = "BaseFlux13",
        .Version = "Version 0.1",

        // .sdlWindowFlagsOverwrite = SDL_WINDOW_BORDERLESS
    };
    // -------------------------------------------------------------------------
    Core::Eval("flux::Load", env);
    // -------------------------------------------------------------------------

    if ( !app.InitSDL() ) return 1;

    // postInitBindings_SDL();
    app.initImGui();


    // -------------------------------------------------------------------------
    Core::Eval("flux::Init", env);
    // -------------------------------------------------------------------------
    app.OnLoopBegin = []() {
        // FIXME
        // ElfSDL3::ClearInputFrameTicks();
    };
    app.OnRender = [&](SDL_Renderer* renderer) {

        Core::Eval("flux::Render", env);
        if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) gShowConsole = !gShowConsole;
        gConsole.Draw("Console",&gShowConsole);
    };

    // -------------------------------------------------------------------------
    app.OnUpdate = [&](const float dt) {
        //FIXME
        // if (gDoUpdateCall)Con::executef("OnUpdate", dt);
        // engineGlue::process((SimTime)(dt * 1000.f));
    };
    // -------------------------------------------------------------------------
    app.OnEvent = [&](const SDL_Event& event) {
        if (app.getImGuiIO())  {
            if (app.getImGuiIO()->WantTextInput) {
            if (event.type == SDL_EVENT_KEY_DOWN ||
                event.type == SDL_EVENT_KEY_UP ||
                event.type == SDL_EVENT_TEXT_INPUT) {
                    return;
                }
            }
            if (app.getImGuiIO()-> WantCaptureMouse) {
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                    event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                    event.type == SDL_EVENT_MOUSE_WHEEL
                ) {
                    return;
                }

            }
        }

        // FIXME
        // ElfSDL3::onEvent(event);
        switch (event.type) {
            case SDL_EVENT_MOUSE_MOTION:
                gMousePos.x = event.motion.x;
                gMousePos.y = event.motion.y;
                break;

            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_KEY_DOWN:
            {
                // HACK THIS IS SHIT!
                std::string cmd = "flux::OnKey";
                cmd = cmd + " " + std::to_string(event.key.key);
                cmd = cmd + " " + ((event.type == SDL_EVENT_KEY_DOWN ) ? "1" : "0");
                cmd = cmd + " " + std::to_string(event.key.mod);
                Core::Eval(cmd, env);
            }
            break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                // HACK THIS IS SHIT!
                std::string cmd = "flux::OnMouse";
                cmd = cmd + " " + std::to_string(gMousePos.x);
                cmd = cmd + " " + std::to_string(gMousePos.y);
                cmd = cmd + " " + std::to_string(event.button.button);
                cmd = cmd + " " + ((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ) ? "1" : "0");
                Core::Eval(cmd, env);
            }
            break;

        } //switch

    };

    app.OnShutDown = [&]() {
        Core::Eval("flux::ShutDown", env);
    };

    app.Execute();



    return 0;
}

