#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <ranges>
#include <string_view>
#include <chrono>
#include <format>
#include <cctype>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>

namespace utillib
{
    using namespace std;

    vector<string> Split(const string &str, const string &delim)
    {
        vector<string> split;
        for (const auto &s : views::split(str, delim))
        {
            split.emplace_back(string_view(s));
        }
        return split;
    }

    string Trim(const string &in, int (*compare_func)(int) = isspace)
    {
        auto sv = in |
                  views::drop_while(compare_func) |
                  views::reverse |
                  views::drop_while(compare_func) |
                  views::reverse;
        return string(sv.begin(), sv.end());
    }

    string ReadFile(const string &file_path, std::ios::openmode open_mode = std::ios::binary)
    {
        ifstream file(file_path, open_mode);
        stringstream str;
        str << file.rdbuf() << "\n";
        return str.str();
    }

    string GetTime()
    {
        auto now = std::chrono::system_clock::now();

        time_t t = std::chrono::system_clock::to_time_t(now);
        tm *st = localtime(&t);

        return std::format("{:0>4}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}",
                           st->tm_year + 1900,
                           st->tm_mon + 1,
                           st->tm_mday,
                           st->tm_hour,
                           st->tm_min,
                           st->tm_sec);
    }

#ifndef WIN32
    std::string Exec(const char* cmd) {
        std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            return "ERROR";
        }
        char buffer[128];
        std::string result;
        while (!feof(pipe.get())) {
            if (fgets(buffer, sizeof(buffer), pipe.get()) != NULL)
                result += buffer;
        }
        return result;
    }
#endif
}