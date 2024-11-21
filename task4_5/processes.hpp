#ifdef WIN32

#include <windows.h>

#else

#include <spawn.h>
#include <wait.h>
#include <string.h>
#include <time.h>
#include <chrono>

#endif

#include <format>
#include <string>
// #include <iostream>

namespace proclib
{
    int start_process(int argc, char **argv, int &status)
    {

#ifdef WIN32
        std::string command_line = "";
        for (int i = 0; i < argc; ++i)
        {
            command_line += std::format("{} ", argv[i]);
        }

        STARTUPINFOA si{};
        PROCESS_INFORMATION pi;
        int success = CreateProcessA(NULL,                         // the path
                                     (char *)command_line.c_str(), // Command line
                                     NULL,                         // Process handle not inheritable
                                     NULL,                         // Thread handle not inheritable
                                     FALSE,                        // Set handle inheritance to FALSE
                                     0,                            // No creation flags
                                     NULL,                         // Use parent's environment block
                                     NULL,                         // Use parent's starting directory
                                     &si,                          // Pointer to STARTUPINFO structure
                                     &pi                           // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
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
        status = posix_spawnp(
            &pid,
            argv[0],
            NULL,
            NULL,
            argv,
            NULL);

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

        return std::format("{:0>4}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}",
                           st.wYear,
                           st.wMonth,
                           st.wDay,
                           st.wHour,
                           st.wMinute,
                           st.wSecond);
#else
        auto now = std::chrono::system_clock::now();

        time_t t = std::chrono::system_clock::to_time_t(now);
        tm *st = localtime(&t);

        return std::format("{:0>4}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}",
                           st->tm_year + 1900,
                           st->tm_mon + 1,
                           st->tm_mday,
                           st->tm_hour,
                           st->tm_min,
                           st->tm_sec);
#endif
    }

    int wait_program(const int pid)
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