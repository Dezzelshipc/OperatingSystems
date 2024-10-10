#include <iostream>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif // _WIN32

using namespace std;

void sleepcp(int milliseconds);

void sleepcp(int milliseconds) // Cross-platform sleep function
{
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif // _WIN32
}
int main()
{
    cout << "Hi! At the count to 3, I'll die! :)" << endl;
    sleepcp(3000);
    cout << "urrrrggghhhh!" << endl;
}