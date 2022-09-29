#pragma once
#include<chrono>
#include<thread>

namespace mytime
{

static void sleep(int n)
{
    // std::this_thread::sleep_for(std::chrono::seconds(n));
    std::this_thread::sleep_for(std::chrono::milliseconds(n));
}

static std::chrono::high_resolution_clock::time_point now()
{
    std::chrono::high_resolution_clock::time_point t=std::chrono::high_resolution_clock::now();
    return t;
}
template<class T>
int64_t getDuration(std::chrono::high_resolution_clock::time_point t0,std::chrono::high_resolution_clock::time_point t1)
{
    
    auto dt= std::chrono::duration_cast<T>(t1-t0);
    return dt.count();
}

}