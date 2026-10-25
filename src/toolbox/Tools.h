//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Tools, Log
//-----------------------------------------------------------------------------
#pragma once
#include <string>



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

    void DefaultLogger(LogLevel level, const char *message);

    inline  LogCallback LogHandler = DefaultLogger;
    // ------------------------------------------------------------------
     void printf(const char *format, ...);
     void warnf(const char *format, ...);
     void infof(const char *format, ...);
     void errorf(const char *format, ...);

     void printSeparator(int length);


    // -------------------------------------------------------------------------
     void sleep(unsigned int ms);

    // -------------------------------------------------------------------------
    // time in UTC
     double time();


    // -------------------------------------------------------------------------
     std::string date(std::string format, double timestamp = -1.0, bool is_utc = false);
    // -------------------------------------------------------------------------
     bool begins_with(const std::string& text, const std::string& prefix);
     bool ends_with(const std::string& text, const std::string& suffix);

}
