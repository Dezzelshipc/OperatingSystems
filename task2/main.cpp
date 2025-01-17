#include "background.hpp"
#include <iostream>

int main(int argc, char *argv[])
{

#ifdef _WIN32
    const char *path = ".\\subprogram.exe";
#else
    const char *path = "./subprogram";
#endif

    bool is_wait = !(argc > 1) || std::stoi(argv[1]);

    if (argc > 2)
    {
        path = argv[2];
    }

    int pid = start_background(path);
    int exit_code = 0, status = 0;
    if (is_wait)
    {
        status = wait_program(pid, &exit_code);
    }

    std::cout << "Ended pid=" << pid << " exit_code=" << exit_code << " status=" << status << "\n";
}