#include <vector>
#include <iostream>
#include <sstream>
#include <sys/select.h>
#include <unistd.h>
#include <string>


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



bool read_line_nonblocking(const char* prompt, std::string& line) {
    static struct linenoiseState ls;
    static char buf[4096];
    static bool is_initialized = false;

    if (!is_initialized) {
        linenoiseEditStart(&ls, -1, -1, buf, sizeof(buf), prompt);
        is_initialized = true;
    }

    fd_set readfds;
    struct timeval tv = {0, 0};

    FD_ZERO(&readfds);
    FD_SET(ls.ifd, &readfds);

    int retval = select(ls.ifd + 1, &readfds, NULL, NULL, &tv);

    if (retval <= 0) {
        return false;
    }

    char *result_buffer = linenoiseEditFeed(&ls);

    if (result_buffer == linenoiseEditMore) {
        return false;
    }

    linenoiseEditStop(&ls);
    is_initialized = false;

    if (result_buffer != nullptr) {
        line = result_buffer;
        if (!line.empty()) {
            linenoiseHistoryAdd(result_buffer);
        }
        linenoiseFree(result_buffer);
        return true;
    }

    line = "";
    return false;
}

// -------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    using namespace DreiZehn;

    Environment env;

    InitSubSystem();
    RegisterCoreFunctions(env);
    RegisterMathFunctions();
    RegisterUserFunc();

    RegisterDebugFunctions();
    RegisterArrayFunctions(env);
    RegisterVectorObjectFunctions();
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
