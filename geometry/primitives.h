#pragma once
#include"../core/material.h"
#include<memory>

struct Primitive : Object
{
    std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material;

    Primitive(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material):shape{shape},material{material}{}
    // std::shared_ptr
    virtual BB3f bound() const override
    {
        return shape->bound();
    }
    virtual bool hit(const Ray& ray,HitRecord& rec) const override
    {
        if(!shape->hit(ray,rec)) return false;
        rec.material=material;
        return true;
    }

    void sample(HitRecord& sampleRec) const 
    {
        shape->sample(sampleRec);
        sampleRec.material=material;
    }
};


