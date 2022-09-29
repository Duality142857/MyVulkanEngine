#pragma once
#include"vec.h"
struct Ray
{
    const MyGeo::Vec3f source;
    const MyGeo::Vec3f direction;
    Ray(const MyGeo::Vec3f& source, const MyGeo::Vec3f& direction):source{source},direction{direction}{}
    MyGeo::Vec3f at(float t) const 
    {
        return {source+t*direction};
    }

};
