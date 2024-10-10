#include "background.hpp"
#include <iostream>

int main(int argc, char **argv)
{
#ifdef _WIN32
    const auto path = "C:\\Programs\\Github\\Operating-systems\\task2\\subprogram.exe";
#else
    const auto path = "/home/dezz/Github/Operating-systems/task2/subprogram";
#endif

    std::cout << path << "\n";
    auto pid = start_background(path);
    auto exit_code = wait_program(pid);

    std::cout << pid << " " << exit_code << " Ended" << "\n";
}