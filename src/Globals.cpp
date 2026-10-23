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
            errorf("DreiZehn Parse Error in Line [%d]:%s\n\t%s\n",
                   Globals::currentScriptLineNumber,
                   Globals::currentScriptLine.c_str(),
                   message.c_str());
        }

        void PrintRuntimeError(std::string message){
            errorf("RunTime Error: %s", message.c_str());
        }
    }

} //namespace
