//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <string>
#include <iostream>
#include "toolbox/Tools.h"
#include "toolbox/SymbolTable.h"

namespace DreiZehn{
class Value;
namespace FunctionMap{ struct  ScriptFunction; }




// =============================================================================
// --- ValueObject ---
// =============================================================================



struct ValueObject {
    int mType;
    int mAssigned = 0;

    std::string mClassName = "";

    static inline uint32_t sClassNameMethodId = SymbolTable::insert("setClassName");
    static inline uint32_t sDumpMethodId = SymbolTable::insert("dump");
    static inline uint32_t sIsMethodId = SymbolTable::insert("isScriptMethod");

    // cache the type name for user methods
    std::string mObjectTypeName = "";

    std::unordered_map<uint32_t, FunctionMap::ScriptFunction* > mMethodMap;
    std::unordered_map<uint32_t, Value > mDynmaicFields;


    virtual ~ValueObject() = default;

    virtual bool onGetField(uint32_t fieldSymbolId, Value& ret);
    virtual Value* onGetFieldPtr(uint32_t fieldSymbolId);
    virtual bool onSetField(uint32_t fieldSymbolId, const Value& value);

    virtual bool onMethodCall(uint32_t methodNameSymbolId,  std::vector<Value>& args, Value& ret);
    virtual std::string toString();


    static void RegisterSymbols() {}

    // garbage collection control
    inline void setAssigned(bool v) {mAssigned += v ? 1 : -1;}



protected:
    ValueObject(int t);
};

// =============================================================================
// --- ValueObjectProperty, UserObjectDefintion  ---
// =============================================================================
struct ValueObjectProperty;

struct UserObjectDefintion {
    std::string mName;
    std::vector <ValueObjectProperty> mProperties;
};

inline int gLastValueObjectType = 0;
inline std::unordered_map <int,UserObjectDefintion> gUserObjectTypes;


struct ValueObjectProperty {
    uint32_t mSymbolId = 0;
    bool mIsMethod = true;
    std::string mName;
    std::string mHelp;
    uint32_t mMinParams = 0;
    uint32_t mMaxParams = 0;

    /// Static
    ValueObjectProperty(){}

    // Method
    ValueObjectProperty(std::string name,  uint32_t minParams, uint32_t maxParams, std::string help, int valueObjectTypeId )
    : mIsMethod(true),mName(name),  mHelp(help), mMinParams(minParams), mMaxParams(maxParams) {
        mSymbolId = SymbolTable::insert(name);
        if (valueObjectTypeId > 0) RegisterAtType(valueObjectTypeId);
    }
    // Field
    ValueObjectProperty(std::string name,  std::string help , int valueObjectTypeId )
    : mIsMethod(false),mName(name),  mHelp(help), mMinParams(0), mMaxParams(0) {
        mSymbolId = SymbolTable::insert(name);
        if (valueObjectTypeId > 0) RegisterAtType(valueObjectTypeId);
    }

    inline void RegisterAtType(int valueObjectTypeId) {
        auto it = gUserObjectTypes.find(valueObjectTypeId);
        if (it != gUserObjectTypes.end()) {
            it->second.mProperties.push_back(*this);
        }
        assert(true && "registerObjectProperty impossible on unknown valueObjectTypeId");
    }


    bool ValidateArgs( std::vector<Value>& args);
    // -------------------------------------------------------------------------
    // like ValidateArgs but also check the symbolId
    // NOTE return 1 on success 0 = no match = -1 == param error
    inline int matchMethod(uint32_t symbolId, std::vector<Value>& args) {
        if (this->mSymbolId != symbolId) return 0;
        if ( !ValidateArgs(args) ) return -1;
        return 1;
    }
    inline bool matchField(uint32_t symbolId) {
        if (this->mSymbolId != symbolId) return false;
        return true;
    }

};



// =============================================================================
// --- RegisterUserObjectType ---
// =============================================================================

inline int RegisterUserObjectType(std::string typeName, bool initial = false) {

    gLastValueObjectType++;
    gUserObjectTypes[gLastValueObjectType] = {typeName};
    return gLastValueObjectType;
}


inline const char* GetObjectTypeName(ValueObject* object) {
    if (!object) return "";
    auto it = gUserObjectTypes.find(object->mType);
    if (it != gUserObjectTypes.end()) {
        return it->second.mName.c_str();
    }
    return "";

}
// =============================================================================
// --- RegisterObjectProperty ---
// =============================================================================

// inline uint32_t RegisterObjectProperty(int valueObjectTypeId, ValueObjectProperty property) {
//     auto it = gUserObjectTypes.find(valueObjectTypeId);
//     if (it != gUserObjectTypes.end()) {
//         it->second.mProperties.push_back(property);
//         return property.mSymbolId;
//     }
//     assert(true && "registerObjectProperty impossible on unknown valueObjectTypeId");
//     return 0;
// }


} //namespace
