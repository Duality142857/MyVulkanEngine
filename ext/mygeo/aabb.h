#pragma once
#include"vec.h"
#include"ray.h"

template<class T, int N>
struct AABB
{
    MyGeo::Vec<T,N> min,max;
    AABB():min{(std::numeric_limits<T>::max(),N)},max{(std::numeric_limits<T>::min(),N)}
    {
    }
    AABB(const MyGeo::Vec<T,N>& min,const MyGeo::Vec<T,N>& max):min{min},max{max}{}

    AABB(const MyGeo::Vec<T,N>& r):min{r},max{r}{}

    AABB(const AABB& box):min{box.min},max{box.max}{}
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
        assert(N==3);
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
            auto recid=1.0f/ray.direction[i];
            auto t0=(min[i]-ray.source[i])*recid;
            auto t1=(max[i]-ray.source[i])*recid;
            //t0 being the earliest enter time of current direction, t1 being the latest
            if(recid<0) std::swap(t0,t1);

            if(t0>tmin) tmin=t0;
            if(t1<tmax) tmax=t1;
            if(tmin>=tmax) return false;
        }
        return true;
    }

    AABB Union(const AABB& b) const 
    {
        return AABB(MyGeo::mixMin(min,b.min),MyGeo::mixMax(max,b.max));
    }
    AABB Union(const MyGeo::Vec<T,N> r) const 
    {
        return Union(AABB{r});

    }
    
    bool intersectP(const AABB& b) const 
    {
        MyGeo::Vec<T,N> p=MyGeo::mixMax(min,b.min);
        MyGeo::Vec<T,N> q=MyGeo::mixMax(max,b.max);
        for(auto i=0;i!=N;++i)
        {
            if(p[i]>q[i]) return false;
        }
        return true;
    }

    AABB intersect(const AABB& b) const 
    {
        return AABB(MyGeo::mixMax(min,b.min),MyGeo::mixMin(max,b.max));
    }

    friend AABB Union(const AABB& a, const AABB& b)
    {
        return AABB(MyGeo::mixMin(a.min,b.min),MyGeo::mixMax(a.max,b.max));
    }
    friend AABB Union(const AABB& a, const MyGeo::Vec<T,N> r)
    {
        return Union(a,AABB{r});
    }

    friend AABB intersect(const AABB& a, const AABB& b)
    {
        return AABB(MyGeo::mixMax(a.min,b.min),MyGeo::mixMin(a.max,b.max));
    }
    friend std::ostream& operator<<(std::ostream& ostr, const AABB& box)
    {
        return ostr<<"bbox: "<<box.min<<" -> "<<box.max<<std::endl;
    }
};