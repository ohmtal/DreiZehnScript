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



// =============================================================================
// --- ValueObject ---
// =============================================================================

struct ValueObject {
    int mType;
    int mAssigned = 0;
    virtual ~ValueObject() = default;

    inline virtual bool onGetField(uint32_t fieldSymbolId, Value& ret) {
        Tools::errorf("Runtime Error GetField: Field %s not found.\n",SymbolTable::getName(fieldSymbolId).c_str());
        return false;
    }

    inline virtual Value* onGetFieldPtr(uint32_t fieldSymbolId) {
        Tools::errorf("Runtime Error GetField: Field %s not found.\n",SymbolTable::getName(fieldSymbolId).c_str());
        return nullptr;
    }

    inline virtual bool onSetField(uint32_t fieldSymbolId, const Value& value) {
        Tools::errorf("Runtime Error SetField: Field %s not found.\n",SymbolTable::getName(fieldSymbolId).c_str());
        return false;
    }

    inline virtual bool onMethodCall(uint32_t methodNameSymbolId,  std::vector<Value>& args, Value& ret) {
        Tools::errorf("Runtime Error: Method %s not found.\n",SymbolTable::getName(methodNameSymbolId).c_str());
        return false;
    }

    virtual std::string toString();


    static void RegisterSymbols() {}

    // garbage collection control
    inline void setAssigned(bool v) {mAssigned += v ? 1 : -1;}



protected:
    ValueObject(int t) : mType(t) {}
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
