#pragma once
#include"object.h"
#include"camera.h"
#include"../geometry/aggregates.h"
#include"myrandom.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image_write.h>
#include<fstream>

// #include <stb_image_write.h>


struct Canvas
{
    int width, height;
    std::vector<MyGeo::Vec3c> framebuffer;
    std::shared_ptr<PerspectiveCamera> camera;
    MyGeo::Vec3f backgroundColor{0,0,0};


    Canvas(int width, int height):
    width{width},height{height}
    {
        framebuffer.resize(width*height);
    }

    void setCamera(std::shared_ptr<PerspectiveCamera> cam){camera=cam;}

    void setPixel(int i, int j, const MyGeo::Vec3f& color)
    {
        framebuffer[j*width+i]=
        MyGeo::Vec3c{(uint8_t)(255*clamp(0.f,0.999f,color.x)),
        (uint8_t)(255*clamp(0.f,0.999f,color.y)),
        (uint8_t)(255*clamp(0.f,0.999f,color.z))};

        // std::cout<<(int)framebuffer[j*width+i].x<<(int)framebuffer[j*width+i].y<<(int)framebuffer[j*width+i].z<<std::endl;
    }

    void render(const std::vector<std::shared_ptr<Instance>>& instances,const std::vector<std::shared_ptr<Instance>>& lights ,int spp, int maxdepth)
    {
        for(int j=0;j<height;++j)
        {
            std::cerr<<"\rFinished "<<j<<" lines"<<std::flush;
#pragma omp parallel for num_threads(8)
            for(int i=0;i<width;++i)
            {
                MyGeo::Vec3f pixelColor{0,0,0};
                for(int k=0;k<spp;++k)
                {
                    auto ray=camera->genRay(i,j);
                    pixelColor+=trace(ray,instances,lights,1);
                }
                pixelColor/=spp;
                for(auto& x:pixelColor.data)
                {
                    x=std::fabs(x);
                }
                setPixel(i,j,pixelColor);
            }
        }
        std::cout<<"write png"<<std::endl;
        stbi_write_png("test.png",width,height,3,framebuffer.data(),3*width);
    }

float lightArea=0;

    //
    bool hit(const Ray& ray, const std::vector<std::shared_ptr<Instance>>& instances, HitRecord& rec)
    {
        bool hitflag=false;
        for(auto const& instance:instances)
        {
            if(instance->hit(ray,rec)) hitflag=true;
        }
        return hitflag;
    }

    //trace a ray and get back the color
    MyGeo::Vec3f trace(const Ray& ray, const std::vector<std::shared_ptr<Instance>>& instances,const std::vector<std::shared_ptr<Instance>>& lights, int depth)
    {
        if(depth>16) return {0,0,0};

        HitRecord rec;
        bool hitflag=hit(ray,instances,rec);
        if(!hitflag) return backgroundColor;
        return shade(instances,lights,-ray.direction.v3,rec,depth);
    }


    MyGeo::Vec3f traceBHV(const Ray& ray, float t0, float t1, const BVH& bvh, int maxdepth)
    {
        HitRecord rec;
        if(!bvh.hit(ray,rec)) return backgroundColor;

        if(rec.material->ifemit())
        {
            return rec.material->getColor(rec.u,rec.v);
        }

        return backgroundColor;
    }


    //shade surface point
    //sample a light, then trace on
    //lo: direction of tracing ray, li will be generated later
    MyGeo::Vec3f shade(const std::vector<std::shared_ptr<Instance>>& instances,const std::vector<std::shared_ptr<Instance>>& lights, const MyGeo::Vec3f& wo, HitRecord& rec,int depth)
    {
        //! reach depth limit, return 0
        if(depth>32) return {0,0,0};

        //!hit surface is light, level 0 lighting
        if(rec.material->ifemit())
        {
            if(depth>1) return {0,0,0};
            return rec.material->getColor(rec.u,rec.v);
        }

        MyGeo::Vec3f L_direct{0,0,0};

        if(!(rec.material->getScatterFlag() & (Specular|DIELECTRIC)))
        {
            //!Sample light to surface, level 1 lighting
            HitRecord sampleRec;
            sampleLight(lights,sampleRec);
            //check if surface and light sample point are blocked by other objects
            
            auto dvec=sampleRec.position.v3-rec.position.v3;
            auto dvec_normalized=dvec.normalVec();
            sampleRec.fixNormal(dvec_normalized);

            auto d2_reci=1.f/dvec.norm2();
            float cos0=dvec_normalized.dot(rec.normal.v3);//surface cos
            float cos1=-dvec_normalized.dot(sampleRec.normal.v3);//lightsample cos

            HitRecord blockRec;
            Ray ray{rec.position,dvec_normalized};
            bool blockHitFlag=hit(ray,instances,blockRec);
            auto blockVec=(blockRec.position-rec.position).v3;
            auto blockd2=blockVec.norm2();

            // second judgement is a trick, ensuring only oneside of the rect is light
            if(blockd2>=dvec.norm2()-0.01f && blockVec.dot(blockRec.normal.v3)<=0 ) 
            {
                MyGeo::Vec3f L_direct_r=rec.material->brdf(wo,dvec_normalized,rec.normal.v3)*sampleRec.material->getColor(rec.u,rec.v)*cos0*cos1*d2_reci*sampleRec.area;
                MyGeo::Vec3f L_direct_t={0,0,0};
                L_direct=L_direct_r+L_direct_t;
            }
        }

        static const float rus=0.8;
        if(getRand(0.f,1.f)>rus) return L_direct;
        rec.fixNormal(-wo);

        MyGeo::Vec3f L_indirect{0,0,0};
        auto wr=rec.material->wr(wo,rec.normal.v3);

        if(rec.material->getScatterFlag() == DIELECTRIC)
        {
            // float R=fresnel(-wo,rec.normal.v3,1.f,rec.material->getIor());
            float R=0;
            // std::cout<<rec.material->getIor()<<" R "<<R<<std::endl;
            float T=1-R;
            auto wt=rec.material->wt(-wo,rec.normal.v3,1.f);
            // HitRecord rRec;
            // Ray rRay{rec.position,wr};
            // bool rHitFlag=hit(rRay,instances,rRec);
            // if(!rHitFlag || rRec.material->ifemit()) return L_direct;
            // auto L_indirect_R=R*trace(rRay,instances,lights,depth+1);
            HitRecord tRec;
            Ray tRay{rec.position,wt};
            // bool tHitFlag=hit(tRay,instances,tRec);
            // if(!tHitFlag || tRec.material->ifemit()) return L_direct;
            auto L_indirect_T=trace(tRay,instances,lights,depth+1);

            L_indirect=L_indirect_T;
            return L_indirect;
        }
        

        if(rec.material->getScatterFlag() & Specular)
        {
            // HitRecord rRec;
            Ray rRay{rec.position,wr};
            L_indirect=trace(rRay,instances,lights,depth+1);
            return L_indirect;
        }

        float cos=wr.dot(rec.normal.v3);
        Ray rRay{rec.position,wr};
        L_indirect=rec.material->brdf(wo,wr,rec.normal.v3)*trace(rRay,instances,lights,depth+1)*cos*2*Pi*1.25f;
        return L_direct+L_indirect;
    }

float sumArea=-1.f;
void computeLightsArea(const std::vector<std::shared_ptr<Instance>>& lights)
{
    sumArea=0.f;
    for(auto& light:lights)
    {
        sumArea+=light->area();
    }
}

    void sampleLight(const std::vector<std::shared_ptr<Instance>>& lights, HitRecord& sampleRec)
    {
        if(sumArea<0) computeLightsArea(lights);
        float threshold=clamp(0.f,sumArea,getRand(0.f,1.f)*sumArea);
        float accum=0.f;
        for(auto& light:lights)
        {
            accum+=light->area();
            if(accum>=threshold)
            {
                light->sample(sampleRec);
                return;
            }
            // std::cout<<"asdf"<<std::endl;
            light->sample(sampleRec);
        }
    }
     
};
