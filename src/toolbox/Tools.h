//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Tools, Log
//-----------------------------------------------------------------------------
#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

namespace DreiZehn::Tools{

    // ------------------------------------------------------------------
    enum class LogLevel {
        normal = 0
        ,error
        ,warning
        ,info
        ,debug
    };

    typedef void(*LogCallback)(LogLevel level, const char* message);

    static inline void DefaultLogger(LogLevel level, const char *message) {
        switch (level) {
            case LogLevel::warning: printf("[warn] %s",  message); break;
            case LogLevel::error:   printf("[error] %s",  message); break;
            case LogLevel::info:    printf("[info] %s",  message); break;
            default:                printf("%s",  message); break;
        }
    }

    inline LogCallback LogHandler = DefaultLogger;

    static void _printf(LogLevel level, const char* fmt, va_list argptr)
    {
        if (!LogHandler) return;
        char buffer[8192] = {};
        uint32_t offset = 0;
        vsnprintf(buffer + offset, sizeof(buffer) - offset, fmt, argptr);

        LogHandler(level,buffer);

    }

    // ------------------------------------------------------------------
    // TODO should be redirectable
    // ==> int len = vsnprintf(buffer, bufferSize, format, arglist);
    //      => buffer to callback
    inline void printf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        _printf(LogLevel::normal, format, args);
        va_end(args);
    }
    inline void warnf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        _printf(LogLevel::warning,format, args);
        va_end(args);
    }
    inline void infof(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        _printf(LogLevel::info,format, args);
        va_end(args);
    }
    inline void errorf(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        _printf(LogLevel::error,format, args);
        va_end(args);
    }

    inline void printSeparator(int length) {
        const char* dashes = "----------------------------------------------------------------------------------------------------";
        if (length > 100) length = 100;
        if (length < 0) length = 0;

        printf("%.*s\n", length, dashes);
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
