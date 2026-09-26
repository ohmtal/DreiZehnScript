#include <vector>
#include <iostream>
#include <sstream>

#include "engine/DreiZehn.h"
#include "engine/functions/DebugFunctions.h"
#include "engine/functions/ArrayFunctions.h"
#include "engine/functions/VectorObjectFunctions.h"

#ifdef DREIZEHN_FENSTER
#include "engine/functions/FensterFunctions.h"
#endif

#ifdef DREIZEHN_SDL3
#include "engine/functions/SDL3Functions.h"
#endif

#include "linenoise/linenoise.h"

void RegisterUserFunc() {
    using namespace DreiZehn;
    FunctionMap::RegisterFunction("fnFoo", [](std::vector<Value>& args, Value& ret) -> bool {
        printf("The Foo was here ...\n");
        ret = Value(4711.0815);
        return true;
    });
}



static void completion_callback(
    const char* input,
    linenoiseCompletions* completions
) {
    const std::string prefix = input ? input : "";

    for (const auto& [key, value] : DreiZehn::FunctionMap::RegisteredFunctions) {
        const std::string name = SymbolTable::getName(key);

        if (name.compare(0, prefix.size(), prefix) == 0) {
            linenoiseAddCompletion(completions, name.c_str());
        }
    }
}

bool read_line(const char* prompt, std::string& line) {
    char* buffer = linenoise(prompt);

    if (buffer == nullptr) {
        return false;
    }

    line = buffer;

    if (!line.empty()) {
        linenoiseHistoryAdd(buffer);
    }

    linenoiseFree(buffer);
    return true;
}

// -------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    using namespace DreiZehn;

    Environment env;
    RegisterCoreFunctions(env);
    RegisterArrayFunctions(env);
    RegisterVectorObjectFunctions();
    RegisterMathFunctions();
    RegisterDebugFunctions();
    RegisterUserFunc();

    #ifdef DREIZEHN_FENSTER
    RegisterFensterFunctions();
    #endif
    #ifdef DREIZEHN_SDL3
    RegisterSDL3Functions();
    #endif



    if (argc > 1) {
        std::string scriptPath = argv[1];

        bool success = RunScriptFile(scriptPath, env);

        env.shutDown();

        return success ? 0 : 1;
    }

    // -------------------------------------------------------------------------
    // Console Mode - handling multi line input for "for" and "fn"
    // -------------------------------------------------------------------------
    std::string line;

    linenoiseSetCompletionCallback(completion_callback);


    std::vector<OpenBlock> blockStack;

    while (true) {
        read_line("> ", line);
        if (line == "exit") break;
        if (line == "quit") break;

        std::stringstream stream(line);
        RunScriptStream(stream, env);

    }
    env.shutDown();
    return 0;
}
