#pragma once
#include"../geometry/ray.h"
#include"record.h"
struct Object
{
    virtual bool hit(const Ray& ray, HitRecord& rec) const =0;
    virtual BB3f bound() const =0;

};
