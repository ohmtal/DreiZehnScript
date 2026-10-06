//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Tools, Log
//-----------------------------------------------------------------------------
#pragma once
#include <stdio.h>
#include <stdarg.h>

namespace DreiZehn::Tools{

    // ------------------------------------------------------------------
    // TODO should be redirectable
    inline void printf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    inline void errorf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
    // -------------------------------------------------------------------------
    #if defined(_WIN32) || defined(__WIN32__) || defined(MSC_VER)
    #include <windows.h>
    #define platform_sleep(ms) Sleep(ms)
    #elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #include <time.h>
    inline void sleep(unsigned int ms) {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }
    #else
    #error "UNKNOWN OS"
    #endif
    // -------------------------------------------------------------------------
    inline bool begins_with(const std::string& text, const std::string& prefix)
    {
        return text.size() >= prefix.size() &&
               text.compare(0, prefix.size(), prefix
        ) == 0;
    }
    // -------------------------------------------------------------------------
    inline bool ends_with(const std::string& text, const std::string& suffix)
    {
        return text.size() >= suffix.size() &&
        text.compare(
            text.size() - suffix.size(),
                     suffix.size(),
                     suffix
        ) == 0;
    }

}
