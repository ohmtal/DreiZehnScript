//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "CompilerScope.h"
#include "core/Types.h"
#include "toolbox/Tools.h"


namespace DreiZehn {



        U32 CompilerScope::insert(U32 varRegister) {
#ifdef DREIZEHN_DEBUG
            // Tools::printf("SCOPE INSERT: %u\n", varRegister);
#endif
            auto it = std::find(mSlots.begin(),mSlots.end(),varRegister);
            if (it != mSlots.end()) return varRegister;
            if (mSlots.size() <= varRegister ) mSlots.resize(varRegister+1);
            mSlots[varRegister] = varRegister;
            mNextSlotIndex++;
            return varRegister;
        }

        int CompilerScope::resolve(U32 varRegister) const {
            auto it = std::find(mSlots.begin(),mSlots.end(),varRegister);
            if (it != mSlots.end()) return varRegister;
            return -1;
        }
        U32 CompilerScope::getLocalCount() const {
#ifdef DREIZEHN_DEBUG
            Tools::printSeparator(80);
            Tools::printf("GET LOCAL COUNT: %u :", mNextSlotIndex);
            for (U32 index: mSlots) Tools::printf(" %u", index);
            Tools::printf("\n");
            Tools::printSeparator(80);
#endif
            return mNextSlotIndex;

        }


} // namespace DreiZehn
