//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Compiler Scope for Variable Rgisters woring with SymbolTable
// Example:
//   uint32_t symId = SymbolTable::insert("localX");
//   int slot = currentScope.getOrAssignSlot(symId);
//-----------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <cstdint>
#include <string>
#include <vector>
#include <cassert>

namespace DreiZehn {



    struct LoopInfo {
        std::vector<U32> continueOffsets; // continue jump point
        std::vector<U32> breakOffsets; // all break patches
    };

    // --------------------
    enum class FuncPatchType {
        None,
        Adress,
        VarCount,
        CFuncPrt
    };
    struct FunctionPatch {
        // FunctionMap::ScriptFunction* mFuncPtr = nullptr;
        void* mFuncPtr = nullptr;
        U32 mPatchByteCodePos = 0;
        FuncPatchType mPatchType = FuncPatchType::None;
    };

    inline std::vector<FunctionPatch> mFunctionPatches;
    // --------------------

    class CompilerScope {
    private:
        std::unordered_map<uint32_t, int> mSymbolIdToSlot;
        std::vector<uint32_t> mSlotToSymbolId;
        int mNextSlotIndex = 0;


    public:
        std::vector<LoopInfo> mLoopStack;




        uint32_t insert(uint32_t symbolId) {
            auto it = mSymbolIdToSlot.find(symbolId);
            if (it != mSymbolIdToSlot.end()) {
                return it->second;
            }

            uint32_t assignedSlot = mNextSlotIndex++;
            mSymbolIdToSlot[symbolId] = assignedSlot;
            mSlotToSymbolId.push_back(symbolId);
            return assignedSlot;
        }

        uint32_t allocateTemporarySlot() {
            return mNextSlotIndex++;
        }

        void freeTemporarySlot() {
            assert(mNextSlotIndex > 0 && "Compile ERROR too many slots released!!!!");
            mNextSlotIndex--;
        }
        // --------------------------------------
        int resolve(S32 symbolId) const {
            auto it = mSymbolIdToSlot.find(symbolId);
            if (it != mSymbolIdToSlot.end()) {
                return it->second;
            }
            return -1;
        }
        // --------------------------------------

        uint32_t getSymbolIdForSlot(int slot) const {
            return mSlotToSymbolId.at(slot);
        }

        int getLocalCount() const { return mNextSlotIndex; }
    };


// // class CompilerScope {
// // private:
// //     std::unordered_map<uint32_t, int> mSymbolIdToSlot;
// //     std::vector<uint32_t> mSlotToSymbolId;
// //     int mNextSlotIndex = 0;
// //
// // public:
// //     uint32_t insert(uint32_t symbolId) {
// //         auto it = mSymbolIdToSlot.find(symbolId);
// //         if (it != mSymbolIdToSlot.end()) {
// //             return it->second;
// //         }
// //
// //         uint32_t assignedSlot = mNextSlotIndex++;
// //         mSymbolIdToSlot[symbolId] = assignedSlot;
// //         mSlotToSymbolId.push_back(symbolId);
// //         return assignedSlot;
// //     }
// //
// //     uint32_t getSymbolIdForSlot(int slot) const {
// //         return mSlotToSymbolId.at(slot);
// //     }
// //
// //     int getLocalCount() const { return mNextSlotIndex; }
// // };

} // namespace DreiZehn
