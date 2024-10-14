#ifdef _WIN32

#include <windows.h>

#else

#include <spawn.h>
#include <wait.h>
#include <string.h>

#endif

// #include <iostream>
#include "background.hpp"

int signed start_background(const char *program_path)
{
    // std::cout << program_path << "\n";

#ifdef _WIN32

    STARTUPINFO si{};
    PROCESS_INFORMATION pi;
    int status = CreateProcess(program_path, // the path
                               NULL,         // Command line
                               NULL,         // Process handle not inheritable
                               NULL,         // Thread handle not inheritable
                               FALSE,        // Set handle inheritance to FALSE
                               0,            // No creation flags
                               NULL,         // Use parent's environment block
                               NULL,         // Use parent's starting directory
                               &si,          // Pointer to STARTUPINFO structure
                               &pi           // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );

    // if (status == 0) {
    //     std::cout << status << " " << GetLastError() << "\n";
    // }

    return pi.dwProcessId;

#else

    pid_t pid;
    char *argv[] = {};
    int status = posix_spawn(
        &pid,
        program_path,
        NULL,
        NULL,
        argv,
        NULL);

    // std::cout << strerror(status) << "\n";

    return pid;
#endif
}

int wait_program(const int signed pid)
{
#ifdef _WIN32

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    signed int status = WaitForSingleObject(handle, INFINITE);
    CloseHandle(handle);

    return status;

#else

    int status;
    waitpid(pid, &status, 0);

    // std::cout << status << " "<< strerror(status) <<"\n";
    return status;

#endif
}