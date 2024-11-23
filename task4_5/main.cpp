#include "server.hpp"
#include "serial.hpp"
#include "utility.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <thread>
#include <atomic>

namespace fs = std::filesystem;

struct ParsedData
{
    double temp = 0;
    bool is_error = false;
};

std::string LOG_DIR = "logs";
std::string LOG_SEC_NAME = LOG_DIR + "/second.log";
std::string LOG_HOUR_NAME = LOG_DIR + "/hour.log";
std::string LOG_DAY_NAME = LOG_DIR + "/day.log";

// constexpr int64_t HOUR_SEC = 10; // speed up process
constexpr int64_t HOUR_SEC = 60 * 60;
constexpr int64_t DAY_SEC = HOUR_SEC * 24;
constexpr int64_t MONTH_SEC = DAY_SEC * 30;
constexpr int64_t YEAR_SEC = DAY_SEC * 365;

void CreateFileIfNotExists(const std::string name)
{
    if (!fs::exists(name))
    {
        std::ofstream(name).close();
    }
}

double GetMeanTemp(const std::string file_name, const int64_t &now, const int64_t &diff_sec)
{
    std::ifstream log(file_name);

    double mean = 0;
    uint32_t count = 1;

    std::string line;
    while (std::getline(log, line))
    {
        auto split = utillib::Split(line, " ");
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
        auto split = utillib::Split(line, " ");
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
        auto split = utillib::Split(line, " ");
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

// Парсинг температуры в формате $$temp$$, где могут быть не все знаки $
ParsedData ParseTemperature(const std::string &str)
{
    auto comp_dollar = [](int ch)
    {
        return (std::isspace(ch) != 0 || ch == '$') ? 1 : 0;
    };

    auto str_temp = utillib::Trim(str, comp_dollar);

    try
    {
        return {std::stod(str_temp), false};
    }
    catch (const std::invalid_argument &e)
    {
        return {0, true};
    }
}

std::string DEFAULT_SERIAL_PORT_NAME = "COM4";
std::string DEFAULT_SERVER_HOST = "127.0.0.1";
short int DEFAULT_SERVER_PORT = 8080;

std::atomic_int64_t last_hour_gather = 0;
std::atomic_int64_t last_day_gather = 0;

void SerialThread(const std::string &serial_port_name)
{
    splib::SerialPort serial_port(serial_port_name, splib::SerialPort::BAUDRATE_115200);

    if (!serial_port.IsOpen())
    {
        std::cout << "Args: [1: port]; default: " << DEFAULT_SERIAL_PORT_NAME << std::endl;
        std::cerr << "Failed to open " << serial_port_name << " port!" << std::endl;
        return;
    }

    std::cout << "Listening to serial port: " << serial_port_name << std::endl;

    fs::create_directory(LOG_DIR);
    CreateFileIfNotExists(LOG_SEC_NAME);
    CreateFileIfNotExists(LOG_HOUR_NAME);
    CreateFileIfNotExists(LOG_DAY_NAME);
    last_hour_gather = utillib::GetUNIXTimeNow();
    last_day_gather = utillib::GetUNIXTimeNow();

    std::string input;
    serial_port.SetTimeout(1.0);

    while (true)
    {
        // format: $$temp$$
        serial_port >> input;
        auto parsed = ParseTemperature(input);
        if (parsed.is_error)
        {
            // std::cerr << "Parsing error " << utillib::GetTime() << " -- Recieved: '" << input << "'" << std::endl;
            continue;
        }
        auto now_time = utillib::GetUNIXTimeNow();

        WriteTempToFile(LOG_SEC_NAME, parsed.temp, now_time, DAY_SEC);

        // std::cout << now_time - last_hour_gather << std::endl;
        if (now_time - last_hour_gather >= HOUR_SEC)
        {
            // get hour mean
            double hour_mean = GetMeanTemp(LOG_SEC_NAME, now_time, HOUR_SEC);
            WriteTempToFile(LOG_HOUR_NAME, hour_mean, now_time, MONTH_SEC);
            last_hour_gather = now_time;
        }
        if (now_time - last_day_gather >= DAY_SEC)
        {
            // get day mean
            double day_mean = GetMeanTemp(LOG_HOUR_NAME, now_time, DAY_SEC);
            WriteTempToFile(LOG_DAY_NAME, day_mean, now_time, YEAR_SEC);
            last_day_gather = now_time;
        }
    }
}

void ServerThread(const std::string &host_ip, const short int port)
{
    srvlib::HTTPServer server(host_ip, port);
    if (!server.IsValid())
    {
        std::cerr << "Args: [2: Host ip]; default: " << DEFAULT_SERVER_HOST << std::endl;
        std::cerr << "Args: [3: Host port]; default: " << DEFAULT_SERVER_PORT << std::endl;
        std::cerr << "Server error" << std::endl;
        return;
    }

    std::cout << "Server listening to: http://" << host_ip << ":" << port << std::endl;

    std::vector<srvlib::SpecialResponse> resps;
    auto log_sec = []()
    { return utillib::ReadFile(LOG_SEC_NAME); };
    auto log_hour = []()
    { return utillib::ReadFile(LOG_HOUR_NAME); };
    auto log_day = []()
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

int main(int argc, char **argv)
{
    using namespace std;
    using namespace utillib;

    string serial_port_name(argc > 1 ? argv[1] : DEFAULT_SERIAL_PORT_NAME);
    string host_ip(argc > 2 ? argv[2] : DEFAULT_SERVER_HOST);
    signed int server_port = argc > 3 ? stoi(argv[3]) : DEFAULT_SERVER_PORT;

    thread serial_thread(SerialThread, serial_port_name);
    thread server_thread(ServerThread, host_ip, server_port);

    this_thread::sleep_for(chrono::milliseconds(10));

    cout << "lhg, ldg, lhg_m, ldg_m, sp, host" << endl;
    string input;
    while (true)
    {
        cin >> input;
        // modify last_hout_gather
        if (input == "lhg_m")
        {
            cout << "Inupt number of seconds to subtruct from now and apply to 'last_hout_gather':\n";
            cin >> input;
            try
            {
                last_hour_gather = GetUNIXTimeNow() - stoll(input);
            }
            catch (const std::invalid_argument &e)
            {
                cout << "Error parsing " << e.what() << endl;
            }
        }
        else if (input == "ldg_m")
        {
            cout << "Inupt number of seconds to subtruct from now and apply to 'last_day_gather':\n";
            cin >> input;
            try
            {
                last_day_gather = GetUNIXTimeNow() - stoll(input);
            }
            catch (const std::invalid_argument &e)
            {
                cout << "Error parsing " << e.what() << endl;
            }
        }
        else if (input == "lhg")
        {
            cout << "last_hout_gather = " << last_hour_gather << " | " << GetTimeFromSec(last_hour_gather) << endl;
        }
        else if (input == "ldg")
        {
            cout << "last_day_gather  = " << last_day_gather << " | " << GetTimeFromSec(last_day_gather) << endl;
        }
        else if (input == "sp")
        {
            cout << "Sreial port is listening to '" << serial_port_name << "'" << endl;
        }
        else if (input == "host")
        {
            cout << "Server is listening to 'http://" << host_ip << ":" << server_port << "'" << endl;
        }
    }
}