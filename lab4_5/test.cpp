// http test

#include "socket.hpp"
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    HTTPServer srv;
    srv.Listen("127.0.0.1", 8010);
    // socket init
    for (;;)
    {
        if (!srv.IsValid())
        {
            std::cerr << "ASDASDSAD!!\n";
            break;
        }
        srv.ProcessClient("Abdya");
    }
    // socket end
}