#include "serial.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <string>
#include <chrono>

std::string DEFAULT_PORT = "COM3";

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
    std::normal_distribution norm(0.0, 8.0);

    std::string out;
    for (;;) {
        out = "$$" + std::to_string(norm(gen)) + "$$";
        serial_port << out;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}