//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String / SymbolTable for fast map lookup ,
// NOTE: Singleton
// 2026-11-14: heavy update for c++20 optimize and thread safety
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include <deque>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string_view>



    class BaseStringTable {
    protected:
        BaseStringTable() {
            internalInsert("");
        }

        uint32_t internalInsert(std::string_view name) {
            {
                std::shared_lock readLock(mMutex);
                if (auto it = mNameToId.find(name); it != mNameToId.end()) {
                    return it->second;
                }
            }

            std::unique_lock writeLock(mMutex);
            if (auto it = mNameToId.find(name); it != mNameToId.end()) {
                return it->second;
            }

            uint32_t newId = static_cast<uint32_t>(mIdToName.size());
            mIdToName.emplace_back(name);

            mNameToId.emplace(mIdToName.back(), newId);
            return newId;
        }

        const std::string& internalGetName(uint32_t id) const {
            std::shared_lock readLock(mMutex);
            return mIdToName.at(id);
        }

        size_t internalSize() const {
            std::shared_lock readLock(mMutex);
            return mIdToName.size();
        }

    private:
        struct StringHash {
            using is_transparent = void;
            [[nodiscard]] size_t operator()(std::string_view sv) const noexcept {
                return std::hash<std::string_view>{}(sv);
            }
        };

        mutable std::shared_mutex mMutex;
        std::unordered_map<std::string_view, uint32_t, StringHash, std::equal_to<>> mNameToId;
        std::deque<std::string> mIdToName;
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
        return get().internalSize();
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
        return get().internalSize();
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

