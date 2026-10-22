#include "Globals.h"
#include "core/GarbageCollection.h"

namespace DreiZehn {

 bool BaseBreath() {

        if (OnBreath) {
            return OnBreath();
        }
        return true;
    }

} //namespace
