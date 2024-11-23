#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <ranges>
#include <chrono>
#include <format>
#include <cctype>

#include <cstdio>
#include <memory>


namespace utillib
{
    using namespace std;

    // Разделяет строку str разделителем delim
    vector<string> Split(const string &str, const string &delim)
    {
        vector<string> split;
        for (const auto &s : views::split(str, delim))
        {
            split.emplace_back(s.begin(), s.end());
        }
        return split;
    }

    // Убирает по краям строки любые символы, определённые функцией сравнения. По умолчанию isspace (пробельные, в т.ч. \n)
    string Trim(const string &in, int (*compare_func)(int) = isspace)
    {
        auto sv = in |
                  views::drop_while(compare_func) |
                  views::reverse |
                  views::drop_while(compare_func) |
                  views::reverse;
        return string(sv.begin(), sv.end());
    }

    // Возвращает строку, содержащую текст файла. По умолчанию режим открытия binary
    string ReadFile(const string &file_path, std::ios::openmode open_mode = std::ios::binary)
    {
        ifstream file(file_path, open_mode);
        stringstream str;
        str << file.rdbuf() << "\n";
        return str.str();
    }

    // Строка с текущей датой и временем
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

    string GetTimeFromSec(int64_t unix_sec)
    {
        time_t t = time_t( unix_sec );
        tm *st = localtime(&t);

        return std::format("{:0>4}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}",
                           st->tm_year + 1900,
                           st->tm_mon + 1,
                           st->tm_mday,
                           st->tm_hour,
                           st->tm_min,
                           st->tm_sec);
    }

    // UNIX time now в секундах
    int64_t GetUNIXTimeNow()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    // Выполняет команду в командной строке и возвращает ответ команды либо "ERROR"
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
}