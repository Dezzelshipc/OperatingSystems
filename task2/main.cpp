#include "background.hpp"
#include <iostream>


int main(int argc, char **argv)
{

#ifdef _WIN32
    const char *path = "..\\subprogram.exe";
#else
    const char *path = "../subprogram";
#endif

    if (argc > 1)
    {
        path = argv[1];
    }

    int pid{};
    // pid = start_background(path);
    // auto exit_code = wait_program(pid);

    auto exit_code = start_wait(path, true);

    std::cout << pid << " " << exit_code << " Ended" << "\n";
}