//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Tools, Log
//-----------------------------------------------------------------------------
#pragma once
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

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
    // time in UTC
    inline double time() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration<double>(duration).count();
    }

    // -------------------------------------------------------------------------
    // inline double time_tz(std::string_view tz_name = "") {
    //     auto now = std::chrono::system_clock::now();
    //
    //     if (!tz_name.empty()) {
    //         std::chrono::zoned_time zt{tz_name, now};
    //         auto local_duration = zt.get_local_time().time_since_epoch();
    //         return std::chrono::duration<double>(local_duration).count();
    //     }
    //
    //     auto duration = now.time_since_epoch();
    //     return std::chrono::duration<double>(duration).count();
    // }

    // -------------------------------------------------------------------------
    inline std::string date(std::string format, double timestamp = -1.0, bool is_utc = false) {
        std::time_t time_raw;

        if (timestamp < 0.0) {
            time_raw = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        } else {
            time_raw = static_cast<std::time_t>(timestamp);
        }

         std::tm tm_zone;
        if (is_utc) {
            #if defined(_MSC_VER)
            gmtime_s(&tm_zone, &time_raw);
            #else
            gmtime_r(&time_raw, &tm_zone);
            #endif
        } else {
            #if defined(_MSC_VER)
            localtime_s(&tm_zone, &time_raw);
            #else
            localtime_r(&time_raw, &tm_zone);
            #endif
        }

        std::string c_format = "";
        for (size_t i = 0; i < format.length(); ++i) {
            if (format[i] == '%' && i + 1 < format.length()) {
                c_format += format[i];
                c_format += format[i+1];
                i++;
                continue;
            }

            switch (format[i]) {
                case 'Y': c_format += "%Y"; break; // Year 2026
                case 'm': c_format += "%m"; break; // month (01-12)
                case 'd': c_format += "%d"; break; // day (01-31)
                case 'H': c_format += "%H"; break; // hour (00-23)
                case 'i': c_format += "%M"; break; // minute (00-59)
                case 's': c_format += "%S"; break; // second (00-59)
                default:  c_format += format[i]; break; //others
            }
        }

        std::ostringstream oss;
        oss << std::put_time(&tm_zone, c_format.c_str());
        return oss.str();
    }
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
