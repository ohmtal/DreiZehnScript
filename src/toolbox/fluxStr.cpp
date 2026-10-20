//-----------------------------------------------------------------------------
// Copyright (c) 2012/2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String Helpers
// Subset  of Ohmflux FluxStr + some new
//-----------------------------------------------------------------------------
#include <string>
#include <string_view>
#include <algorithm>
#include <cstring>
#include "fluxStr.h"

namespace FluxStr {
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

    void rtrim(std::string& s){
        s.erase(std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) {
            return std::isspace(ch);
        }).base(), s.end());
    }

    std::string toLower(std::string s){
        std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    }
}
