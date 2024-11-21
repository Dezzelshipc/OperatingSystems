// http test

#include "server.hpp"
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
    return str.str();
}

std::string log2()
{
    return "<pre>" + log1() + "</pre>\n";
}


int main(int argc, char **argv)
{
    using namespace srvlib;

    HTTPServer srv;
    srv.Listen("127.0.0.1", 8010);

    std::vector<Response> resps;
    resps.emplace_back("GET", "/", test);
    resps.emplace_back("GET", "/sec/raw", log1, true);
    resps.emplace_back("GET", "/sec/raw2", log2);

    srv.RegisterResponses(resps);
    for (;;)
    {
        if (!srv.IsValid())
        {
            std::cerr << "ASDASDSAD!!\n";
            break;
        }
        srv.ProcessClient();
    }
}