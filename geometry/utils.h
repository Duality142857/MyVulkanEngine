#pragma once
#include<optional>

static constexpr float Pi=3.14159265f;
static constexpr float ReciPi=1.f/Pi;

static constexpr float toRad(float x)
{
    return x*3.1415927/180.0;
}

static std::optional<std::tuple<float,float>> solveQuadratic(const float& a, const float& b, const float& c)
{
    float d=b*b-4*a*c;
    if(d<0) return std::nullopt;
    float recidenom=0.5f/a;
    float m=-b*recidenom;
    float n=std::sqrt(d)*recidenom;
    float x1=m-n,x2=m+n;
    if(x1>x2) std::swap(x1,x2);
    return std::make_optional(std::make_tuple(x1,x2));
}
template <typename T, typename U, typename V>
inline T clamp(T low, U high, V val) 
{
    if (val < low)
        return low;
    else if (val > high)
        return high;
    else
        return val;
}

template<class T>
T lerp(float t, const T& x0, const T& x1)
{
    return t*x0+(1-t)*x1;
}

template <typename Predicate>
int findInterval(int size, const Predicate &pred) 
{
    int first = 0, len = size;
    while (len > 0) 
    {
        int half = len >> 1, middle = first + half;
        // Bisect range based on value of _pred_ at _middle_
        if (pred(middle)) 
        {
            first = middle + 1;
            len -= half + 1;
        } 
        else
            len = half;
    }
    
    return clamp(first - 1, 0, size - 2);
}

static inline MyGeo::Vec3f reflect(const MyGeo::Vec3f& l, const MyGeo::Vec3f& n)
{
    return n*l.dot(n)*2.f-l;
}

static MyGeo::Vec3f refract(const MyGeo::Vec3f& l, const MyGeo::Vec3f& N, const float& ior1,const float& ior2)
{
    auto n=N;
    float cos=l.dot(n);
    float etai=ior1,etat=ior2;
    if(cos<0) cos=-cos;
    else {std::swap(etai,etat);n=-n;}
    float eta=etai/etat;
    float k=1-eta*eta*(1-cos*cos);
    return k<0?MyGeo::Vec3f{0,0,0}:l*eta+n*(eta*cos-sqrtf(k));
}

static float fresnel(const MyGeo::Vec3f& l, const MyGeo::Vec3f& N,const float& ior1,const float& ior2)
{
    float cosi=l.dot(N);
    float etai=ior1,etat=ior2;
    if(cosi>0)
    {
        std::swap(etai,etat);
    }
    float sint=etai/etat*sqrtf(std::max(0.f,1-cosi*cosi));

    if(sint>=1) return 1.0f;
    float cost=sqrtf(std::max(0.f,1-sint*sint));
    cosi=fabsf(cosi);
    float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
    float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
    return (Rs * Rs + Rp * Rp) / 2;
}