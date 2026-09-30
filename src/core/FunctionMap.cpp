//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value Object
//-----------------------------------------------------------------------------
#include <vector>

#include <string.h>

#include "Value.h"
#include "FunctionMap.h"
#include "Environment.h"

namespace DreiZehn::FunctionMap {
    // -------------------------------------------------------------------------
    Value CallScriptFunction(Environment& env,const ScriptFunction* sf, std::vector<Value>& args)
    {
        if (sf) {
            auto& func = *(sf);
            Environment localEnv(&env);

            for (size_t i = 0; i < func.parameterNames.size(); ++i) {
                if (i < args.size()) {
                    localEnv.getVariableFrame()->setVariable(
                        SymbolTable::insert(func.parameterNames[i])
                        , args[i]
                        , true);
                }
            }

            for (auto& statement : func.body) {
                FlowSignal sig = env.execute(statement.get(), localEnv);

                if (sig == FlowSignal::Return) {
                    Value retVal = localEnv.getVariableFrame()->getVariable(SymbolTable::insert("__return_value__"));
                    return retVal;
                }
            }
            return Value(0);
        }
        return Value(0);
    }
    // -------------------------------------------------------------------------

}
