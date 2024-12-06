#include "serial.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

std::string DEFAULT_PORT = "COM3";

double months_temp[] = {-11.6, -7.6, -1.0, 6., 10.7, 14.5, 18.8, 20.6, 16.7, 9.6, -0.1, -8.8};
double get_month_temp(double t_month) // 0-11 == Jan-Dec
{
    short month = short(t_month);
    double start_month_temp = months_temp[month];
    double end_month_temp = months_temp[month > 10 ? 0 : month + 1];
    
    double t_this_month = t_month - month;

    return start_month_temp * (1 - t_this_month) + end_month_temp * t_this_month;

}

double get_day_temp(double t_day) // 0-1 == 0h - 24h
{
    if (t_day < 0.25)
    {
        return -4 - 2*(t_day*4);
    }
    else if (t_day < 0.5)
    {
        return -6 + 14 * (t_day - 0.25)*4;
    }
    else if (t_day < 0.75)
    {
        return 8 - 4 * (t_day - 0.5)*4;
    }
    else
    {
        return 4 - 8 * (t_day - 0.75)*4;
    }
}

constexpr double DAY_SEC = 60*60*24;
double get_t_day(auto utc_now)
{
    auto local_now = zoned_time{current_zone(), floor<seconds>(utc_now)}.get_local_time();
    auto local_midnight = floor<days>(local_now);
    auto delta = local_now - local_midnight;
    return delta.count() / DAY_SEC;
}

double get_t_month(auto utc_now)
{
    year_month_day ymd{floor<days>(utc_now)};

    year_month_day_last ymdl
    {
        ymd.year(), ymd.month() / std::chrono::last
    };
    std::chrono::year_month_day ymdll{ymdl};

    return (unsigned(ymd.month()) - 1) + (unsigned(ymd.day()) - 1) / unsigned(ymdll.day());
}

int main(int argc, char **argv)
{
    std::string port(DEFAULT_PORT);
    if (argc > 1) {
        port = argv[1];
    }

    splib::SerialPort serial_port(port, splib::SerialPort::BAUDRATE_115200);

    if (!serial_port.IsOpen())
    {
        std::cout << "Args: [1: port]; default: " << DEFAULT_PORT << std::endl;
        std::cerr << "Failed to open " << port << " port!" << std::endl;
        return -2;
    }
    std::cout << "Sending to port: " << port << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution norm(0.0, 0.001);    

    std::string out;
    for (;;) {
        auto utc_now = system_clock::now();
        double t_day = get_t_day(utc_now);
        double t_month = get_t_month(utc_now);
        out = "$$" + std::to_string( get_month_temp(t_month) + get_day_temp(t_day) + norm(gen)) + "$$";
        serial_port << out;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}