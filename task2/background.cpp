#ifdef _WIN32

#include <windows.h>

#else

#include <spawn.h>
#include <wait.h>
#include <string.h>

#endif

// #include <iostream>
#include "background.hpp"

int start_background(const char *program_path, int& status)
{
    // std::cout << program_path << "\n";

#ifdef _WIN32

    STARTUPINFO si{};
    PROCESS_INFORMATION pi;
    int success = CreateProcess(program_path, // the path
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

    if (success == 0) {
        status = GetLastError();
    }

    return pi.dwProcessId;

#else

    pid_t pid;
    char *const argv[] = {NULL};
    status = posix_spawnp(
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

int start_background(const char *program_path) {
    int status{};
    return start_background(program_path, status);
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

int start_wait(const char *program_path, bool is_wait)
{
    int status{};
    int pid = start_background(program_path, status);

    if (status != 0) {
        return status;
    }

    if (is_wait)
    {
        return wait_program(pid);
    }
    return 0;
}