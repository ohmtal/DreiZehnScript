//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// DLS Demo
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>

#include "DirtyLittleScript.h"


using namespace DLS;


namespace DLS {
extern void RegisterMathFunctions();
extern void RegisterStringFunctions();
extern void RegisterSDL3Functions();

}


int main(int argc, char* argv[]) {
    DLS::Init();
    RegisterMathFunctions();
    RegisterStringFunctions();
    RegisterSDL3Functions();


    int minargs = 1; // for more params
    for (int i = 1; i < argc; i++) {
        std::string strArg = argv[i];
        if (strArg == "--debug") {
            DEBUG_TOKEN = true;
            minargs++;
            break;
        }
    }


    if (argc > minargs) {
        Run(loadScript( argv[minargs]));
        return 0;
    }

    std::string mySrc = "";
    std::cout << "============= Dirty Little Script ===============\n";
    std::cout << "------------ SDL TEST  ---------------\n";


mySrc = R"(

foo = function
    print "Hello World!"
    print "Hello Tom"
end

)"; Run(mySrc);
// return 0;

    mySrc = R"(
print "SDL Window Example Dirty Little Script"


Window = 0
Renderer = 0
Event = 0
fontScale = 2.5

fnShutDown = function
    Renderer = SDL_DestroyRenderer Renderer
    Window = SDL_DestroyWindow Window
    Event = SDL_DestroyEvent Event
    print "fnShutDown pointers:" Renderer Window Event
end

# print fnShutDown

InitOK = SDL_Init
if InitOK == 0
    print "FAILED to init SDL3! it's:" InitOK
    return
end

print "creating window:"
Window = SDL_CreateWindow "Hello SDL3" 800 450 0

print "creating renderer Window:" Window
Renderer = SDL_CreateRenderer Window
print "Window:" Window "Renderer:" Renderer
print "fontScale is " fontScale
Event = SDL_CreateEvent
Running = true
while Running
    while SDL_PollEvent Event
        Type = SDL_GetEventType Event
        if Type == SDL_EVENT_QUIT
            Running = false
        end
        if Type == SDL_EVENT_KEY_DOWN
            if (SDL_GetEventKey Event) == SDLK_ESCAPE
                Running = false
            end
        end
    end
    SDL_SetRenderDrawColor Renderer 60 20 60 255
    SDL_RenderClear Renderer

    SDL_SetRenderScale Renderer  fontScale fontScale
    SDL_SetRenderDrawColor Renderer 55 155 155 255
    SDL_RenderDebugText Renderer (250 / fontScale)  (180 / fontScale) "Hello World!"

    SDL_RenderPresent Renderer
end

# SDL_DestroyRenderer Renderer ;
# SDL_DestroyWindow Window
fnShutDown

)"; Run(mySrc);

}
