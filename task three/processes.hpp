#ifdef WIN32

#include <windows.h>

#else

#include <spawn.h>
#include <wait.h>
#include <string.h>

#endif

#include <format>
#include <string>
// #include <iostream>

namespace proclib
{
    int start_process(int argc, char** argv, int &status)
    {

#ifdef WIN32
        std::string command_line = "";
        for (int i = 0; i < argc; ++i) {
            command_line += std::format("{} ", argv[i]);
        }

        STARTUPINFOA si{};
        PROCESS_INFORMATION pi;
        int success = CreateProcessA(NULL,         // the path
                                     (char*)command_line.c_str(), // Command line
                                     NULL,         // Process handle not inheritable
                                     NULL,         // Thread handle not inheritable
                                     FALSE,        // Set handle inheritance to FALSE
                                     0,            // No creation flags
                                     NULL,         // Use parent's environment block
                                     NULL,         // Use parent's starting directory
                                     &si,          // Pointer to STARTUPINFO structure
                                     &pi           // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
        );

        if (success == 0)
        {
            status = GetLastError();
        }
        else
        {
            status = 0;
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

    int get_current_pid()
    {
#ifdef WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
    }

    std::string get_current_time_str()
    {
#ifdef WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);

    return std::format("{:0>4}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}.{:0>3}", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
    return "";
#endif
    }

    int wait_program(const int signed pid)
    {
#ifdef WIN32

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
}