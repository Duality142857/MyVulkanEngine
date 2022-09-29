#pragma once
#include<mygeo/vec.h>
#include<optional>
#include"myrandom.h"
enum ScatterFlag {Nicht=0,Reflect=1,Transmit=2,Specular=4,DIELECTRIC=8};

struct Material
{
    virtual float getIor() const {return 0;}

    virtual MyGeo::Vec3f wr(const MyGeo::Vec3f& wo, const MyGeo::Vec3f& n) const
    {
        return {1,0,0};
    }
    virtual MyGeo::Vec3f wt(const MyGeo::Vec3f& wo, const MyGeo::Vec3f& n, const float ior0) const
    {
        return {1,0,0};
    }


    virtual ScatterFlag getScatterFlag() const 
    {
        return Nicht;
    }
    virtual MyGeo::Vec3f getColor(float u,float v) const =0;     
    // virtual MyGeo::Vec3f emitColor(float u,float v) const =0;
    virtual MyGeo::Vec3f brdf(const MyGeo::Vec3f& lo, const MyGeo::Vec3f& li, const MyGeo::Vec3f& n) const
    {
        return {0,0,0};
    }
    virtual bool ifemit() const 
    {
        return false;
    }
};

struct Light : public Material
{
    virtual bool ifemit() const override 
    {
        return true;
    }
    // virtual float reciPDF() const =0;
};

struct MonoLight : public Light
{
    MyGeo::Vec3f color;
    MonoLight(const MyGeo::Vec3f& color):color{color}{}
    virtual MyGeo::Vec3f getColor(float u,float v) const override
    {
        return color;
    }    
    // virtual float reciPDF() const override
    // {
    //     return 1
    // }

};


struct Lambertian : public Material
{
    virtual ScatterFlag getScatterFlag() const override
    {
        return Reflect;
    }

    MyGeo::Vec3f color;
    MyGeo::Vec3f albedo;
    //wr for reflect, but is reversed
    MyGeo::Vec3f brdf(const MyGeo::Vec3f& wo, const MyGeo::Vec3f& wr, const MyGeo::Vec3f& n) const override
    {
        return color*ReciPi;
    }

    // MyGeo::Vec3f scatter(const MyGeo::Vec3f& wo, const MyGeo::Vec3f& n) const 
    // {
    //     return brdf(wo,dvec_normalized,n)*getColor(rec.u,rec.v)*cos0*cos1*d2_reci*sampleRec.area;

    // }

    MyGeo::Vec3f wr(const MyGeo::Vec3f& wo, const MyGeo::Vec3f& n) const override
    {
        auto res=randomInHemisphere(n);
        assert(res.dot(n)>0);
        return res;
    }


    Lambertian(const MyGeo::Vec3f& color):color{color},albedo{0.8,0.8,0.8}{}
    Lambertian(const MyGeo::Vec3f& color, const MyGeo::Vec3f& albedo):color{color},albedo{albedo}{}

    virtual MyGeo::Vec3f getColor(float u,float v) const override
    {
        return color;
    }
};

struct Mirror : public Material
{
    virtual ScatterFlag getScatterFlag() const override
    {
        return Specular;
    }

    MyGeo::Vec3f color;
    MyGeo::Vec3f albedo;
    MyGeo::Vec3f brdf(const MyGeo::Vec3f& lo, const MyGeo::Vec3f& li, const MyGeo::Vec3f& n) const override
    {
        // return color*ReciPi*reflect(li,n).dot(lo)*3;
        return color*ReciPi;
    }

    MyGeo::Vec3f wr(const MyGeo::Vec3f& wo, const MyGeo::Vec3f& n) const override
    {
        // std::cout<<wo<<std::endl;
        // std::cout<<n<<std::endl;
        // std::cout<<reflect(wo,n)<<std::endl;
        // exit(0);
        return reflect(wo,n);
    }

    Mirror(const MyGeo::Vec3f& color):color{color},albedo{0.8,0.8,0.8}{}
    Mirror(const MyGeo::Vec3f& color, const MyGeo::Vec3f& albedo):color{color},albedo{albedo}{}
    // float pdf()

    // virtual bool ifemit() const override 
    // {
    //     return false;
    // }
    virtual MyGeo::Vec3f getColor(float u,float v) const override
    {
        return color;
    }
};

struct Dielectric : public Material
{
    float ior;
    float getIor() const override 
    {
        return ior;
    }
    virtual ScatterFlag getScatterFlag() const override
    {
        // return (ScatterFlag) (Transmit | Reflect);
        return DIELECTRIC;
    }
    MyGeo::Vec3f color;
    MyGeo::Vec3f brdf(const MyGeo::Vec3f& lo, const MyGeo::Vec3f& li, const MyGeo::Vec3f& n) const override
    {
        // return color*ReciPi*reflect(li,n).dot(lo)*3;
        return color*ReciPi;
    }

    MyGeo::Vec3f wr(const MyGeo::Vec3f& wo, const MyGeo::Vec3f& n) const override
    {
        return reflect(wo,n);
    }

    MyGeo::Vec3f wt(const MyGeo::Vec3f& wo, const MyGeo::Vec3f& n, const float ior0) const override
    {
        return refract(wo,n,ior0,ior);
    }

    Dielectric(const float ior):ior{ior}{}
    // float pdf()

    // virtual bool ifemit() const override 
    // {
    //     return false;
    // }
    virtual MyGeo::Vec3f getColor(float u,float v) const override
    {
        return color;
    }
};