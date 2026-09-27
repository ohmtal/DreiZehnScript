//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String / SymbolTable for fast map lookup ,
// NOTE: Singleton
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

class BaseStringTable {

protected:
    BaseStringTable() {
        internalInsert(""); //first is a emty string
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
// ----------------------------------------------------------------
// for Symbols like variable names, methods, fieldnames, ..
// ----------------------------------------------------------------
class SymbolTable:  BaseStringTable {
public:
    inline static uint32_t insert(const std::string& name) {
        return get().internalInsert(name);
    }

    inline static const size_t size() {
        return get().mNameToId.size();
    }

    inline static const std::string& getName(uint32_t id) {
        return get().internalGetName(id);
    }

    SymbolTable() = default;
    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) = delete;
    SymbolTable& operator=(SymbolTable&&) = delete;

private:
    static SymbolTable& get() {
        static SymbolTable instance;
        return instance;
    }

};
// ----------------------------------------------------------------
// For Strings only saved in Value
// ----------------------------------------------------------------
class StringTable:  BaseStringTable {
public:
    inline static uint32_t insert(const std::string& name) {
        return get().internalInsert(name);
    }
    inline static const size_t size() {
        return get().mNameToId.size();
    }
    inline static const std::string& get(uint32_t id) {
        return get().internalGetName(id);
    }

    StringTable() = default;
    StringTable(const StringTable&) = delete;
    StringTable& operator=(const StringTable&) = delete;
    StringTable(StringTable&&) = delete;
    StringTable& operator=(StringTable&&) = delete;

private:
    static StringTable& get() {
        static StringTable instance;
        return instance;
    }

};
