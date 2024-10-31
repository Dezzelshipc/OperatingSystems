#include "shmem.hpp"
#include "processes.hpp"
#include <iostream>
#include <fstream>

#include <thread>
#include <chrono>
#include <atomic>

#include <vector>
#include <string>
#include <format>

#define LOG_FILE_NAME "./log.txt"

typedef std::thread::id pid;

struct Data
{
    int counter = 0;

    int copy_num = 0;
    int count_programs = 0;

    int main_process = -1;
};

shlib::SharedMem<Data> GetSharedMem()
{
    return shlib::SharedMem<Data>("sheared_memory");
}

template <typename T>
void write_file(shlib::SharedMem<Data> &shared_data, T &str)
{
    shared_data.Lock();
    std::ofstream log(LOG_FILE_NAME, std::ios::app);
    log << str << "\n";
    shared_data.Unlock();
}

bool is_working(shlib::SharedMem<Data> &shared_data)
{
    shared_data.Lock();
    bool is_working = shared_data.Data()->count_programs > 0;
    shared_data.Unlock();
    return is_working;
}

std::string get_time_str()
{
    // auto now_c = std::chrono::system_clock::now();
    // std::time_t now = std::chrono::system_clock::to_time_t(now_c);

    // std::string time_str(30, '\0');
    // std::strftime(&time_str[0], time_str.size(), "%Y-%m-%d %H:%M:%S.000", std::localtime(&now));
    // return time_str.c_str();
    return proclib::get_current_time_str();
}

std::string get_pid_str()
{
    return std::to_string(proclib::get_current_pid());
}

void wait_other(const std::atomic_bool &is_thread_running)
{
    auto shared_data = GetSharedMem();
    auto this_pid = proclib::get_current_pid();
    auto time_sleep = std::chrono::milliseconds(10);

    do
    {
        shared_data.Lock();
        auto main_proc = shared_data.Data()->main_process;
        if (main_proc == -1)
        {
            shared_data.Data()->main_process = this_pid;
        }
        shared_data.Unlock();
        if (main_proc == this_pid)
        {
            break;
        }
        std::this_thread::sleep_for(time_sleep);
    } while (is_thread_running);
}

void counter_thread(const std::atomic_bool &is_thread_running)
{
    auto time_sleep = std::chrono::milliseconds(300);
    auto shared_data = GetSharedMem();
    while (shared_data.IsValid() && is_thread_running)
    {
        shared_data.Lock();
        shared_data.Data()->counter += 1;
        shared_data.Unlock();
        std::this_thread::sleep_for(time_sleep);
    }
}

void copies_thread(const std::atomic_bool &is_thread_running)
{
    wait_other(is_thread_running);

    auto shared_data = GetSharedMem();
    auto this_pid = proclib::get_current_pid();
    auto time_sleep = std::chrono::seconds(3);

    std::cout << "THREAD COPIES\n";
}

void log_thread(const std::atomic_bool &is_thread_running)
{
    wait_other(is_thread_running);

    auto shared_data = GetSharedMem();
    auto this_pid = proclib::get_current_pid();
    auto time_sleep = std::chrono::seconds(1);

    while (shared_data.IsValid() && is_thread_running)
    {
        shared_data.Lock();
        auto counter = shared_data.Data()->counter;
        std::string time_pid = std::format("[{} | {}] {} {}", get_time_str(), get_pid_str(), counter, shared_data.Data()->count_programs);
        write_file(shared_data, time_pid);
        shared_data.Unlock();
        std::this_thread::sleep_for(time_sleep);
    }

    shared_data.Lock();
    auto main_process = shared_data.Data()->main_process;
    if (main_process == this_pid)
    {
        shared_data.Data()->main_process = -1;
    }
    shared_data.Unlock();
}

int main(int argc, char **argv)
{
    std::cout << std::format("Started {} : pid={}. Log file: {}\n", argv[0], get_pid_str(), LOG_FILE_NAME);

    auto shared_data = shlib::SharedMem<Data>("sheared_memory");
    if (!shared_data.IsValid())
    {
        std::cout << "Failed to create shared memory block!" << std::endl;
        return -1;
    }

    std::vector<std::thread> threads;

    std::atomic_bool is_runnig = true;
    if (!is_working(shared_data))
    {
        std::ofstream log(LOG_FILE_NAME);
        log.clear();
    }

    shared_data.Lock();
    shared_data.Data()->count_programs++;
    threads.emplace_back(counter_thread, std::ref(is_runnig));
    threads.emplace_back(log_thread, std::ref(is_runnig));
    threads.emplace_back(copies_thread, std::ref(is_runnig));

    shared_data.Unlock();

    std::string time_pid = std::format("[{} | {}] Started", get_time_str(), get_pid_str());
    write_file(shared_data, time_pid);

    std::string param;
    int set_counter = 0;
    while (true)
    {
        std::cin >> param;
        if (param == "e" || param == "exit")
        {
            break;
        }
        else if (param == "m" || param == "mpdify")
        {
            std::cout << "Enter int to set counter to\n";
            std::cin >> set_counter;
            shared_data.Lock();
            shared_data.Data()->counter = set_counter;
            shared_data.Unlock();
        }
        else if (param == "s" || param == "show")
        {
            shared_data.Lock();
            auto counter = shared_data.Data()->counter;
            shared_data.Unlock();
            std::cout << "Counter value: " << counter << "\n";
        }
    }
    std::cout << "Exiting...\n";

    shared_data.Lock();
    shared_data.Data()->count_programs--;
    shared_data.Unlock();

    std::cout << "Joining threads\n";
    is_runnig = false;

    for (auto &t : threads)
    {
        t.join();
    }
    std::cout << "Goodbye\n";

    time_pid = std::format("[{} | {}] Finished", get_time_str(), get_pid_str());
    write_file(shared_data, time_pid);

    return 0;
}