//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Misc
//-----------------------------------------------------------------------------
#include <string>
#include <algorithm> // std::sort
#include <cstdlib> // std::getenv

// for FetchEnv
#if defined(_WIN32)
#define environ _environ
#else
#ifdef __cplusplus
extern "C" {
    #endif
    extern char **environ;
    #ifdef __cplusplus
}
#endif
#endif

#include "core/FunctionMap.h"
#include "toolbox/SysTools.h"
#include <core/BaseObjects.h>
namespace DreiZehn {
    class Environment;

    void RegisterSysFunctions(Environment& env) {
         static bool registered = false; if (registered) return; registered = true;

          using namespace FunctionMap;

          // ---------------------------------------------------------------------
          // Environment
          // ---------------------------------------------------------------------
          RegisterFunction("sys::getEnv", [](std::vector<Value>& args, Value& ret) -> bool {
              if (args.size() < 1 || !args[0].isString()) {
                  Tools::printf("Usage example: core::getenv \"PATH\"");
                  ret = EmptyStringValue;
                  return true;
              }
              const char* env_p = std::getenv(args[0].getStringRef().c_str());
              if (env_p) {
                  ret = Value(std::string(env_p));
              } else {
                  ret = EmptyStringValue;
              }

              return true;
          }, "get a Environment variable. param variable name as string");

          RegisterFunction("sys::fetchEnv", [](std::vector<Value>& args, Value& ret) -> bool {
              ArrayObject* resObj = new ArrayObject();
              ret = Value(resObj);
              for (char** env = environ; *env != nullptr; ++env) {
                  std::string env_entry(*env);
                  size_t pos = env_entry.find('=');
                  if (pos != std::string::npos) {
                      std::string key = env_entry.substr(0, pos);
                      std::string value = env_entry.substr(pos + 1);

                      resObj->insertUpdateDynamicField(SymbolTable::insert(key),Value(value));
                  }
              }

              return true;
          }, "get all Environment variables and add them as fields to a Object");


          // ---------------------------------------------------------------------
          // Exec
          // ---------------------------------------------------------------------
          RegisterFunction("sys::exec", [](std::vector<Value>& args, Value& ret) -> bool {

              if (!ArgsCheckString("sys::exec", args, 1)) return true;
              SysTools::ExecResult res = SysTools::ExecCommand(args[0].getString());

              ArrayObject* resObj = new ArrayObject();
              ArrayObject* outputObj = new ArrayObject();
              ret = Value(resObj);
              resObj->insertUpdateDynamicField(SymbolTable::insert("command"), args[0]);
              resObj->insertUpdateDynamicField(SymbolTable::insert("exit_code"), Value(res.exit_code));
              resObj->insertUpdateDynamicField(SymbolTable::insert("output"), Value(outputObj));
              for (int i = 0; i < res.output.size(); i++) {
                outputObj->insertUpdateDynamicField(SymbolTable::insert(std::to_string(i)), Value(res.output[i]));
              }

              return true;
          }, "return execute an exteral Program and return a object with  fields int .exit_code, int output_count and .output_line_[num}]");
    }

}
