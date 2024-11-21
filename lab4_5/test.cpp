// http test

#include "socket.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

std::string test()
{
    return "LOOOGGG!!";
}

std::string log1()
{
    std::ifstream log_s("./logs/second.log");
    std::stringstream str;
    str << log_s.rdbuf();
    return "<pre>" + str.str() + "</pre>\n";
}

int main(int argc, char **argv)
{
    sclib::HTTPServer srv;
    srv.Listen("127.0.0.1", 8010);

    std::vector<sclib::Response> resps;
    resps.emplace_back("GET", "/", test);
    resps.emplace_back("GET", "/sec/raw", log1);

    srv.RegisterResponses(resps);
    for (;;)
    {
        if (!srv.IsValid())
        {
            std::cerr << "ASDASDSAD!!\n";
            break;
        }
        srv.ProcessClient("Abdya");
    }
}