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

inline int gLastValueObjectType = 1;
inline std::unordered_map <int,std::string> gUserObjectTypes; //NOTE Type Registey

inline int registerUserObjectType(std::string typeName) {
    if (gLastValueObjectType == 1) {
        gUserObjectTypes[0] = "String";
        gUserObjectTypes[1]  = "UserData";
    }
    gLastValueObjectType++;
    gUserObjectTypes[gLastValueObjectType] = typeName;
    return gLastValueObjectType;
}


// TODO ... i need a id system for that ?! or as static .. later
struct ValueObjectMethod {
    uint32_t mSymbolId = 0;
    bool mIsMethod = true;
    std::string mName;
    std::string mHelp;
    uint32_t mMinParams = 0;
    uint32_t mMaxParams = 0;

    // for static define
    ValueObjectMethod(){};
    // method
    ValueObjectMethod(std::string name,  uint32_t minParams, uint32_t maxParams, std::string help)
    : mIsMethod(true),mName(name),  mHelp(help), mMinParams(minParams), mMaxParams(maxParams) {
        mSymbolId = SymbolTable::insert(name);
    }
    // property
    ValueObjectMethod(std::string name,  std::string help)
    : mIsMethod(false),mName(name),  mHelp(help), mMinParams(0), mMaxParams(0) {
        mSymbolId = SymbolTable::insert(name);
    }

    bool ValidateArgs( std::vector<Value>& args);

};

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

    // garbage collection control
    inline void setAssigned(bool v) {mAssigned += v ? 1 : -1;}

protected:
    ValueObject(int t) : mType(t) {}
};

struct StringValueObject : public ValueObject {
    std::string mValue;
    StringValueObject(std::string str) : ValueObject(ValueObjectType::String), mValue(std::move(str)) {}
    bool onMethodCall(uint32_t methodNameSymbolId,  std::vector<Value>& args, Value& ret) override;
};

struct UserdataValueObject : public ValueObject {
    void* mRawCustomPointer;
    UserdataValueObject(void* p) : ValueObject(ValueObjectType::Userdata), mRawCustomPointer(p) {}
};


} //namespace
