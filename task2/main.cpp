#include "background.hpp"
#include <iostream>

#include <cstring>
#include <filesystem>
namespace fs = std::filesystem;

int main(int argc, char **argv)
{

#ifdef _WIN32
    const char *in_path = "..\\subprogram.exe";
#else
    const char *in_path = "..\\subprogram";
#endif

    if (argc > 1)
    {
        in_path = argv[1];
    }

    fs::path path{in_path};

    if (path.is_relative())
    {
        path = fs::absolute(path);
    }

    if (!fs::exists(path))
    {
        std::cout << path << " Not exists\n";
        return 0;
    }

    std::cout << path << "\n";

    auto pid = start_background(path.string().c_str());
    auto exit_code = wait_program(pid);

    std::cout << pid << " " << exit_code << " Ended" << "\n";
}