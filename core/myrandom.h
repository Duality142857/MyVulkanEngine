#pragma once
#include<random>
#include<type_traits>
#include<concepts>
#include<mygeo/vec.h>
// #include<ranges>
// #include<math.h>
// #include"mytime.h"

template<class T>
concept is_int=std::is_integral_v<T>;

template<class T>
concept is_real=std::is_floating_point_v<T>;

template<is_real T>
static T getRand(const T & min=0.f, const T & max=1.f) 
{//thread_local
    static  std::mt19937 generator;
    std::uniform_real_distribution<T> distribution(min,max);
    return distribution(generator);
}
template<is_int T>
static T getRand(const T & min=0, const T & max=RAND_MAX) 
{//thread_local
    static  std::mt19937 generator;
    std::uniform_int_distribution<T> distribution(min,max);
    return distribution(generator);
}

template<class T>
static MyGeo::Vec<T,3> randomVec3(T min, T max)
{
    return MyGeo::Vec<T,3>{getRand<T>(min,max),getRand<T>(min,max),getRand<T>(min,max)};
}

//random vector in unit sphere, 长度<=1
template<class T>
static MyGeo::Vec<T,3> randomInUnitSphere()
{
    while(true)
    {
        auto p=randomVec3<T>(-1,1);
        if(p.norm2()>=1) continue;
        return p;
    }
}

template<class T>
static MyGeo::Vec<T,3> randomUnitVector()
{
    return randomInUnitSphere<T>().normalVec();
}

template<class T>
static MyGeo::Vec<T,3> randomInHemisphere(const MyGeo::Vec<T,3>& normal)
{
    MyGeo::Vec<T,3> inUnitSphere=randomUnitVector<T>();//randomInUnitSphere<T>();
    if(inUnitSphere.dot(normal)>0) return inUnitSphere;
    return -inUnitSphere;
}
template<class T>
inline MyGeo::Vec<T,3> randomInUnitDisk()
{
    while(true)
    {
        auto p=MyGeo::Vec<T,3>{getRand<T>(-1,1),getRand<T>(-1,1),0};
        if(p.norm2()>=1) continue;
        return p;
    }
}



// static int getRandInt(int min, int max)
// {
//     return min+rand()%(max-min);
// }

// static float getRandFloat(float min, float max)
// {
//     return (float)rand()/(float)RAND_MAX;
// }

// static void test()
// {
//     auto t0=mytime::now();
//     for(auto _:std::views::iota(0,1e8))
//     {
//         getRand(0.f,1.f);
//     }
//     auto t1=mytime::now();
//     std::cout<<"cpp "<<mytime::getDuration(t0,t1)<<" ms"<<std::endl;

//     t0=mytime::now();
//     for(auto _:std::views::iota(0,1e8))
//     {
//         getRandFloat(0.f,1.f);
//     }
//     t1=mytime::now();
//     std::cout<<"c "<<mytime::getDuration(t0,t1)<<" ms"<<std::endl;
// }
