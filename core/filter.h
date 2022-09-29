#pragma once
#if defined(_MSC_VER)
#define NOMINMAX
// #pragma once
#endif

#include<mygeo/vec.h>
#include"../geometry/space.h"



// Filter Declarations
class Filter 
{
  public:
    // Filter Interface
    virtual ~Filter();
    Filter(const MyGeo::Vec2f &radius) : radius{radius}, invRadius{MyGeo::Vec2f{1 / radius.x, 1 / radius.y}} 
    {}
    virtual float Evaluate(const Point &p) const = 0;

    // Filter Public Data
    const MyGeo::Vec2f radius, invRadius;
};
