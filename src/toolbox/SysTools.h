//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// System Tools
//-----------------------------------------------------------------------------
#pragma once

#include <future>

namespace DreiZehn::SysTools {


    struct ExecResult {
        int exit_code;
        std::vector<std::string> output;
    };

    ExecResult ExecCommand(const std::string& command) ;

    std::future<ExecResult> ExecCommandAsync(const std::string& command);
    /* example:
         std::future<ExecResult> async_result = php_exec_async(command);

        while (async_result.wait_for(std::chrono::milliseconds(200)) != std::future_status::ready) {
            std::cout << "[Main Thread] running\n";
        }

        ExecResult result = async_result.get();
    */
}
