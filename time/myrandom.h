#pragma once

#include<random>
namespace myrand{
// static uint32_t seed_val;
static std::default_random_engine generator;
static std::uniform_real_distribution<float> floatdist(0,1);

// static std::mt19937 myrand();
// static void randInit()
// {
//     // static auto dice=std::bind(floatdist,generator);
    
// }
static int randInt()
{
    return std::rand();
}

static float randFloat()
{
    return floatdist(generator);
}
}


