#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>

namespace StringUtil
{
    inline std::string toLower(std::string s)
    {
        std::transform(
            s.begin(),
            s.end(),
            s.begin(),
            [](unsigned char c) {return std::tolower(c); }
        );
        return s;
    }

    inline std::string trim(const std::string& s)
    {
        auto start = std::find_if_not(s.begin(), s.end(),
            [](unsigned char ch) { return std::isspace(ch); });

        auto end = std::find_if_not(s.rbegin(), s.rend(),
            [](unsigned char ch) { return std::isspace(ch); }).base();

        if (start >= end)
            return "";

        return std::string(start, end);
    }

    // TODO: reserve some vector space, most titles and names are a few words long...
    inline std::vector<std::string> splitWords(const std::string& s)
    {
        std::stringstream ss(s);
        std::vector<std::string> words;
        words.reserve(6);

        std::string word;

        while (ss >> word)
            words.emplace_back(word);

        return words;
    }
}