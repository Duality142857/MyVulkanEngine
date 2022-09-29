#pragma once
#include<mygeo/vec.h>
#include"../geometry/space.h"
#include"../geometry/transforms.h"
#include"material.h"
#include<memory>

struct Primitive;

struct HitRecord
{
    // float t;
    float tmin=0.3f;
    float t=std::numeric_limits<float>::max();
    float u,v;
    Point position;
    Normal normal;
    float area;//for pdf
    // const Primitive* primitive=nullptr;
    std::shared_ptr<Material> material;
    // HitRecord(){}
    
    void transform(const MatrixTransform& transform)
    {
        position=transform(position);
        normal=transform(normal);
    }
    void fixNormal(const MyGeo::Vec3f& raydirection)
    {
        if(normal.v3.dot(raydirection)>0) normal.v3=-normal.v3;
    }    

    friend std::ostream& operator<<(std::ostream& out,const HitRecord& rec)
    {
       return out<<"position "<<rec.position<<std::endl<<" normal "<<rec.normal<<std::endl;
    }

};