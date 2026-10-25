//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Tools, Log
//-----------------------------------------------------------------------------
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

#include "Tools.h"


namespace DreiZehn::Tools{


    void DefaultLogger(LogLevel level, const char* message){
        switch (level) {
            case LogLevel::warning: ::printf("[warn] %s",  message); break;
            case LogLevel::error:   ::printf("[error] %s",  message); break;
            case LogLevel::info:    ::printf("[info] %s",  message); break;
            default:                ::printf("%s",  message); break;
        }
    }

    void raw_printf(LogLevel level, const char* fmt, va_list argptr)
    {
        if (!LogHandler) return;
        char buffer[8192] = {};
        uint32_t offset = 0;
        vsnprintf(buffer + offset, sizeof(buffer) - offset, fmt, argptr);

        LogHandler(level,buffer);

    }

    void printf(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        raw_printf(LogLevel::normal, format, args);
        va_end(args);
    }

    void warnf(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        raw_printf(LogLevel::warning,format, args);
        va_end(args);
    }

    void infof(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        raw_printf(LogLevel::info,format, args);
        va_end(args);
    }

    void errorf(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        raw_printf(LogLevel::error,format, args);
        va_end(args);
    }

    void printSeparator(int length) {
        const char* dashes = "----------------------------------------------------------------------------------------------------";
        if (length > 100) length = 100;
        if (length < 0) length = 0;

        printf("%.*s\n", length, dashes);
    }

    void sleep(unsigned int ms) ;

    void sleep(unsigned int ms){
        #if defined(_WIN32)
        ::Sleep(ms);
        #else
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
        #endif
    }

    double time(){
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration<double>(duration).count();
    }

    std::string date(std::string format, double timestamp, bool is_utc) {
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

    bool begins_with(const std::string& text, const std::string& prefix)
    {
        return text.size() >= prefix.size() &&
        text.compare(0, prefix.size(), prefix
        ) == 0;
    }

    bool ends_with(const std::string& text, const std::string& suffix)
    {
        return text.size() >= suffix.size() &&
        text.compare(
            text.size() - suffix.size(),
                     suffix.size(),
                     suffix
        ) == 0;
    }
} //namespace
