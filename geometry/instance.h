#pragma once
#include"shapes.h"
#include"transforms.h"
#include"primitives.h"
#include<memory>
struct Instance: public Object
{
    std::shared_ptr<Primitive> primitive;
    MatrixTransform transform;
    MatrixTransform transform_inv;
    virtual float area() const 
    {
        //todo,not right under some transformation
        return primitive->shape->area();
    }
    virtual BB3f bound() const override
    {
        return transform(primitive->bound());
    }

    Instance(const MatrixTransform& transform, std::shared_ptr<Primitive> primitive):transform{transform},primitive{primitive},transform_inv{transform.inverse()}
    {}
    
    void sample(HitRecord& sampleRec) const 
    {
        primitive->sample(sampleRec);
        sampleRec.transform(transform);
        // std::cout<<sampleRec;
    }

    virtual bool hit(const Ray& ray, HitRecord& rec) const override
    {
        auto flag=primitive->hit(transform_inv(ray),rec);
        if(flag) 
        {
            rec.transform(transform);
        }
        return flag;
    }
    
};
