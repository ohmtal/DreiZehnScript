//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value Object
//-----------------------------------------------------------------------------
#include <vector>

#include <string.h>

#include "Value.h"
#include "ValueObject.h"
#include "VariableFrame.h"
#include "FunctionMap.h"


namespace DreiZehn {

    // -------------------------------------------------------------------------
    int RegisterUserObjectType(std::string typeName) {

        gLastValueObjectType++;
        gUserObjectTypes[gLastValueObjectType] = {typeName};
        std::string typeIdent = "Type";
        typeIdent = typeIdent + typeName;
        FunctionMap::RegisterConstants(typeIdent, gLastValueObjectType);
        return gLastValueObjectType;
    }


    const char* GetObjectTypeName(ValueObject* object) {
        if (!object) return "";
        auto it = gUserObjectTypes.find(object->mType);
        if (it != gUserObjectTypes.end()) {
            return it->second.mName.c_str();
        }
        return "";
    }

    // -------------------------------------------------------------------------
    bool ValueObjectProperty::ValidateArgs( std::vector<Value>& args) {
        if (args.size() < mMinParams || args.size() > mMaxParams) {
            Tools::errorf("Method %s parameter error. min:%d max:%d\n%s\n", mName.c_str(),mMinParams, mMaxParams, mHelp.c_str());
            return false;
        }
        return true;
    }
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    ValueObject::ValueObject(int t) : mType(t) {
        GarbageCollection::insert(this);
        mObjectTypeName = GetObjectTypeName(this);
        // this make is slower !!
        // mDynmaicFieldIndex.reserve(256);
        // mDynamicFieldValues.reserve(256);
    }
    // -------------------------------------------------------------------------
    void ValueObject::insertUpdateDynamicField(uint32_t fieldSymbolId, const Value& value) {
        auto it = mDynmaicFieldIndex.find(fieldSymbolId);
        if (it == mDynmaicFieldIndex.end()) {
            mDynamicFieldValues.push_back(value);
            mDynmaicFieldIndex[fieldSymbolId] = mDynamicFieldValues.size() - 1;

        } else {
            size_t index = it->second;
            mDynamicFieldValues[index] = value;
        }
    }
    Value* ValueObject::getDynamicFieldPtr(uint32_t fieldSymbolId) {
        auto it = mDynmaicFieldIndex.find(fieldSymbolId);
        if (it == mDynmaicFieldIndex.end()) return nullptr;
        size_t index = it->second;
        return &mDynamicFieldValues[index];
    }

    size_t ValueObject::getDynamicFieldCount(){
        return mDynamicFieldValues.size();
    }
    Value* ValueObject::getDynamicFieldPtrByIndex(size_t index) {
        if (index >= mDynamicFieldValues.size()) return nullptr;
        return &mDynamicFieldValues[index];
    }

    bool ValueObject::SymbolIdByIndex(size_t index, uint32_t& symbolId) {
        if (index >= mDynamicFieldValues.size()) return false;
        for (const auto& [id, mappedIndex] : mDynmaicFieldIndex) {
            if (mappedIndex == index) {
                symbolId = id;
                return true;
            }
        }
        return false;
    }


    // -------------------------------------------------------------------------
    FunctionMap::ScriptFunction* ValueObject::getScriptMethod(uint32_t methodId) {
        FunctionMap::ScriptFunction* sf = nullptr;
        auto it = mMethodMap.find(methodId);
        if (it != mMethodMap.end()) {
            sf = it->second;
        } else {
            // lookup Object name
            std::string methodStr = SymbolTable::getName(methodId);
            uint32_t fnId = SymbolTable::insert(std::string(mObjectTypeName + "::" + methodStr));
            sf = FunctionMap::GetScriptFunction(fnId);
            if (!sf && !mClassName.empty()) {
                fnId = SymbolTable::insert(std::string(mClassName + "::" + methodStr));
                sf = FunctionMap::GetScriptFunction(fnId);
            }
            // add to mMethodMap
            if (sf) mMethodMap[methodId] = sf;
        }
        return sf;
    }

    // -------------------------------------------------------------------------
    std::string  ValueObject::toString() {
        char buff[64];
        snprintf(buff, sizeof(buff), "%s [%p] ", GetObjectTypeName(this), (void*)this);
        return std::string(buff);
    }
    // -------------------------------------------------------------------------
    void ValueObject::DumpBase() {
        Tools::printSeparator(40);
        Tools::printf(" ----- Dump %s ----- \n", mObjectTypeName.c_str());
        if (!mClassName.empty()) {

            Tools::printf("ClassName: %s\n",  mClassName.c_str());
            Tools::printSeparator(40);
        }


        if (mType > 0 && mType < gLastValueObjectType && gUserObjectTypes[mType].mProperties.size() > 0) {
            Tools::printf(" ----- %s Class Methods and Fields ----- \n", gUserObjectTypes[mType].mName.c_str());
            for (int i = 0; i < gUserObjectTypes[mType].mProperties.size(); i++) {
                Tools::printf(" %s%s  :: %s\n",
                            gUserObjectTypes[mType].mProperties[i].mIsMethod ? "->" : ".",
                            gUserObjectTypes[mType].mProperties[i].mName.c_str(),
                            gUserObjectTypes[mType].mProperties[i].mHelp.c_str()

                );
            }
            Tools::printSeparator(40);
        }


        if (mDynamicFieldValues.size() > 0) {
            Tools::printf(" ----- Dynamic Fields: %zu -----\n", mDynamicFieldValues.size());
            for ( auto& [key, index] : mDynmaicFieldIndex) {
                Tools::printf("  .%s = %s\n"
                , SymbolTable::getName(key).c_str()
                , mDynamicFieldValues[index].toString().c_str());
            }

            Tools::printSeparator(40);
        }
        if (mMethodMap.size() > 0) {
            Tools::printf(" ----- Script Methods (cached): %zu -----\n", mMethodMap.size());
            for ( auto& [key, method] : mMethodMap) {
                Tools::printf("  ->%s\n"
                , SymbolTable::getName(key).c_str()
                );
            }
        }
    }
    // -------------------------------------------------------------------------
    bool ValueObject::onMethodCall(uint32_t methodNameSymbolId,  std::vector<Value>& args, Value& ret) {

        // ----- User defined methods first ----
        FunctionMap::ScriptFunction* sf;
        if (Globals::gCurEnv) {
            sf = getScriptMethod(methodNameSymbolId);

            if (sf) {
                args.insert(args.begin(), Value(this));
                ret = FunctionMap::CallScriptFunction(*Globals::gCurEnv, sf, args);
                return true;
            }
        }

        // ------ Base methods ----------
        if (methodNameSymbolId == sClassNameMethodId) {
            if (args.size() != 1 || !args[0].isStringId()) {
                Tools::errorf("Usage: setClassName string ClassName\n");
                ret = Value(0);
                return true;
            }
            mClassName = args[0].getString();
            ret = Value(1);
            return true;
        }
       else if (methodNameSymbolId == sDumpMethodId) {
            DumpBase();

            ret = Value(1);
            return true;
        }

        // -------- isScriptMethod ----
        else if (methodNameSymbolId == sIsMethodId) {
            if (args.size() != 1 || !args[0].isStringId()) {
                Tools::errorf("Usage: isScriptMethod string MethodName\n");
                ret = Value(0);
                return true;
            }
            uint32_t loopupSymbol = SymbolTable::insert(args[0].getStringRef());
            if (getScriptMethod(loopupSymbol) != nullptr) {
                ret = Value(1);
            } else {
                ret = Value(0);
            }
            return true;
        }

        // ----------- getType ---------------
        else if (methodNameSymbolId == sGetTypeId) {
            if (args.size() != 0 ) {
                Tools::errorf("return the type id. Usage: getType\n");
            }
            ret = Value(mType);
            return true;
        }
        // ----------- setField ---------------
        else if (methodNameSymbolId == sSetFieldId) {
            if (args.size() != 2  || !args[0].isStringId() ) {
                Tools::errorf("set a field by fieldname Usage: \"fieldname\" value\n");
                ret = Value(0);
                return true;
            }
            ret = Value(onSetField(SymbolTable::insert(args[0].getStringRef()), args[1]));
            return true;
        }
        // ----------- getField ---------------
        else if (methodNameSymbolId == sGetFieldId) {
            if (args.size() != 1   ) {
                Tools::errorf("get a field value by fieldname Usage: \"fieldname\" OR index od dynamic field\n");
                ret = Value(0);
                return true;
            }
            Value* valPtr = nullptr;
            if (args[0].isStringId()) {
                // valPtr = getDynamicFieldPtr(SymbolTable::insert(args[0].getStringRef()));
                if (onGetField(SymbolTable::insert(args[0].getStringRef()),ret))
                    return true;
            } else {
                valPtr = getDynamicFieldPtrByIndex(args[0].getUInt());
            }

            if (valPtr) {
                ret = *valPtr;
            } else {
                Tools::warnf("Field %s not found.\n", args[0].toString().c_str());
            }
            return true;
        }
        // ----------- getFieldname ----------------
        else if (methodNameSymbolId == sGetFieldNameId) {
            if (args.size() != 1   ) {
                Tools::errorf("get a dynamic field name by index \n");
                ret = Value(0);
                return true;
            }
            uint32_t index = 0;
            if (SymbolIdByIndex(args[0].getUInt(), index) ) {
                ret = Value( SymbolTable::getName(index) );
            } else {
                Tools::warnf("Dynamic Field %s not found.\n", args[0].toString().c_str());
            }
            return true;
        }

        // ----------- getFieldCount ---------------
        else if (methodNameSymbolId == sGetFieldCountId) {
            if (args.size() != 0  ) {
                Tools::warnf("getFieldCount - should not get parameter - get the dynamic field count.\n");
            }
            ret = Value(static_cast<int32_t> (mDynamicFieldValues.size()));
            return true;
        }



        Tools::errorf("Runtime Error: %s Method %s not found.\n",mObjectTypeName.c_str(), SymbolTable::getName(methodNameSymbolId).c_str());
        return false;
    }

    // -------------------------------------------------------------------------
    bool ValueObject::onGetField(uint32_t fieldSymbolId, Value& ret) {
        Value* valPtr = getDynamicFieldPtr(fieldSymbolId);
        if (valPtr) {
            ret = *valPtr;
            return true;
        }

        Tools::errorf("Runtime Error GetField: Field %s not found.\n",SymbolTable::getName(fieldSymbolId).c_str());
        return false;
    }
    // -------------------------------------------------------------------------

    Value* ValueObject::onGetFieldPtr(uint32_t fieldSymbolId ) {
        Value* valPtr = getDynamicFieldPtr(fieldSymbolId);
        if (valPtr) {
            return valPtr;
        }
        else if (mAutoCreateDynamicFields ) {
            insertUpdateDynamicField(fieldSymbolId, Value(0));
            return  getDynamicFieldPtr(fieldSymbolId);
        }
        Tools::errorf("Runtime Error: Field %s not found.\n",SymbolTable::getName(fieldSymbolId).c_str());

        return nullptr;

    }
    // -------------------------------------------------------------------------

    bool ValueObject::onSetField(uint32_t fieldSymbolId, const Value& value) {
        Value* valPtr = onGetFieldPtr(fieldSymbolId);
        if (valPtr) {
            *valPtr = value;
            return true;
        }
        return false;
    }
    // -------------------------------------------------------------------------
    bool ValueObject::cloneBase(ValueObject* theClone) {
        if (! mSupportClone ){
            Tools::errorf("Cloning call but not set mSupportClone!\n");
            return false;
        }
        theClone->mClassName = this->mClassName;

        theClone->mMethodMap = this->mMethodMap;
        // theClone->mDynmaicFields = this->mDynmaicFields;
        theClone->mDynmaicFieldIndex = this->mDynmaicFieldIndex;
        theClone->mDynamicFieldValues = this->mDynamicFieldValues;


        return true;
    }


}
