#include "Globals.h"
#include "core/GarbageCollection.h"

namespace DreiZehn {

    // bool BaseBreath() {
    //     if (OnBreath) {
    //         return OnBreath();
    //     }
    //     return true;
    // }
    // ------------------------------------------------
    namespace Tools {
        void PrintParseError(const std::string& message , const std::string& p2 , const std::string& p3 ){
            errorf("Parse Error:\n%s %s %s\n| %03d |%s\n",
                   message.c_str()
                   , p2.c_str(),p3.c_str()
                   ,Globals::currentScriptLineNumber
                   ,Globals::currentScriptLine.c_str());
        }

        void PrintRuntimeError(const std::string& message, const std::string& p2 , const std::string& p3 ){
            errorf("RunTime Error: %s %s %s", message.c_str(), p2.c_str(),p3.c_str());
        }
    }

} //namespace
