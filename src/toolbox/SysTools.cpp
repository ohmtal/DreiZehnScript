//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// System Tools
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <future>
#include <chrono>



#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#include "SysTools.h"

namespace DreiZehn::SysTools{



    struct PipeDeleter {
        void operator()(FILE* p) const {
            if (p) pclose(p);
        }
    };
    // ------------------------------------------------------------------
    ExecResult ExecCommand(const std::string& command) {
        std::array<char, 128> buffer;
        std::string full_command = command + " 2>&1";

        std::unique_ptr<FILE, PipeDeleter> pipe(popen(full_command.c_str(), "r"));

        if (!pipe) {
            return { -1, {"Error: popen() failed"} };
        }

        std::vector<std::string> output;
        std::string current_line;

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            for (char c : buffer) {
                if (c == '\0') break;
                if (c == '\n' || c == '\r') {
                    if (!current_line.empty()) {
                        output.push_back(current_line);
                        current_line.clear();
                    }
                } else {
                    current_line += c;
                }
            }
        }
        if (!current_line.empty()) {
            output.push_back(current_line);
        }

        FILE* raw_pipe = pipe.release();
        int exit_code = pclose(raw_pipe);

        return { exit_code, output };
    }
    // ------------------------------------------------------------------
    std::future<ExecResult> ExecCommandAsync(const std::string& command) {
        return std::async(std::launch::async, ExecCommand, command);
    }
    // ------------------------------------------------------------------


}
