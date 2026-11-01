//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Parts of formatString are borrowed from ElfScript/TorqueScript:
//      Copyright (c) 2012 GarageGames, LLC
//      Copyright (c) 2026 Thomas Hühn (XXTH)
//      SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String Functions and Object
//-----------------------------------------------------------------------------
#include <string.h>
#include <span>
#include <algorithm>
#include "core/Value.h"
#include "core/FunctionMap.h"

namespace DLS {

    Value EmptyStringValue = Value("");

    //-----------------------------------------------------------------------------
    namespace FluxStr {
        std::string toLower(std::string s);
        static constexpr std::string_view whitespaces = " \t\r\n";

        void ltrim(std::string& s){
            s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](unsigned char ch) {
                return std::isspace(ch);
            }));
        }

        std::string addTrailingSlash(std::string path) {
            if (path.empty()) return "/";

            char lastChar = path.back();
            if (lastChar == '/' || lastChar == '\\') {
                return path;
            }
            return path + "/";
        }

        std::string sanitizeFilenameWithUnderScores(std::string name)
        {
            std::string result;
            for (unsigned char c : name) {
                if (std::isalnum(c)) {
                    result += c;
                } else if (std::isspace(c)) {
                    result += '_';
                }
                // Special characters (like '.') are ignored/dropped here
            }
            return result;
        }

        std::string extractFileExt(std::string fileName, bool lowerCase){
            std::string ext = fileName.substr(fileName.find_last_of(".") + 1);
            if (lowerCase) ext = toLower(ext);
            return ext;
        }

        std::string_view extractFilename(std::string_view path){
            if (path.empty())
                return "";
            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash == std::string_view::npos) {
                return path;
            }
            return path.substr(lastSlash + 1);
        }

        std::vector< std::string_view > Tokenize(std::string_view str, const char delimiter) {
            std::vector<std::string_view> tokens;
            size_t start = 0;
            size_t end = str.find(delimiter);

            while (end != std::string_view::npos) {
                if (end > start) { // Skip empty tokens (multiple spaces)
                    tokens.push_back(str.substr(start, end - start));
                }
                start = end + 1;
                end = str.find(delimiter, start);
            }

            // Add the final token
            if (start < str.size()) {
                tokens.push_back(str.substr(start));
            }

            return tokens;
        }

        std::string getWord(std::string_view str, int no, const char delimiter) {
            int count = 0;
            size_t start = str.find_first_not_of(delimiter);

            while (start != std::string_view::npos) {
                size_t end = str.find(delimiter, start);

                if (count == no) {
                    // Found the index; return as a new std::string
                    return std::string(str.substr(start, end - start));
                }

                count++;
                start = str.find_first_not_of(delimiter, end);
            }
            return "";
        }

        std::string removeWord(std::string text, int index, const char delimiter) {
            int count = 0;
            size_t start = text.find_first_not_of(delimiter);

            while (start != std::string::npos) {
                size_t nextSpace = text.find(delimiter, start);
                size_t nextWord = text.find_first_not_of(delimiter, nextSpace);

                if (count == index) {
                    // Remove from start of this word to start of next word (to clean up spaces)
                    size_t length = (nextWord == std::string::npos) ? text.size() - start : nextWord - start;
                    text.erase(start, length);
                    return text;
                }
                count++;
                start = nextWord;
            }
            return text;
        }

        std::string setWord(std::string text, int index, std::string_view replace, const char delimiter) {
            int count = 0;
            size_t start = text.find_first_not_of(delimiter);

            while (start != std::string::npos) {
                size_t end = text.find(delimiter, start);
                if (count == index) {
                    // Replace the segment [start, end) with replace string
                    text.replace(start, (end == std::string::npos ? text.size() : end) - start, replace);
                    return text;
                }
                count++;
                start = text.find_first_not_of(delimiter, end);
            }
            return text; // Return original if index not found
        }

        std::string getWords(std::string_view text, int index, int endIndex, const char delimiter) {
            size_t startPos = std::string_view::npos;
            size_t endPos = std::string_view::npos;
            int count = 0;

            size_t cur = text.find_first_not_of(delimiter);
            while (cur != std::string_view::npos) {
                if (count == index) startPos = cur;

                size_t nextSpace = text.find(delimiter, cur);
                if (count == endIndex) {
                    endPos = nextSpace;
                    break;
                }

                count++;
                cur = text.find_first_not_of(delimiter, nextSpace);
                if (cur == std::string_view::npos) endPos = text.size(); // End of string
            }

            if (startPos == std::string_view::npos) return "";
            if (endPos == std::string_view::npos) endPos = text.size();

            return std::string(text.substr(startPos, endPos - startPos));
        }

        int getWordCount(std::string_view str, const char delimiter) {
            int count = 0;
            size_t pos = str.find_first_not_of(delimiter);

            while (pos != std::string_view::npos) {
                count++;
                pos = str.find(delimiter, pos);             // Find next space
                pos = str.find_first_not_of(delimiter, pos); // Skip extra spaces
            }
            return count;
        }

        std::string toUpper(std::string s){
            std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::toupper(c); });
            return s;
        }

        void toLowerInline(std::string& s){
            std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::tolower(c); });
        }

        void toUpperInline(std::string& s){
            std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::toupper(c); });
        }

        std::string removePart(std::string s, const std::string& part){
            if (part.empty()) return s;
            size_t pos = 0;
            while ((pos = s.find(part, pos)) != std::string::npos) {
                s.erase(pos, part.length());
            }
            return s;
        }
        void rtrim(std::string& s){
            s.erase(std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) {
                return std::isspace(ch);
            }).base(), s.end());
        }

        void trim(std::string& s){
            ltrim(s);
            rtrim(s);
        }

        std::string truncate(std::string str, size_t width, bool addPoints){
            if (addPoints && width < 4) return str;
            if (str.length() > width) {
                if (addPoints) return str.substr(0, width - 3) + "...";
                else return str.substr(0, width);
            }
            return str;
        }

        std::string trim(std::string_view str, bool bothEnds) {
            size_t start = bothEnds ? str.find_first_not_of(whitespaces) : 0;
            if (start == std::string_view::npos) return "";

            size_t end = str.find_last_not_of(whitespaces);

            return std::string(str.substr(start, end - start + 1));
        }



        std::string toLower(std::string s){
            std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::tolower(c); });
            return s;
        }
    }

    //-----------------------------------------------------------------------------

    bool ArgsCheckString( const char* command,std::span<Value> args, uint32_t count) {
        if (args.size() < count ) {
            Tools::errorf("Usage: %s  [string value] %d times",command, count);
            return false;
        }

        for (int i = 0; i < count; i++) {
            if (!args[i].isString() ) {
                Tools::errorf("Usage: %s  [string value] %d times",command, count);
                return false;
            }
        }

        return true;
    }
    // =============================================================================
    // --- formatString ---
    // =============================================================================
    std::string formatString(std::span<Value> args) {

        if (args.size() < 1) return "";

        const char* fmt = args[0].getString().c_str();
        int currentArgIndex = 1; // First arg at 1

        std::string result = "";

        while (*fmt)
        {
            if (*fmt == '%')
            {
                if (*(fmt + 1) == '\0')
                {
                    result += "%";
                    break;
                }

                const char* specStart = fmt;
                fmt++;

                // We have a double %%, it's just an escaped percent sign
                if (*fmt == '%')
                {
                    result += '%';
                    fmt++;
                    continue;
                }

                if (currentArgIndex >= args.size())
                {
                    Tools::errorf("formatString: invalid argument count. current index: %d, args count: %zu at: %s!!\n",
                                  currentArgIndex, args.size(), fmt);
                    break;
                }

                Value& curVal = args[currentArgIndex];
                currentArgIndex++;

                // skip flags
                while (*fmt != '\0' && !isalpha(*fmt))
                {
                    fmt++;
                }

                // long/long flags check
                bool isLongLong = false;
                bool isLong = false;
                bool isLongDouble = false;

                // modifier flags
                while (*fmt == 'l' || *fmt == 'h' || *fmt == 'z' || *fmt == 'L')
                {
                    if (*fmt == 'l')
                    {
                        if (isLong) { isLongLong = true; isLong = false; }
                        else { isLong = true; }
                    }
                    else if (*fmt == 'L')
                    {
                        isLongDouble = true;
                    }
                    fmt++;
                }

                // null check
                if (*fmt == '\0')
                {
                    Tools::errorf("formatString: invalid format specifier!!");
                    break;
                }

                char tokenBuffer[512];
                tokenBuffer[0] = '\0';

                // calc length
                uint32_t specLen = (fmt - specStart) + 1;
                char specBuffer[32];
                if (specLen > 31)
                {
                    Tools::errorf("formatString: format specifier too long!");
                    break;
                }

                strncpy(specBuffer, specStart, specLen);
                specBuffer[specLen] = '\0';

                // here is the beaf
                switch (*fmt)
                {
                    case 's': // String
                        snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, curVal.getString().c_str());
                        break;

                    case 'c':
                    case 'C':
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X':
                    {
                        if (isLongLong)
                        {
                            if (*fmt == 'u' || *fmt == 'x' || *fmt == 'X')
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (unsigned long long)curVal.getUInt());
                            else
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (long long)curVal.getInt());
                        }
                        else if (isLong)
                        {
                            if (*fmt == 'u' || *fmt == 'x' || *fmt == 'X')
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (unsigned long)curVal.getUInt());
                            else
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (long)curVal.getInt());
                        }
                        else //  32-Bit Integer
                        {
                            if (*fmt == 'u' || *fmt == 'x' || *fmt == 'X')
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (unsigned int)curVal.getUInt());
                            else
                                snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (int)curVal.getInt());
                        }
                        break;
                    }

                    case 'e':
                    case 'E':
                    case 'f':
                    case 'g':
                    case 'G':
                    {
                        // sprintf erwartet für %f/%e/%g standardmäßig double (64-Bit Fließkomma)
                        if (isLongDouble)
                            snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (long double)curVal.getDouble());
                        else
                            snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, curVal.getDouble());
                        break;
                    }

                    case 'p':
                        snprintf(tokenBuffer, sizeof(tokenBuffer), specBuffer, (void*)curVal.getPointer());
                        break;

                    default:
                        Tools::errorf("formatString: unknown type: '%c'!", *fmt);
                        tokenBuffer[0] = '\0';
                        break;
                }

                result += tokenBuffer;
            }
            else
            {
                result += *fmt;
            }

            fmt++;
        }

        return result;
    }

    // =============================================================================
    // --- RegisterStringFunctions ---
    // =============================================================================

    void RegisterStringFunctions( ) {
        static bool registered = false;
        if (registered) return;
        registered = true;

        using namespace FunctionMap;

        // ---------------------------------------------------------------------
        // StringObject::RegisterSymbols();
        // RegisterFunction("String::new", [](std::span<Value> args, Value& ret) -> bool {
        //     if (args.size() != 1 || !args[0].isString()) {
        //         Tools::errorf("Usage: String::new string\n");
        //         return false;
        //     }
        //     StringObject* s = new StringObject(args[0].getString());
        //     ret = Value(s);
        //     return true;
        // });
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        RegisterFunction("str::concat", [](std::span<Value> args, Value& ret) -> bool {
            std::string resultStr = "";

            for ( auto& val : args) {
                resultStr += val.toString();
            }

            ret = Value(resultStr);
            return true;
        }, "Retrun a concat string of values. The values can be string, int or float.");

        // ---------------------------------------------------------------------
        RegisterFunction("str::format", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() < 1) {
                ret = EmptyStringValue;
                return true;
            }
            ret = Value( formatString(args));
            return true;
        }, "Return a formated string. Works like sprintf. first parameter is the format next are the values.");

        // ---------------------------------------------------------------------
        RegisterFunction("str::cast", [](std::span<Value> args, Value& ret) -> bool {

            if (args.size() < 1 || args[0].isPointer()) {
                ret = EmptyStringValue;
                return true;
            }
            if (args[0].isLong()) ret = Value (std::to_string(args[0].getLong()));
            else
            if (args[0].isDouble()) ret = Value (std::to_string(args[0].getDouble()));

            return true;
        }, "return a converted  number to string and cast the variable to string type.");

        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        RegisterFunction("str::toNumber", [](std::span<Value> args, Value& ret) -> bool {
            if (!ArgsCheckString("str::toNumber", args, 1)) return true;
            char* endptr = nullptr;
            std::string s = args[0].getString();
            double resDouble = std::strtod(s.c_str(), &endptr);
            if (s.empty() || *endptr != '\0') {
                ret = Value(0.0);
            } else {
                ret = Value(resDouble);
            }
            return true;
        }, "return the number representation of a string.");
        // ---------------------------------------------------------------------
        RegisterFunction("str::strlen", [](std::span<Value> args, Value& ret) -> bool {
            if (!ArgsCheckString("str::len", args, 1)) return true;
            ret = Value((int32_t)args[0].getString().length());
            return true;
        } , "return the len of the given string.");
        // ---------------------------------------------------------------------
        RegisterFunction("str::strpos", [](std::span<Value> args, Value& ret) -> bool {
            if (!ArgsCheckString("str::strpos", args, 2)) return true;
            const char* str = args[0].getString().c_str();
            const char* pos = strstr(str, args[1].getString().c_str());
            if (!pos) {
                ret = Value(-1);
                return true;
            }
            ret = Value((int32_t)(pos-str));
            return true;
        }, "return the position of the string value2 inside value1. Returns -1 if not found.");
        // ---------------------------------------------------------------------
        RegisterFunction("str::ltrim", [](std::span<Value> args, Value& ret) -> bool {
            if (!ArgsCheckString("str::ltrim", args, 1)) return true;
            std::string str = args[0].getString();
            FluxStr::ltrim(str);
            ret = Value(str);
            return true;
        }, "return left side trimmed string");
        // ---------------------------------------------------------------------
        RegisterFunction("str::rtrim", [](std::span<Value> args, Value& ret) -> bool {
            if (!ArgsCheckString("str::ltrim", args, 1)) return true;
            std::string str = args[0].getString();
            FluxStr::rtrim(str);
            ret = Value(str);
            return true;
        }, "return right side trimmed string");
        // ---------------------------------------------------------------------
        RegisterFunction("str::trim", [](std::span<Value> args, Value& ret) -> bool {
            if (!ArgsCheckString("str::ltrim", args, 1)) return true;
            std::string str = args[0].getString();
            FluxStr::trim(str);
            ret = Value(str);
            return true;
        }, "return both sides trimmed string");
        // ---------------------------------------------------------------------
        RegisterFunction("str::toLower", [](std::span<Value> args, Value& ret) -> bool {
            if (!ArgsCheckString("str::toLower", args, 1)) return true;
            ret = Value(FluxStr::toLower(args[0].getString()));
            return true;
        }, "return converted to lower string");
        // ---------------------------------------------------------------------
        RegisterFunction("str::toUpper", [](std::span<Value> args, Value& ret) -> bool {
            if (!ArgsCheckString("str::toUpper", args, 1)) return true;
            ret = Value(FluxStr::toUpper(args[0].getString()));
            return true;
        }, "return converted to upper string");
        // ---------------------------------------------------------------------
        RegisterFunction("str::ord", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() < 1 || !args[0].isString()) {
                Tools::errorf("Usage: str::ord string [index]\n");
                return true;
            }
            std::string str = args[0].getString();
            int offset = 0;
            if (args.size() > 1) offset = args[1].getInt();
            if (offset >= 0 && offset < str.length()) {
                ret = Value(static_cast<int32_t>(str[offset]));
            }
            return true;
        }, "return the oridinal value of a character given by string if not index is added it use 0");
        // ---------------------------------------------------------------------
        RegisterFunction("str::chr", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: str::chr ascii (0..255)\n");
                return true;
            }

            int ascii = args[0].getInt();

            if (ascii < 0 || ascii > 255) {
                Tools::errorf("str::chr error: ascii %d out of bounds (0..255)!\n", ascii);
                return true;
            }

            std::string result(1, static_cast<char>(ascii));

            ret = Value(result);
            return true;
        }, "Return the characted of the given a ascii number.");
        // ---------------------------------------------------------------------
        RegisterFunction("str::substr", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() < 2 || !args[0].isString()) {
                Tools::errorf("Usage: str::substr string start [count]\n");
                return true;
            }

            const std::string& str = args[0].getString();
            int len = static_cast<int>(str.length());

            int start = args[1].getInt();
            if (start < 0) {
                start = len + start;
                if (start < 0) start = 0;
            }

            if (start > len) {
                Tools::errorf("str::substr start out of bounds: %d! (String length is %d)\n", args[1].getInt(), len);
                return true;
            }

            int count = 0;
            if (args.size() > 2) {
                int input_count = args[2].getInt();
                if (input_count < 0) {
                    int end_pos = len + input_count;
                    count = end_pos - start;
                    if (count < 0) count = 0;
                } else {
                    count = input_count;
                }
            } else {
                count = len - start;
            }

            ret = Value(str.substr(static_cast<size_t>(start), static_cast<size_t>(count)));
            return true;
        }, "return a substring from start. Len is optional.");
        // ---------------------------------------------------------------------
        RegisterFunction("str::replace", [](std::span<Value> args, Value& ret) -> bool {
            if (args.size() < 3 || !args[0].isString() || !args[1].isString()
                || !args[2].isString()
            ) {
                Tools::errorf("Usage: str::replace str search repl\n");
                return true;
            }
            const std::string& str = args[0].getString();
            const std::string& search = args[1].getString();
            const std::string& repl = args[2].getString();

            if (search.empty()) {
                ret = Value(str);
                return true;
            }

            std::string result = str;
            size_t pos = 0;

            while ((pos = result.find(search, pos)) != std::string::npos) {
                result.replace(pos, search.length(), repl);
                pos += repl.length();
            }

            ret = Value(result);
            return true;
        }, "Replace a part of a string");
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        RegisterFunction("str::wordcount", [](std::span<Value> args, Value& ret) -> bool {
             if (args.size() < 1 || !args[0].isString()) {

                Tools::errorf("Usage: str::wordcount str [separator]\n");
                return true;
            }
            std::string_view str = args[0].getString();
            if (str.empty()) {
                ret = Value(0.0);
                return true;
            }

            char delim = ' ';
            if (args.size() > 1 ) {
                delim = args[1].toString()[0];
            }

            ret =  Value(FluxStr::getWordCount(str,delim));

            return true;
        }, "Get the count of Words - optional separator char");
        // ---------------------------------------------------------------------
        RegisterFunction("str::word", [](std::span<Value> args, Value& ret) -> bool {
             if (args.size() < 2 || !args[0].isString()) {

                Tools::errorf("Usage: str::word str index [separator]\n");
                return true;
            }
            std::string_view str = args[0].getString();
            if (str.empty()) {
                ret = EmptyStringValue;
                return true;
            }

            uint32_t index = args[1].getUInt();

            char delim = ' ';
            if (args.size() > 2 ) {
                delim = args[2].toString()[0];
            }

            ret =  Value(FluxStr::getWord(str,index,delim));

            return true;
        }, "Get a word at index - optional separator char");
        // ---------------------------------------------------------------------
        RegisterFunction("str::setword", [](std::span<Value> args, Value& ret) -> bool {
             if (args.size() < 3 || !args[0].isString()) {

                Tools::errorf("Usage: str::setword str index newword [separator]\n");
                return true;
            }
            std::string str = args[0].getString();
            if (str.empty()) {
                ret = EmptyStringValue;
                return true;
            }

            uint32_t index = args[1].getUInt();
            std::string replace = args[2].toString();

            char delim = ' ';
            if (args.size() > 3 ) {
                delim = args[3].toString()[0];
            }

            ret =  Value(FluxStr::setWord(str,index,replace, delim));

            return true;
        }, "Set a words at index - optional separator char");
        // ---------------------------------------------------------------------
        RegisterFunction("str::removeword", [](std::span<Value> args, Value& ret) -> bool {
             if (args.size() < 2 || !args[0].isString()) {

                Tools::errorf("Usage: str::removeword str index [separator]\n");
                return true;
            }
            std::string str = args[0].getString();
            if (str.empty()) {
                ret = EmptyStringValue;
                return true;
            }

            uint32_t index = args[1].getUInt();

            char delim = ' ';
            if (args.size() > 2 ) {
                delim = args[2].toString()[0];
            }

            ret =  Value(FluxStr::removeWord(str,index,delim));

            return true;
        }, "Remove a words at index - optional separator char");
        // ---------------------------------------------------------------------



    } //RegisterStringFunctions

} //namespace
