#include "server.hpp"
#include "serial.hpp"
#include "utility.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <thread>

namespace fs = std::filesystem;

struct ParsedData
{
    double temp = 0;
    bool is_error = false;
};

std::string DEFAULT_PORT = "COM4";

std::string LOG_DIR = "logs";
std::string LOG_SEC_NAME = LOG_DIR + "/second.log";
std::string LOG_HOUR_NAME = LOG_DIR + "/hour.log";
std::string LOG_DAY_NAME = LOG_DIR + "/day.log";

constexpr int64_t HOUR_SEC = 10;
// constexpr int64_t HOUR_SEC = 60 * 60;
constexpr int64_t DAY_SEC = HOUR_SEC * 24;
constexpr int64_t MONTH_SEC = DAY_SEC * 30;
constexpr int64_t YEAR_SEC = DAY_SEC * 365;

struct TrackingData
{
    int64_t last_hour_gather = 0;
    int64_t last_day_gather = 0;
};

int64_t GetUNIXTimeNow()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::vector<std::string> SplitString(const std::string str, const char delimeter = ' ')
{
    std::vector<std::string> split;
    std::istringstream iss(str);
    std::string s;
    while (getline(iss, s, ' '))
    {
        split.emplace_back(s);
    }
    return split;
}

std::string GetLastLine(std::ifstream &fi)
{
    fi.seekg(-1, std::ios_base::end);
    if (fi.peek() == '\n')
    {
        // Start searching for \n occurrences
        fi.seekg(-1, std::ios_base::cur);
        int i = fi.tellg();
        for (i; i > 0; i--)
        {
            if (fi.peek() == '\n')
            {
                // Found
                fi.get();
                break;
            }
            // Move one character back
            fi.seekg(i, std::ios_base::beg);
        }
    }
    std::string lastline;
    getline(fi, lastline);
    return lastline;
}

void CreateFileIfNotExists(const std::string name)
{
    if (!fs::exists(name))
    {
        std::ofstream(name).close();
    }
}

void SetTracking(const std::string name, int64_t &tracking)
{
    CreateFileIfNotExists(name);
    std::ifstream fi(name);
    fi.seekg(0, std::ios::end);
    // if (fi.tellg() < 10)
    // {
    tracking = GetUNIXTimeNow();
    // }
    // else
    // {
    //     auto last_line = GetLastLine(fi);
    //     auto split = SplitString(last_line); // 0 -> time, 1 -> temp
    //     tracking = std::stoll(split[0]);
    // }
    fi.close();
}

double GetMeanTemp(const std::string file_name, const int64_t &now, const int64_t &diff_sec)
{
    std::ifstream log(file_name);

    double mean = 0;
    uint32_t count = 1;

    std::string line;
    while (std::getline(log, line))
    {
        auto split = SplitString(line);
        if (split.size() <= 0)
        {
            continue;
        }

        int64_t time = std::stoll(split[0]);
        if (now - time < diff_sec)
        {
            mean += std::stod(split[1]);
            break;
        }
    }

    while (std::getline(log, line))
    {
        auto split = SplitString(line);
        mean += std::stod(split[1]);
        ++count;
    }

    return mean / count;
}

void WriteTempToFile(const std::string file_name, const double &temp, const int64_t &now, const int64_t &lim_sec)
{
    std::string temp_name = LOG_DIR + "/temp.log";
    if (fs::exists(temp_name))
    {
        fs::remove(temp_name);
    }

    fs::copy_file(file_name, temp_name);
    std::ofstream of_log(file_name);
    std::ifstream temp_log(temp_name);

    std::string line;
    while (std::getline(temp_log, line))
    {
        auto split = SplitString(line);
        if (split.size() <= 0)
        {
            continue;
        }

        int64_t time = std::stoll(split[0]);
        if (now - time < lim_sec)
        {
            break;
        }
    }

    if (!temp_log.eof())
    {
        of_log << line << '\n';
    }
    while (std::getline(temp_log, line))
    {
        of_log << line << '\n';
    }
    of_log << now << " " << temp << std::endl;

    of_log.close();
    temp_log.close();
    fs::remove(temp_name);
}

ParsedData ParseTemperature(const std::string str)
{
    auto len = str.size();

    int pos_f;
    int pos_r;

    for (int i = 0; i < len / 2 + 1; ++i)
    {
        if (str[i] != '$')
        {
            pos_f = i;
            break;
        }
    }

    for (int i = len - 1; i > len / 2 - 1; --i)
    {
        if (str[i] != '$')
        {
            pos_r = i;
            break;
        }
    }

    if (pos_f >= pos_r)
    {
        return {0, true};
    }

    auto str_temp = str.substr(pos_f, pos_r - pos_f);
    try
    {
        return {std::stod(str_temp), false};
    }
    catch (const std::invalid_argument &e)
    {
        return {0, true};
    }
}

void ServerThread();

int main(int argc, char **argv)
{
    std::thread server_thread(ServerThread);

    std::string port(DEFAULT_PORT);
    if (argc > 1)
    {
        port = argv[1];
    }

    splib::SerialPort serial_port(port, splib::SerialPort::BAUDRATE_115200);

    if (!serial_port.IsOpen())
    {
        std::cout << "Args: [1: port]; default: " << DEFAULT_PORT << std::endl;
        std::cerr << "Failed to open " << port << " port!" << std::endl;
        return -2;
    }
    std::cout << "Listening to port: " << port << std::endl;

    auto tracking_data = TrackingData();

    fs::create_directory(LOG_DIR);
    CreateFileIfNotExists(LOG_SEC_NAME);                        // every recieved
    SetTracking(LOG_HOUR_NAME, tracking_data.last_hour_gather); // mean every hour
    SetTracking(LOG_DAY_NAME, tracking_data.last_day_gather);   // mean every day

    std::string input;
    serial_port.SetTimeout(1.0);

    // format: $$temp$$
    while (true)
    {
        serial_port >> input;
        auto parsed = ParseTemperature(input);
        if (parsed.is_error)
        {
            std::cerr << "Parsing error " << utillib::GetTime() << " -- Recieved: '" << input << "'" << std::endl;
            continue;
        }
        auto now_time = GetUNIXTimeNow();

        WriteTempToFile(LOG_SEC_NAME, parsed.temp, now_time, DAY_SEC);

        // std::cout << now_time - tracking_data.last_hour_gather << std::endl;
        if (now_time - tracking_data.last_hour_gather >= HOUR_SEC)
        {
            // get hour mean
            double hour_mean = GetMeanTemp(LOG_SEC_NAME, now_time, HOUR_SEC);
            WriteTempToFile(LOG_HOUR_NAME, hour_mean, now_time, MONTH_SEC);
            tracking_data.last_hour_gather = now_time;
        }
        if (now_time - tracking_data.last_day_gather >= DAY_SEC)
        {
            // get day mean
            double day_mean = GetMeanTemp(LOG_HOUR_NAME, now_time, DAY_SEC);
            WriteTempToFile(LOG_DAY_NAME, day_mean, now_time, YEAR_SEC);
            tracking_data.last_day_gather = now_time;
        }
    }
}

void ServerThread()
{
    srvlib::HTTPServer server;

    server.Listen("0.0.0.0", 80);
    std::vector<srvlib::SpecialResponse> resps;

    auto log_sec = [&]()
    { return utillib::ReadFile(LOG_SEC_NAME); };
    auto log_hour = [&]()
    { return utillib::ReadFile(LOG_HOUR_NAME); };
    auto log_day = [&]()
    { return utillib::ReadFile(LOG_DAY_NAME); };

    resps.emplace_back("GET", "/sec/raw", log_sec, true);
    resps.emplace_back("GET", "/hour/raw", log_hour, true);
    resps.emplace_back("GET", "/day/raw", log_day, true);

    server.RegisterResponses(resps);
    while (true)
    {
        if (!server.IsValid())
        {
            std::cerr << "Server error" << std::endl;
            break;
        }
        server.ProcessClient();
    }
}