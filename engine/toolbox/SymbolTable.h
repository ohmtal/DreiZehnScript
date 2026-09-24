//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// SymbolTable for fast map lookup ,
// NOTE: Singleton
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

class SymbolTable {
public:
    inline static uint32_t insert(const std::string& name) {
        return get().internalInsert(name);
    }

    inline static const std::string& getName(uint32_t id) {
        return get().internalGetName(id);
    }

    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) = delete;
    SymbolTable& operator=(SymbolTable&&) = delete;

private:
    // SymbolTable() = default;
    SymbolTable() {
        internalInsert(""); //first is a emty string
    }

    static SymbolTable& get() {
        static SymbolTable instance;
        return instance;
    }

    uint32_t internalInsert(const std::string& name) {
        auto it = mNameToId.find(name);
        if (it != mNameToId.end()) {
            return it->second;
        }

        uint32_t newId = static_cast<uint32_t>(mIdToName.size());
        mIdToName.push_back(name);
        mNameToId[name] = newId;
        return newId;
    }

    const std::string& internalGetName(uint32_t id) const {
        return mIdToName.at(id);
    }

    std::unordered_map<std::string, uint32_t> mNameToId;
    std::vector<std::string> mIdToName;
};
