//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String Helpers
// Subset  of Ohmflux FluxStr + some new
//-----------------------------------------------------------------------------
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace FluxStr {
    //--------------------------------------------------------------------------
    static constexpr std::string_view whitespaces = " \t\r\n";
    //--------------------------------------------------------------------------
    // NEW TODO copy to Ohmflux
    // -------------------------------------------------------------------------
    // Left trim: removes leading whitespace
    void ltrim(std::string &s);

    // Right trim: removes trailing whitespace
    void rtrim(std::string &s);

    // Full trim: removes both leading and trailing whitespace
    void trim(std::string &s);

    // -------------------------------------------------------------------------
    std::string trim(std::string_view str, bool bothEnds = true);
    std::string truncate(std::string str, size_t width, bool addPoints = true);
    std::string removePart(std::string s, const std::string& part);
    void toUpperInline(std::string& s);
    void toLowerInline(std::string& s);
    std::string toUpper(std::string s);
    std::string toLower(std::string s);
    //--------------------------------------------------------------------------
    // Returns the number of words separated by spaces
    int getWordCount(std::string_view str, const char delimiter = ' ');
    //--------------------------------------------------------------------------
    // Helper to find the start/end bounds of a word index
    std::string getWords(std::string_view text, int index, int endIndex = 999999,  const char delimiter = ' ');
    std::string setWord(std::string text, int index, std::string_view replace ,const  char delimiter = ' ');
    std::string removeWord(std::string text, int index , const char delimiter = ' ');
    //--------------------------------------------------------------------------
    // Returns the word at index 'no' (0-based). Returns empty string if not found.
    std::string getWord(std::string_view str, int no , const char delimiter = ' ');

    //--------------------------------------------------------------------------
    std::vector<std::string_view> Tokenize(std::string_view str, const char delimiter = ' ');

    //--------------------------------------------------------------------------
    // ----- filename tools but string only -----
    //--------------------------------------------------------------------------
    std::string_view extractFilename(std::string_view path);
    std::string extractFileExt(std::string fileName, bool lowerCase = true);
    std::string sanitizeFilenameWithUnderScores(std::string name);
    std::string addTrailingSlash(std::string path);
    //--------------------------------------------------------------------------


} //namespace
