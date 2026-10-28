//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value Marshaller NOTE: EXPERIMENTAL!
//
// At the moment i use it for lazy translate. Example:
//
// // FunctionMap::RegisterFunction("flux::maxfps", [](std::span<Value> args, Value& ret) -> bool {
// //     DirectVariableAccessor vsyncer(&app.getSettings().FpsLimit);
// //     if (args.size() == 1) {
// //         vsyncer.toCpp(args[0]);
// //     }
// //     vsyncer.toValue(ret);
// //     return true;
// // }, "set / get the max fps");
//
//-----------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include "Value.h"
#include "ValueObject.h"
#include "BaseObjects.h"
#include "Globals.h"

namespace DreiZehn {

    // -------------------------------------------------------------------------
    struct ISymbolAccessor {
        virtual ~ISymbolAccessor() = default;
        virtual void toValue(Value& dest) const = 0;
        virtual void toCpp(const Value& src) const = 0;
    };
    // -------------------------------------------------------------------------
    template <typename MemberType>
    class DirectVariableAccessor : public ISymbolAccessor {
        MemberType* m_ptr;
    public:
        explicit DirectVariableAccessor(MemberType* ptr) : m_ptr(ptr) {}

        void toValue(Value& dest) const override {
            if constexpr (std::is_same_v<MemberType, double>) {
                dest = Value(*m_ptr);
            } else if constexpr (std::is_same_v<MemberType, float>) {
                dest = Value(*m_ptr);
            } else if constexpr (std::is_same_v<MemberType, uint32_t>) {
                dest = Value(*m_ptr);
            } else if constexpr (std::is_same_v<MemberType, int>) {
                dest = Value(*m_ptr);
            } else if constexpr (std::is_same_v<MemberType, bool>) {
                dest = Value(*m_ptr);
            }
        }

        void toCpp(const Value& src) const override {
            if constexpr (std::is_same_v<MemberType, double>) {
                *m_ptr = src.getDouble();
            } else if constexpr (std::is_same_v<MemberType, float>) {
                *m_ptr = src.getFloat();
            } else if constexpr (std::is_same_v<MemberType, int>) {
                *m_ptr = src.getInt();
            } else if constexpr (std::is_same_v<MemberType, uint32_t>) {
                *m_ptr = src.getUInt();
            } else if constexpr (std::is_same_v<MemberType, bool>) {
                *m_ptr = src.getBool();
            }
        }
    };

    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    class VariableMarshaller {
        std::unordered_map<uint32_t, std::unique_ptr<ISymbolAccessor>> mVariables;

    public:
        template <typename T>
        void registerSymbol(const uint32_t symbolId, T* varPtr) {
            mVariables[symbolId] = std::make_unique<DirectVariableAccessor<T>>(varPtr);
        }

        bool Load(const uint32_t symbolId, Value& val) {
            auto it = mVariables.find(symbolId);
            if (it != mVariables.end()) {
                it->second->toValue(val);
                return false;
            }
            return false;
        }

        bool Save(const uint32_t symbolId, const Value& val) {
            auto it = mVariables.find(symbolId);
            if (it != mVariables.end()) {
                it->second->toCpp(val);
                return true;
            }
            return false;
        }


        // struct example:
        // template <typename ClassType, typename T>
        // void linkMember(uint32_t symbolId, ClassType* instance, size_t offset) {
        //     mVariables[symbolId] = std::make_unique<StructMemberAccessor<ClassType, T>>(instance, offset);
        // }

    };


    // // =========================================================================
    // // VariableMarshaller - for globals only!
    // // =========================================================================
    // template <typename T>
    // class VariableMarshaller {
    // private:
    //     inline static std::unordered_map<
    //     uint32_t,
    //     std::unique_ptr<IFieldAccessor>> mVariables;
    // public:
    //     template <typename MemberType>
    //     void registerVar(uint32_t symbolId, MemberType T::*ptr) {
    //         auto impl = std::make_unique<FieldAccessorImpl<MemberType>>();
    //         impl->offset = reinterpret_cast<size_t>(&(reinterpret_cast<T*>(0)->*ptr));
    //         mVariables[symbolId]= std::move(impl);
    //     }
    //     void Setter(const T& cppVar, uint32_t symbolId) const {
    //         auto it = mVariables.find(symbolId);
    //         if (it != mVariables.end()) {
    //             Value val;
    //             *it->second->toScript(&cppVar, val);
    //             Globals::MasterVariableFrame->setVariable(symbolId,val, true);
    //         }
    //     }
    // };
    // // -------------------------------------------------------------------------
    // // FWD
    // template <typename T> class ValueMarshaller;
    //
    //
    // // =========================================================================
    // // Templates
    // // =========================================================================
    // // Base:
    // struct IFieldAccessor {
    //     size_t offset;
    //     virtual ~IFieldAccessor() = default;
    //     virtual void toScript(const void* cppBase, Value& dest) const = 0;
    //     virtual void toCpp(const Value& src, void* cppBase) const = 0;
    // };
    // // -------------------------------------------------------------------------
    // // Workers
    // template <typename MemberType>
    // struct FieldAccessorImpl : public IFieldAccessor {
    //
    //     //  ------- Double ---------------
    //     template<typename T = MemberType>
    //     typename std::enable_if_t<std::is_same_v<T, double>>
    //     writeToScript(const T& val, Value& dest) const {
    //         dest = Value(val);
    //     }
    //
    //     template<typename T = MemberType>
    //     typename std::enable_if_t<std::is_same_v<T, double>>
    //     readToCpp(const Value& src, T& val) const {
    //         val = src.getDouble();
    //     }
    //     //  ------- float ---------------
    //     template<typename T = MemberType>
    //     typename std::enable_if_t<std::is_same_v<T, float>>
    //     writeToScript(const T& val, Value& dest) const {
    //         dest = Value(val);
    //     }
    //
    //     template<typename T = MemberType>
    //     typename std::enable_if_t<std::is_same_v<T, float>>
    //     readToCpp(const Value& src, T& val) const {
    //         val = src.getDouble();
    //     }
    //     //  ------- Int ---------------
    //     template<typename T = MemberType>
    //     typename std::enable_if_t<std::is_same_v<T, int>>
    //     writeToScript(const T& val, Value& dest) const {
    //         dest = Value(val);
    //     }
    //
    //     template<typename T = MemberType>
    //     typename std::enable_if_t<std::is_same_v<T, int>>
    //     readToCpp(const Value& src, T& val) const {
    //         val = src.getInt();
    //     }
    //     //  ------- Bool ---------------
    //     template<typename T = MemberType>
    //     typename std::enable_if_t<std::is_same_v<T, bool>>
    //     writeToScript(const T& val, Value& dest) const {
    //         dest = Value(val);
    //     }
    //
    //     template<typename T = MemberType>
    //     typename std::enable_if_t<std::is_same_v<T, bool>>
    //     readToCpp(const Value& src, T& val) const {
    //         val = src.getBool();
    //     }
    //     // //FIXME  // Struct Example
    //     // template<typename T = MemberType>
    //     // typename std::enable_if_t<!std::is_same_v<T, int>>
    //     // writeToScript(const T& val, Value& dest) const {
    //     //     auto* subObj = new BaseValueObject();
    //     //     static ValueMarshaller<T> subMarshaller;
    //     //     subMarshaller.marshalAll(val, subObj->mDynamicFieldValues);
    //     //     dest = Value(subObj);
    //     // }
    //     //
    //     // template<typename T = MemberType>
    //     // typename std::enable_if_t<!std::is_same_v<T, int>>
    //     // readToCpp(const Value& src, T& val) const {
    //     //     ValueObject* obj = src.asPointerObject();
    //     //     if (obj && obj->mType == TypeBaseObject) {
    //     //         auto* subObj = static_cast<BaseValueObject*>(obj);
    //     //         static ValueMarshaller<T> subMarshaller;
    //     //         // recursive
    //     //         subMarshaller.unmarshalAll(subObj->mDynamicFieldValues, val);
    //     //     }
    //     // }
    //
    //     void toScript(const void* cppBase, Value& dest) const override {
    //         const auto& val = *reinterpret_cast<const MemberType*>(static_cast<const char*>(cppBase) + offset);
    //         writeToScript(val, dest);
    //     }
    //
    //     void toCpp(const Value& src, void* cppBase) const override {
    //         auto& val = *reinterpret_cast<MemberType*>(static_cast<char*>(cppBase) + offset);
    //         readToCpp(src, val);
    //     }
    // };
    //
    //
    // // =========================================================================
    // // StructMarshaller
    // // =========================================================================
    // template <typename T>
    // class StructMarshaller {
    // private:
    //     inline static std::vector<std::unique_ptr<IFieldAccessor>> fields;
    //
    // public:
    //     // Register
    //     template <typename MemberType>
    //     void registerField(MemberType T::*ptr) {
    //         auto impl = std::make_unique<FieldAccessorImpl<MemberType>>();
    //         impl->offset = reinterpret_cast<size_t>(&(reinterpret_cast<T*>(0)->*ptr));
    //         fields.push_back(std::move(impl));
    //     }
    //
    //     void marshalAll(const T& cppStruct, std::vector<Value>& destVector) const {
    //         if (destVector.size() < fields.size()) destVector.resize(fields.size());
    //         for (size_t i = 0; i < fields.size(); ++i) {
    //             fields[i]->toScript(&cppStruct, destVector[i]);
    //         }
    //     }
    //
    //     void unmarshalAll(const std::vector<Value>& srcVector, T& cppStruct) const {
    //         for (size_t i = 0; i < fields.size() && i < srcVector.size(); ++i) {
    //             fields[i]->toCpp(srcVector[i], &cppStruct);
    //         }
    //     }
    // };

} // namespace DreiZehn
