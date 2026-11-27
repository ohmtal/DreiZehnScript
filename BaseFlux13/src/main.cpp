//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// BaseFlux13 Main source
// HACK using Eval for events! this enable both: Interpreter and ByteCode
//-----------------------------------------------------------------------------
#include <cstdio>
#include <string>
#include <filesystem>

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
#define FILE_EXT ".13"


BaseFlux::Main app;
ImConsole gConsole;
SDL_Point gMousePos = {0,0};



namespace fs = std::filesystem;
std::vector<fs::path> scriptFiles;

bool gDoUpdateCall; // = Con::isFunction("OnUpdate");
bool gDoRenderCall; // = Con::isFunction("OnRender");


std::string gDirectory = "assets/";
std::string gScriptFile = "assets/main.13";


using namespace DreiZehn;
extern void RegisterBaseFluxFunctions(Environment& env);

//-----------------------------------------------------------------------------
bool fetchScriptFiles() {
    scriptFiles.clear();
    std::string path = BaseFlux::Tools::getBasePath() + "assets/";
    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (entry.is_regular_file() && entry.path().extension() == FILE_EXT) {
                    scriptFiles.push_back(fs::relative(entry.path(), path));
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        SDL_Log("[error]%s",e.what());
        return false;
    }
    return true;
}

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
// ----------------------------------------------------------------------------
//  NOTE: moved me to Functionmap or something!
// #include "bytecode/VM.h"
// struct ScriptEvent {
//     Environment* envPtr = nullptr;
//     FunctionMap::ScriptFunction* sf = nullptr;
//     // std::vector<Value> arguments;
//     // BytecodeChunk chunk;
//     // size_t chunkArguments = 0;
//
//     std::string desc = "";
// };
// void RegisterScriptEvent( Environment* envPtr, ScriptEvent& event,
//         const std::string& funcName, const std::vector<std::string>& parameters
//         , const std::string& desc
//     ) {
//     if (!envPtr) return;
//     event.envPtr = envPtr;
//     event.sf =  DreiZehn::FunctionMap::RegisterScriptFunctionStub(funcName,parameters);
//     event.desc = desc;
// }
//
// // NOTE USING LITERALS!
// Value FireScriptEvent( ScriptEvent& event, std::vector<Value> arguments) {
//     if (!event.sf || !event.envPtr) {
//         Tools::errorf("FireScriptEvent - Init first - function not found!");
//         return NumberNullValue;
//     }
//     if ( Globals::ByteCodeMode ) {
//         //Compile:
//         // if (chunk.mByteCodes.size() == 0 || chunkArguments != arguments.size()) {
//
//             // Copy ! :/
//             BytecodeChunk chunk =  DreiZehn::VM::Compiler::MasterChunk;
//             // // DreiZehn::VM::Compiler::prepare(*event.envPtr,chunk);
//
//             size_t chunkArguments = arguments.size();
//             uint32_t ip = chunk.tell();
//             // arguments
//             for (auto& arg: arguments) {
//                 uint32_t constIdx = chunk.addConstant(arg);
//                 chunk.addConstant(constIdx);
//                 chunk.emit(OP_CONST);
//                 chunk.emit(constIdx);
//             }
//
//             // main function
//             chunk.emit(OP_CALL_SF_FUNCTION);
//             chunk.emit((U32)chunkArguments);
//             chunk.emitPointer(event.sf);
//             // chunk.emit(OP_POP_STACK); //?? need the result
//             chunk.emit(OP_EXIT);
//
//             // attach and patch all functions
//             // NOT! DreiZehn::VM::Compiler::attachFunctions(*event.envPtr,chunk);
//
//             return VM::runDirectThreadedVM(chunk, VM::scope.getLocalCount(), ip);
//         // }
//
//     } else {
//         return DreiZehn::FunctionMap::CallScriptFunction(*event.envPtr, event.sf, arguments);
//     }
//     return NumberNullValue;
// }

// ----------------------------------------------------------------------------
void RunScriptWrapper(Environment& env, std::string filename) {
    if (filename.empty()) return;
    env.getVM().resetChunk(); // doesnt matter if it's bytecode mode reset anyway
    RunScriptFile(filename, env, Globals::ByteCodeMode); // run
    // GarbageCollection::run(); // clean the old stuff
}
// ----------------------------------------------------------------------------
void onDraw(SDL_Renderer* renderer, Environment& env) {

    static bool showConsole = true;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Main")) {
            ImGui::MenuItem("Console", "GraveAccent", &showConsole);
            ImGui::MenuItem("---");
            if (ImGui::MenuItem("Dump Tape")) {
                env.getVM().dumpTape();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scripts")) {
            if (ImGui::MenuItem(("Reload " + env.CurrentFilename).c_str(),"CTRL-R")) {
                RunScriptWrapper(env,env.CurrentFilename);
            }

            ImGui::SeparatorText("Files");
            bool selected = false;
            for (const auto& f : scriptFiles) {
                selected = "assets/" + f.string() ==  env.CurrentFilename;
                if (ImGui::MenuItem(f.string().c_str(), nullptr, selected)) {
                    RunScriptWrapper(env,"assets/" + f.string());
                }
            }

            ImGui::EndMenu();
        }

        BytecodeChunk& chunk =  env.getVM().getChunk();
        ImGui::Text("[TAPE: %05zu]", chunk.mTape.size());
        ImGui::Text("[GLOB: %04zu]", env.getGlobalsCount());
        // ImGui::Text("[STK: %03zu]", DLS::Flux::getStackCount());
        ImGui::Text("[GARB: %05zu]", GarbageCollection::size());
        ImGui::EndMainMenuBar();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) showConsole = !showConsole;

    // FIXME RELOAD see also scriptfile above
    if (app.getImGuiIO()->KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_R))  {
            RunScriptWrapper(env,env.CurrentFilename);
        }
    }


    gConsole.Draw("DLS Console",&showConsole);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//  MAIN
// ----------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    DreiZehn::Environment env;
    DreiZehn::Init(env, argc, argv, MyLogger);
    FunctionMap::ModuleRegistry::LoadAll(env);
    RegisterBaseFluxFunctions(env);

    FunctionMap::ScriptEvent evSetup, evInit, evRender, evUpdate, evShutDown, evOnKey, evOnMouse;
    FunctionMap::RegisterScriptEvent(&env, evSetup,"flux::Setup",{},"When is start bevore sdl is initialited.");
    FunctionMap::RegisterScriptEvent(&env, evInit,"flux::Init",{},"When ready to load stuff");
    FunctionMap::RegisterScriptEvent(&env, evRender,"flux::Render",{"ft"},"When it's rendered.");
    FunctionMap::RegisterScriptEvent(&env, evUpdate,"flux::Update",{"dt"},"When it's updated.");
    FunctionMap::RegisterScriptEvent(&env, evShutDown,"flux::ShutDown",{},"When it's ShutDown.");
    FunctionMap::RegisterScriptEvent(&env, evOnKey,"flux::OnKey",{"keycode","isDown", "mod"},"When is is pressed or released.");
    FunctionMap::RegisterScriptEvent(&env, evOnMouse,"flux::OnMouse",{"x","y","button","isDown"},"mouse event.");


    argParser(argc, argv);

    initConsole(env);
    fetchScriptFiles();

    // inject Flux Object
    BaseValueObject* FluxObject = new BaseValueObject();

    size_t tmpid = env.reserveAndSaveGlobal(SymbolTable::insert("Flux"), Value(FluxObject));

    // Globals::MasterVariableFrame->setVariable(SymbolTable::insert("Flux"), Value(FluxObject), true);
    // FluxObject->mClassName ="Flux"; // mhh does not do anything!

    Tools::printf("Console ready. Mode: %s\n", Globals::ByteCodeMode ? "ByteCode" : "Interpreter");

    if (gDirectory.empty()) gDirectory = BaseFlux::Tools::getBasePath().c_str();
    if (!gDirectory.ends_with("/")) gDirectory += "/";
    app.mBasePath = gDirectory.c_str();


    bool success = RunScriptFile(gScriptFile, env, Globals::ByteCodeMode);
    if (!success) app.TerminateApplication();

    app.getSettings() = {
        .ScreenSize = { 800, 450},
        .FpsLimit = 60,
        .EnableVSync = false,
        .Company = "Ohmtal",
        .Caption = "BaseFlux13",
        .Version = "Version 0.1",

        // .sdlWindowFlagsOverwrite = SDL_WINDOW_BORDERLESS
    };
    // -------------------------------------------------------------------------
    // Core::Eval("flux::Load", env);
    FunctionMap::FireScriptEvent(env,evSetup,{});
    // -------------------------------------------------------------------------

    if ( !app.InitSDL() ) return 1;

    // postInitBindings_SDL();
    app.initImGui();


    // -------------------------------------------------------------------------
    // Core::Eval("flux::Init", env);
    FunctionMap::FireScriptEvent(env,evInit,{});
    // -------------------------------------------------------------------------
    app.OnLoopBegin = []() {
        // FIXME
        // ElfSDL3::ClearInputFrameTicks();
    };
    // -------------------------------------------------------------------------

    app.OnRender = [&env, &evRender](SDL_Renderer* renderer) {
        onDraw( renderer, env);
        // Core::Eval("flux::Render", env);
        FunctionMap::FireScriptEvent(env,evRender,{Value(BaseFlux::getFrameTime() * 100.f)});

    };

    // -------------------------------------------------------------------------
    app.OnUpdate = [&env, &evUpdate](const float dt) {
        FunctionMap::FireScriptEvent(env,evUpdate,{Value(dt)});
    };
    // -------------------------------------------------------------------------
    app.OnEvent = [&env,&evOnKey, &evOnMouse](const SDL_Event& event) {
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


                FunctionMap::FireScriptEvent(env,evOnKey, {
                                Value(event.key.key)
                                , Value(event.type == SDL_EVENT_KEY_DOWN )
                                , Value (event.key.mod)
                                });

            }
            break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {


                FunctionMap::FireScriptEvent(env,evOnMouse, {
                    Value(gMousePos.x), Value(gMousePos.y)
                    , Value( event.button.button)
                    , Value(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN )
                });
            }
            break;

        } //switch

    };

    app.OnShutDown = [&env,&evShutDown]() {
        // Core::Eval("flux::ShutDown", env);
        FireScriptEvent(env,evShutDown,{});
    };

    app.Execute();



    return 0;
}

