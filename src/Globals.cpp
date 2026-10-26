#include "Globals.h"
#include "core/GarbageCollection.h"

namespace DreiZehn {

 bool BaseBreath() {

        if (OnBreath) {
            return OnBreath();
        }
        return true;
    }

    namespace Tools {
        void PrintParseError(std::string message){
            errorf("Parse Error:\n%s\n| %03d |%s\n",
                   message.c_str(),
                   Globals::currentScriptLineNumber,
                   Globals::currentScriptLine.c_str());
        }

        void PrintRuntimeError(std::string message){
            errorf("RunTime Error: %s", message.c_str());
        }
    }

} //namespace
