//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Flat Tape
//-----------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "Types.h"
#include "Value.h"

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

        Value run() {
            while (pc < tape.size()) {
                tape[pc++].code(*this);
            }
            if (stack.size() > 0)
                return stack.back();
            return Value("");
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
    };
    // ------------------------------------------
    // CallFrame
    // ------------------------------------------
    void op_literal(VM& vm) {
        Value val = vm.tape[vm.pc++].value;
        vm.stack.push_back(val);
    }

    void op_add(VM& vm) {
        Value b = vm.stack.back(); vm.stack.pop_back();
        Value a = vm.stack.back(); vm.stack.pop_back();

        Value res = Value(a.getDouble() + b.getDouble());
        vm.stack.push_back(res);
    }

    void op_sub(VM& vm) {
        Value b = vm.stack.back(); vm.stack.pop_back();
        Value a = vm.stack.back(); vm.stack.pop_back();

        Value res = Value(a.getDouble() - b.getDouble());
        vm.stack.push_back(res);
    }

    void op_mul(VM& vm) {
        Value b = vm.stack.back(); vm.stack.pop_back();
        Value a = vm.stack.back(); vm.stack.pop_back();

        Value res = Value(a.getDouble() * b.getDouble());
        vm.stack.push_back(res);
    }

    void op_div(VM& vm) {
        Value b = vm.stack.back(); vm.stack.pop_back();
        Value a = vm.stack.back(); vm.stack.pop_back();
        double right = b.getDouble();
        if (right == 0.0) [[unlikely]]  {
            vm.stack.push_back(Value(0.0));
            return ;
        }
        Value res = Value(a.getDouble() / right);
        vm.stack.push_back(res);
    }
    // ------------------------------------------
    //
    // ------------------------------------------
}

