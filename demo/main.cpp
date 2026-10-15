//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// DreiZehn Implementation with Console
// demo and showcase how to add DreiZehn to you project.
//-----------------------------------------------------------------------------

#include <vector>
#include <iostream>
#include <sstream>
#include <string>


#include "DreiZehn.h"

#ifdef DREIZEHN_FENSTER
#include "addons/Fenster/Fenster.h"
#endif

#ifdef DREIZEHN_SDL3
#include "addons/SDL3/SDL3Functions.h"
#endif

#ifdef DREIZEHN_RAYLIB
#include "addons/Raylib/RaylibFunctions.h"
#endif

#include "demo/cpp-linenoise/linenoise.hpp"
#ifdef _WIN32
#include <conio.h> // Für _kbhit() unter Windows
#else
#include <sys/select.h>
#include <unistd.h>
#endif


static std::string hints_callback(
    const char* input,
    int& color,
    bool& bold
) {
    const std::string prefix = input ? input : "";
    if (prefix.empty()) {
        return "";
    }

    for (const auto& [key, value] : DreiZehn::FunctionMap::RegisteredFunctions) {
        const std::string name = SymbolTable::getName(key);

        if (name.compare(0, prefix.size(), prefix) == 0) {
            color = 35;
            bold = false;

            return name.substr(prefix.size());
        }
    }

    return "";
}

static void completion_callback(
    const char* input,
    std::vector<std::string>& completions
) {
    const std::string prefix = input ? input : "";

    for (const auto& [key, value] : DreiZehn::FunctionMap::RegisteredFunctions) {
        const std::string name = SymbolTable::getName(key);

        if (name.compare(0, prefix.size(), prefix) == 0) {
            completions.push_back(name);
        }
    }
}


bool read_line_nonblocking(const char* prompt, std::string& line) {
    static linenoise::EditState st;
    static bool is_initialized = false;

    if (!is_initialized) {
        linenoise::EditStart(st, prompt);
        is_initialized = true;
    }

    bool has_input = false;
    #ifdef _WIN32
    has_input = _kbhit();
    #else
    fd_set readfds;
    struct timeval tv = {0, 0};
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    has_input = (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0);
    #endif

    if (!has_input) {
        return false;
    }

    auto status = linenoise::EditFeed(st, line);

    if (status == linenoise::EditStatus::More) {
        return false;
    }

    linenoise::EditStop(st);
    is_initialized = false;

    return (status == linenoise::EditStatus::Done);
}

bool ConsoleCall(DreiZehn::Environment& env) {
    static std::string line;

    if (read_line_nonblocking("> ", line)) {
        if (line == "exit" || line == "quit") return false;

        if (!line.empty()) {
            linenoise::AddHistory(line.c_str());
        }

        std::stringstream stream(line);
        DreiZehn::RunScriptStream(stream, env);
    }
    return true;
}

// bool ConsoleCall(DreiZehn::Environment& env) {
//     static std::string line;
//     if (read_line_nonblocking("> ", line )) {
//         if (line == "exit") return false;
//         if (line == "quit") return false;
//
//         std::stringstream stream(line);
//         DreiZehn::RunScriptStream(stream, env);
//     }
//     return true;
// }

// -------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    using namespace DreiZehn;

    Environment env;
    Init(env, argc, argv);

    #ifdef DREIZEHN_FENSTER
    FunctionMap::ModuleRegistry::Register("Fenster", RegisterFensterFunctions);
    FunctionMap::ModuleRegistry::Register("FensterAudio", RegisterFensterAudioFunctions);
    #endif
    #ifdef DREIZEHN_SDL3
    FunctionMap::ModuleRegistry::Register("SDL3", RegisterSDL3Functions);
    #endif

    #ifdef DREIZEHN_RAYLIB
    FunctionMap::ModuleRegistry::Register("Raylib", Raylib::RegisterRaylibFunctions);
    #endif

    #ifdef DREIZEHN_LOADALL_MODULES
    FunctionMap::ModuleRegistry::LoadAll(env);
    #else
    FunctionMap::ModuleRegistry::Load("Core", env);
    #endif


    OnBreath = [&env]() -> bool {
        return ConsoleCall(env);
    };


    if (argc > 1) {

        std::string scriptPath = argv[1];

        bool success = RunScriptFile(scriptPath, env);

        env.shutDown();

        return success ? 0 : 1;

    }




    // -------------------------------------------------------------------------
    // Console Mode
    // -------------------------------------------------------------------------
    std::string line;

    linenoise::SetCompletionCallback(completion_callback);
    linenoise::SetHintsCallback(hints_callback);


    std::vector<OpenBlock> blockStack;
    Tools::printf(">> Console ready .. type quit or exit to escape <<\n");
    while (true) {

        if (!ConsoleCall(env)) break;
        Tools::sleep(16);

    }
    env.shutDown();
    return 0;
}
