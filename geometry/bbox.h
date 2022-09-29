#pragma once
#include<mygeo/vec.h>
#include<limits>
#include"ray.h"

template<class T, int N>
struct BBox
{
    MyGeo::Vec<T,N> min,max;
    BBox():min{(std::numeric_limits<T>::max(),N)},max{(std::numeric_limits<T>::min(),N)}
    {
    }
    BBox(const MyGeo::Vec<T,N>& min,const MyGeo::Vec<T,N>& max):min{min},max{max}{}

    BBox(const MyGeo::Vec<T,N>& r):min{r},max{r}{}

    BBox(const BBox& box):min{box.min},max{box.max}{}
    
    MyGeo::Vec<T,N> diagonal() const
    {
        return max-min;
    }

    MyGeo::Vec<T,N> offset(const MyGeo::Vec<T,N>& p) const 
    {
        return (p-min)/diagonal();
    }

    T area() const 
    {
        MyGeo::Vec<T,N> d=diagonal();
        return 2*(d.x*d.y+d.x*d.z+d.y*d.z);
    }

    
    int mainAxis()
    {
        MyGeo::Vec<T,N> diag=diagonal();
        int k=0;
        if(N==1) return 0;
        if(diag.y>diag.x)
        {
            k=1;
            if(N==2) return k;
            if(diag.z>diag.y) k=2;
        }
        else if(diag.z>diag.x) k=2;
        return k;
    }

    bool hitP(const Ray& ray) const
    {
        float tmin=0,tmax=std::numeric_limits<float>::max();
        for(int i=0;i!=N;++i)
        {
            auto recid=1.0f/ray.direction.v3[i];
            auto t0=(min[i]-ray.source.v3[i])*recid;
            auto t1=(max[i]-ray.source.v3[i])*recid;

            if(recid<0) std::swap(t0,t1);

            if(t0>tmin) tmin=t0;
            if(t1<tmax) tmax=t1;
            if(tmin>=tmax) return false;
        }
        return true;
    }
    

    friend BBox Union(const BBox& a, const BBox& b)
    {
        return BBox(MyGeo::mixMin(a.min,b.min),MyGeo::mixMax(a.max,b.max));
    }

    friend BBox Union(const BBox& a, const MyGeo::Vec<T,N> r)
    {
        return Union(a,BBox{r});
    }

    friend BBox Intersect(const BBox& a, const BBox& b)
    {
        return BBox(MyGeo::mixMax(a,b),MyGeo::mixMin(a,b));
    }

    friend std::ostream& operator<<(std::ostream& ostr, const BBox& box)
    {
        return ostr<<"bbox: "<<box.min<<" -> "<<box.max<<std::endl;
    }
};

using BB3f=BBox<float,3>;
using BB2f=BBox<float,2>;
