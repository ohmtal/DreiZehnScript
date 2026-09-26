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


enum ValueObjectType {
    String   = 0,
    Userdata = 1
};

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

    static void RegisterSymbols() {}

    // garbage collection control
    inline void setAssigned(bool v) {mAssigned += v ? 1 : -1;}



protected:
    ValueObject(int t) : mType(t) {}
};

// =============================================================================
// --- ValueObjectProperty, UserObjectDefintion  ---
// =============================================================================

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
    ValueObjectProperty(std::string name,  uint32_t minParams, uint32_t maxParams, std::string help)
    : mIsMethod(true),mName(name),  mHelp(help), mMinParams(minParams), mMaxParams(maxParams) {
        mSymbolId = SymbolTable::insert(name);
    }
    // Field
    ValueObjectProperty(std::string name,  std::string help)
    : mIsMethod(false),mName(name),  mHelp(help), mMinParams(0), mMaxParams(0) {
        mSymbolId = SymbolTable::insert(name);
    }

    bool ValidateArgs( std::vector<Value>& args);

};

struct UserObjectDefintion {
    std::string mName;
    std::vector <ValueObjectProperty> mProperties;
};

inline int gLastValueObjectType = 1;
inline std::unordered_map <int,UserObjectDefintion> gUserObjectTypes;


// =============================================================================
// --- RegisterUserObjectType ---
// =============================================================================

inline int RegisterUserObjectType(std::string typeName, bool initial = false) {
    if (gLastValueObjectType == 1) {
        gUserObjectTypes[0] = {"String"};
        gUserObjectTypes[1] = {"UserData"};
    }
    if (initial) return 0;
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

inline uint32_t RegisterObjectProperty(int valueObjectTypeId, ValueObjectProperty property) {
    auto it = gUserObjectTypes.find(valueObjectTypeId);
    if (it != gUserObjectTypes.end()) {
        it->second.mProperties.push_back(property);
        return property.mSymbolId;
    }
    assert(true && "registerObjectProperty impossible on unknown valueObjectTypeId");
    return 0;
}
// =============================================================================
// --- StringValueObject, UserdataValueObject  ---
// =============================================================================
struct StringValueObject : public ValueObject {
    std::string mValue;
    StringValueObject(std::string str) : ValueObject(ValueObjectType::String), mValue(std::move(str)) {}
    bool onMethodCall(uint32_t methodNameSymbolId,  std::vector<Value>& args, Value& ret) override;


    static inline ValueObjectProperty toNumberProp;
    static inline ValueObjectProperty getLenProp;
    static inline ValueObjectProperty getCharProp;
    static void RegisterSymbols();

};

struct UserdataValueObject : public ValueObject {
    void* mRawCustomPointer;
    UserdataValueObject(void* p) : ValueObject(ValueObjectType::Userdata), mRawCustomPointer(p) {}
};
// =============================================================================
// --- InitSubSystem  ---
// =============================================================================
inline void InitSubSystem() {
    RegisterUserObjectType("",true);
    StringValueObject::RegisterSymbols();
}

} //namespace
