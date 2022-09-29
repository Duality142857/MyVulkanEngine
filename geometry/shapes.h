#pragma once
#include"bbox.h"
#include"ray.h"
#include"../core/record.h"
#include"transforms.h"
#include"utils.h"
#include"../core/object.h"
#include"../core/myrandom.h"
#include<math.h>






struct Shape : public Object
{
    // const Transform *model, *model_inv;
    // virtual BB3f bound() const =0;
    // virtual BB3f worldBound() const;
    virtual float area() const =0;
    // virtual bool hit(const Ray& ray, HitRecord& rec) const =0;
    // virtual bool hit(const Ray& ray, HitRecord& rec) const =0;
    virtual void sample(HitRecord& record) const =0;

};



struct Sphere: public Shape
{
    const float radius,radius2;

    Sphere(float radius):radius{radius},radius2{radius*radius}{}

    float area() const override {return 4.f*Pi*radius*radius;}

    virtual void sample(HitRecord& sampleRec) const override
    {
        float theta=getRand(0.f,Pi);   
        float phi=getRand(0.f,2*Pi);
        float z=radius*std::cos(theta);
        float g=radius*std::sin(theta);
        float x=g*std::cos(phi);
        float y=g*std::sin(phi);
        sampleRec.position={x,y,z};
        sampleRec.normal={MyGeo::Vec3f{x,y,z}.normalize()};
    }

    
    bool hit(const Ray& ray, HitRecord& rec) const override
    {
        if(!bound().hitP(ray)) return false;
        MyGeo::Vec3f l=ray.source.v3;
        float a=ray.direction.v3.norm2();
        float b=2*ray.direction.v3.dot(l);
        float c=l.norm2()-radius2;

        auto solution=solveQuadratic(a,b,c);
        if(!solution.has_value()) return false;
        auto [x1,x2]=solution.value();

        if(x2<=rec.tmin) return false;
        auto t=x1<rec.tmin?x2:x1;
        if(t>rec.t) return false;

        rec.t=t;
        rec.position=ray.at(t);
        // std::cout<<"raw pos "<<rec.position<<std::endl;
        rec.normal=Normal{rec.position.v3.normalVec()};
        // std::cout<<"hit sphere!"<<std::endl;
        return true;
    }
    virtual BB3f bound() const override
    {
        return BB3f{{-radius,-radius,-radius},{radius,radius,radius}};
    }
    // virtual BB3f worldBound() const
};
