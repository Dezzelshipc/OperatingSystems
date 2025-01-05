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

    return pid;
#endif
}

int start_background(const char *program_path) {
    int status = 0;
    return start_background(program_path, status);
}

int wait_program(const int pid, int* exit_code)
{
#ifdef _WIN32

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    int status = WaitForSingleObject(handle, INFINITE);
    if (exit_code != nullptr)
    {
        GetExitCodeProcess(handle, (unsigned long*)exit_code);
    }
    CloseHandle(handle);

    return status;

#else

    int status;
    waitpid(pid, &status, 0);
    if (exit_code != nullptr)
    {
        *exit_code = WEXITSTATUS(status);
    }

    return WTERMSIG(status);

#endif
}