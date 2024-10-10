#ifdef _WIN32
    #include <windows.h>
#else

#endif

#include <iostream>
#include <filesystem>

#include "background.hpp"

int signed start_background(const char* program_path) {

    #ifdef _WIN32
    
    STARTUPINFO si; 
    PROCESS_INFORMATION pi;
    CreateProcess( program_path,   // the path
        NULL,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );

    std::cout << "Process id: " << pi.dwProcessId << "\n";
    return pi.dwProcessId;

    #else
    return 0;

    #endif
}

int wait_program(const int signed pid) {
    #ifdef _WIN32
    
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    return WaitForSingleObject(handle, INFINITE);

    #else
    return 0;

    #endif
}