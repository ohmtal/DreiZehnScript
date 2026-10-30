//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Flat Tape
//-----------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <vector>
#include <span>
#include "Types.h"
#include "Value.h"
#include "FunctionMap.h"

namespace DLS {

    // ------------------------------------------
    // FWD
    // ------------------------------------------
    struct VM;
    using OpFunc = void(*)(VM&);
    // ------------------------------------------
    // Slot
    // ------------------------------------------
    union Slot {
        OpFunc code;
        Value value;
        size_t jump_offset;
    };
    // ------------------------------------------
    // Upvalue
    // ------------------------------------------
    // An Upvalue represents a captured variable
    // struct Upvalue {
    //     uint64_t* location;   // Points to the value stack OR to the internal closed_value
    //     uint64_t closed_value; // Storage used once the variable leaves the stack
    //     size_t stack_slot;    // Keeps track of its original stack index (for closing)
    //
    //     Upvalue(uint64_t* slot, size_t index) : location(slot), closed_value(0), stack_slot(index) {}
    // };
    // ------------------------------------------
    // Closure
    // ------------------------------------------
    // The execution runtime representation of a function
    // struct Closure {
    //     size_t tape_address;
    //     std::vector<std::shared_ptr<Upvalue>> upvalues;
    // };

    // ------------------------------------------
    // CallFrame
    // ------------------------------------------
    // struct CallFrame {
    //     std::shared_ptr<Closure> closure; // The closure being executed
    //     size_t return_address;
    //     size_t stack_base;
    // };

    // ------------------------------------------
    // VM
    // ------------------------------------------
    struct VM {
        std::vector<Slot> tape;
        size_t pc = 0;
        std::vector<Value> stack;

        void run() {
            while (pc < tape.size()) {
                tape[pc++].code(*this);
            }
        }

        // std::vector<CallFrame> call_stack;

        // // Tracks all active open upvalues on the stack, sorted by stack index
        // std::vector<std::shared_ptr<Upvalue>> open_upvalues;
        //
        // // Helper to find an existing upvalue or create a new one
        // std::shared_ptr<Upvalue> capture_upvalue(size_t stack_index) {
        //     // Look from back to front to see if we already captured this exact slot
        //     for (auto it = open_upvalues.rbegin(); it != open_upvalues.rend(); ++it) {
        //         if ((*it)->stack_slot == stack_index) return *it;
        //     }
        //
        //     // Not captured yet, create a new open upvalue pointing to the stack slot
        //     auto uv = std::make_shared<Upvalue>(&stack[stack_index], stack_index);
        //     open_upvalues.push_back(uv);
        //     return uv;
        // }
        // ------------------------------------------
        // Stack functions
        // ------------------------------------------
        Value POP_STACK() {
            assert(stack.size() > 0 && "POP_STACK but stack empty!");
            Value result = stack.back(); stack.pop_back();
            return result;
        }
        void PUSH_STACK(Value val) {
            stack.push_back(val);
        }
    };
    // =========================================================================
    // ------------------------------------------
    // OP functions
    // ------------------------------------------
    void op_literal(VM& vm) {
        Value val = vm.tape[vm.pc++].value;
        vm.PUSH_STACK(val);
    }
    // ------------------------------------------

    void op_add(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();

        Value res = Value(a.getDouble() + b.getDouble());
        vm.PUSH_STACK(res);
    }

    void op_sub(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();

        Value res = Value(a.getDouble() - b.getDouble());
        vm.PUSH_STACK(res);
    }

    void op_mul(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();

        Value res = Value(a.getDouble() * b.getDouble());
        vm.PUSH_STACK(res);
    }

    void op_div(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();
        double right = b.getDouble();
        if (right == 0.0) [[unlikely]]  {
            vm.PUSH_STACK(Value(0.0));
            return ;
        }
        Value res = Value(a.getDouble() / right);
        vm.PUSH_STACK(res);
    }
    // ------------------------------------------
    void op_cfunc(VM& vm) {
        // args count
        U32 numArgs = vm.POP_STACK().getUInt();
        U32 symid = vm.POP_STACK().getUInt();
        const FunctionMap::CallBack* cb = FunctionMap::GetCFunction(symid);
        if (!cb) [[unlikely]]{
            for (size_t i = 0; i < numArgs; ++i) {
                vm.POP_STACK();
            }
            //FIXME error!? or special push value
            vm.PUSH_STACK(Value(0));
            return;
        }

        if (numArgs <= 12) {
            Value evaluatedArgsStack[12];

            for (size_t i = 0; i < numArgs; ++i) {
                evaluatedArgsStack[i] = vm.POP_STACK();
            }
            Value returnValue = Value(0);
            bool success = (*cb)(std::span<Value>(evaluatedArgsStack, numArgs), returnValue);

            if (!success) [[unlikely]] {
                Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
            }
            vm.PUSH_STACK(returnValue);
        }
        else {
            //FIXME more than 12!
        //     std::vector<Value> evaluatedArgs;
        //     evaluatedArgs.reserve(numArgs);
        //     for (auto& argExpr : arguments) {
        //         if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
        //     }
        //
        //     Value returnValue = NumberNullValue;
        //     bool success = (*cb)(evaluatedArgs, returnValue); // bzw. std::span(evaluatedArgs)
        //
        //     if (!success) [[unlikely]] {
        //         Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
        //     }
        //     return returnValue;
        }
    }

    // ------------------------------------------
}

