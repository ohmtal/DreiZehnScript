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
    // Variables
    // ------------------------------------------
    struct VariableStorage {
        std::unordered_map<U32,Value> mGlobals;
        std::vector<Value> mLocals; // FIXME locals with callframe offset and stack id's '

        bool isGlobal(U32 id) {
            return (mGlobals.find(id) != mGlobals.end());
        }

        void saveGlobal(U32 id, const Value& value) {
            mGlobals[id] = value;
        }

        Value getGlobal(U32 id, bool autoCreate = false) {

            auto it = mGlobals.find(id);
            if (it != mGlobals.end()) [[likely]] {
                return it->second;
            }
            Value v = Value(0.0);
            if (autoCreate) {
                return mGlobals[id] =  v;
            }

            std::string varName = SymbolTable::getName(id);
            Tools::warnf("Variable not found: %s\n", varName.c_str());
            return v;
        }

    } Variables;

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
        size_t tapeChunk;
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
    // // ------------------------------------------
    // // Closure
    // // ------------------------------------------
    // // The execution runtime representation of a function
    // struct Closure {
    //     size_t tape_address;
    //     std::vector<std::shared_ptr<Upvalue>> upvalues;
    // };
    //
    // // ------------------------------------------
    // // CallFrame
    // // ------------------------------------------
    // struct CallFrame {
    //     std::shared_ptr<Closure> closure; // The closure being executed
    //     size_t return_address;
    //     size_t stack_base;
    // };

    // ------------------------------------------
    // Loops
    // ------------------------------------------
    struct LoopContext {
        size_t continue_target;
        std::vector<size_t> break_slots;
    };

    // ------------------------------------------
    // VM
    // ------------------------------------------
    struct VM {
        std::vector<Slot> tape;
        size_t pc = 0;
        std::vector<Value> stack;

        std::vector<LoopContext> mLoopStack;

        void run() {
            while (pc < tape.size()) {
                tape[pc++].code(*this);
            }
        }

        // std::vector<CallFrame> call_stack;

        // Tracks all active open upvalues on the stack, sorted by stack index
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
        // Tape functions
        // ------------------------------------------
        void emitCode(const OpFunc& opcode ) { tape.push_back({.code = opcode}); }
        void emitValue(const Value& value)   { tape.push_back({.value = value}); }
        void emitTapeChunk(const size_t tapeChunk)   { tape.push_back({.tapeChunk = tapeChunk}); }

        void emitJumpOffset(const size_t offset)   { tape.push_back({.jump_offset = offset}); }
        void patchJumpOffset(const size_t tapePosition, const size_t offset)   { tape[tapePosition].jump_offset = offset; }

        size_t tell() { return tape.size(); }

        Slot& peekSlot(size_t reverseOffset) {
            return tape[pc - reverseOffset];
        }

        Slot& advanceSlot() {
            assert (pc < tape.size() && "TAPE OVERFLOW");
            return tape[pc++];
        }
        // ------------------------------------------
        // Stack functions
        // ------------------------------------------
        Value POP_STACK() {
            assert(stack.size() > 0 && "POP_STACK but stack empty!");
            Value result = stack.back(); stack.pop_back();
            return result;
        }
        Value PEEK_STACK() {
            assert(stack.size() > 0 && "PEEK_STACK but stack empty!");
            Value result = stack.back();
            return result;
        }
        void PUSH_STACK(Value val) {
            stack.push_back(val);
        }
    };
    // =========================================================================
    // ------------------------------------------
    // Exit (last for jump offsets)
    // ------------------------------------------
    void op_exit(VM& vm) {}
    // ------------------------------------------
    // POP
    // ------------------------------------------
    void op_pop_stack(VM& vm) {
        vm.POP_STACK();
    }
    void op_pop_stack_count(VM& vm) {
        size_t count = vm.advanceSlot().tapeChunk;
        for (size_t i = 0; i < count; i++) {
            vm.POP_STACK();
        }
    }

    // ------------------------------------------
    // Variables
    // ------------------------------------------
    void op_save_global(VM& vm) {
         U32 symid = (U32)vm.advanceSlot().tapeChunk;
         Variables.saveGlobal(symid,vm.POP_STACK());
    }

    void op_load_global(VM& vm) {
        U32 symid = (U32)vm.advanceSlot().tapeChunk;
        vm.PUSH_STACK( Variables.getGlobal(symid) );
    }


    // void op_load_local(VM& vm) {
    //     size_t relative_var_offset = vm.advanceSlot().tapeChunk;
    //     size_t absolute_index = vm.call_stack.back().stack_base + relative_var_offset;
    //     vm.stack.push_back(vm.stack[absolute_index]);
    // }
    //
    // void op_save_local(VM& vm) {
    //     size_t relative_var_offset = vm.advanceSlot().tapeChunk;
    //     size_t absolute_index = vm.call_stack.back().stack_base + relative_var_offset;
    //     vm.stack[absolute_index] = vm.stack.back();
    // }
    // ------------------------------------------
    // Literal
    // ------------------------------------------
    void op_literal(VM& vm) {
        Value val = vm.tape[vm.pc++].value;
        vm.PUSH_STACK(val);
    }
    // ------------------------------------------
    // Basic Math
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
    // Comparison
    // ------------------------------------------
    void op_greater(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() > b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_lower(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() < b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_equal(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() == b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_not_equal(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() != b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_lower_equal(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() <= b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_greater_equal(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() >= b.getDouble());
        vm.PUSH_STACK(res);
    }
    // ------------------------------------------
    // JUMP
    // ------------------------------------------
    void op_jump(VM& vm) {
        size_t target = vm.advanceSlot().jump_offset;
        vm.pc = target;
    }

    void op_jump_if_true(VM& vm) {
        Value a = vm.POP_STACK();
        size_t target = vm.advanceSlot().jump_offset;
        if (a.getBool()) {
            vm.pc = target;
        }
    }
    void op_jump_if_false(VM& vm) {
        Value a = vm.POP_STACK();
        size_t target = vm.advanceSlot().jump_offset;
        if (!a.getBool()) {
            vm.pc = target;
        }
    }

    void op_jump_if_true_no_pop(VM& vm) {
        Value a = vm.PEEK_STACK();
        size_t target = vm.advanceSlot().jump_offset;
        if (a.getBool()) {
            vm.pc = target;
        }
    }
    void op_jump_if_false_no_pop(VM& vm) {
        Value a = vm.PEEK_STACK();
        size_t target = vm.advanceSlot().jump_offset;
        if (!a.getBool()) {
            vm.pc = target;
        }
    }
    // ------------------------------------------
    // C-Function call
    // ------------------------------------------
    void op_cfunc(VM& vm) {
        // args count
        U32 symid = (U32) vm.advanceSlot().tapeChunk;
        U32 numArgs = (U32) vm.advanceSlot().tapeChunk;

        const FunctionMap::CallBack* cb = FunctionMap::GetCFunction(symid);
        if (!cb) [[unlikely]]{
            for (size_t i = 0; i < numArgs; ++i) {
                vm.POP_STACK();
            }
            //FIXME error!? or special push value
            vm.PUSH_STACK(Value(0));
            return;
        }
        // Tools::printf("CALL FUNC: %s [args:%f]\n", SymbolTable::getName(symid).c_str(), numArgs);

        if (numArgs <= 12) {
            Value evaluatedArgsStack[12];

            for (S64 i = numArgs ; i > 0  ; i--) {
                evaluatedArgsStack[i - 1] = vm.POP_STACK();
            }
            Value returnValue = Value(0);
            bool success = (*cb)(std::span<Value>(evaluatedArgsStack, numArgs), returnValue);

            if (!success) [[unlikely]] {
                Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
            }
            vm.PUSH_STACK(returnValue);
        } else {
            // --- more than 12 parameters fallback ---
            std::vector<Value> evaluatedArgs;
            evaluatedArgs.resize(numArgs);
            for (S64 i = numArgs ; i > 0  ; i--) {
                evaluatedArgs[i - 1] = vm.POP_STACK();
            }
            Value returnValue = Value(0);
            bool success = (*cb)(evaluatedArgs, returnValue);

            if (!success) [[unlikely]] {
                Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
            }
            vm.PUSH_STACK(returnValue);
        }
    }

    // ------------------------------------------
}

