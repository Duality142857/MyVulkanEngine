// #include<mygeo/mat.h>
#pragma once
#include"mat.h"

namespace MyGeo{

template<class T>
struct Quaternion
{
//data
    T w;
    union
    {
        MyGeo::Vec<T,3> v;
        struct 
        {
            T x,y,z;
        };
    };
    Quaternion(){}
    Quaternion(T _w,std::initializer_list<T> vil):w{_w},v{vil}{}
    Quaternion(T _w,const MyGeo::Vec<T,3>& _v):w{_w},v{_v}{}
    Quaternion(const Quaternion& q):w{q.w},v{q.v}{}
    Quaternion(std::initializer_list<T> il):w{*il.begin()},x{*(il.begin()+1)},y{*(il.begin()+2)},z{*(il.begin()+3)}{}

    //向量的四元数形式(0,v)
    Quaternion(const MyGeo::Vec<T,3>& _v):w{0},v{_v}{}

    //由旋转构造四元数，用户负责保证axis的归一性
    Quaternion(const MyGeo::Vec<T,3>& axis, T angle):w{std::cos(angle)},v{std::sin(angle)*axis}{}

    //!should be half rotation angle!
    void updateFromRotation(const MyGeo::Vec<T,3>& axis, T angle)
    {
        w=std::cos(angle);  
        v=std::sin(angle)*axis;
    }

    MyGeo::Mat<T,3,3> toMat3() const 
    {
        return MyGeo::Mat<T,3,3>{
            MyGeo::Vec<T,3>{1-2*(y*y+z*z),2*(x*y+w*z),2*(x*z-w*y)},
            {2*(x*y-w*z),1-2*(x*x+z*z),2*(w*x+y*z)},
            {2*(w*y+x*z),2*(y*z-w*x),1-2*(x*x+y*y)}
        };
    }

    MyGeo::Mat<T,4,4> toMat4() const
    {
        return MyGeo::Mat<T,4,4>{
            MyGeo::Vec<T,4>{1-2*(y*y+z*z),2*(x*y+w*z),2*(x*z-w*y),0},
            {2*(x*y-w*z),1-2*(x*x+z*z),2*(w*x+y*z),0},
            {2*(w*y+x*z),2*(y*z-w*x),1-2*(x*x+y*y),0},
            {0,0,0,1}
        };
    }

    Quaternion& operator=(const Quaternion& q)
    {
        w=q.w;
        v=q.v;
    }

    void operator+=(const Quaternion& other)
    {
        w+=other.w;
        v+=other.v;
    }

    Quaternion operator*(const Quaternion<T>& q2)
    {
        Quaternion qr;
        const Quaternion& q1=*this;
        qr.w=q1.w*q2.w-q1.v.dot(q2.v);
        qr.v=q1.w*q2.v+q2.w*q1.v+q1.v.cross(q2.v);
        return qr;
    }

    T norm2() const
    {
        return w*w+v.dot(v);
    }
    T norm1() const
    {
        return std::sqrt(norm2());
    }

    Quaternion<T> noramlQuaternion() const 
    {
        T n=norm1();
        return operator/(n);

    }

    Quaternion<T> operator*(T k) const 
    {
        return Quaternion<T>{k*w,k*v};
    }

    Quaternion<T> operator/(T k) const 
    {
        return operator*(1./k);
    }

    Quaternion<T> conjugate() const 
    {
        return {w,-v};
    }

    Quaternion<T> inverse() const 
    {
        return conjugate()/norm2();
    }

    friend Quaternion<T> operator*(T k,const Quaternion<T>& q)
    {
        return q*k;
    }


    friend std::ostream& operator<<(std::ostream& ostr,const Quaternion<T>& q)
    {
        ostr<<q.w<<" ("<<q.x<<','<<q.y<<','<<q.z<<')';
        return ostr;
    }

};


class Rotation
{
public:
    Quaternion<float> q;
    Rotation(const MyGeo::Vec3f axis, float angle):q{axis,angle*0.5f}{}
    const Quaternion<float>& quaternion() const 
    {
        return q;
    }
    MyGeo::Mat3f mat3() const 
    {
        return q.toMat3();
    }
    MyGeo::Mat4f mat4() const 
    {
        return q.toMat4();
    }
    MyGeo::Vec3f operator()(const MyGeo::Vec3f& v)
    {
        auto q_star=q.conjugate();
        Quaternion<float> p{0,v};
        return (q*p*q_star).v;
    }
};

}

