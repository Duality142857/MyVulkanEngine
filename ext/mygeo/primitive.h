#pragma once
#include"vec.h"
// #include"obb.h"
#include"geoConstants.h"
namespace MyGeo{

enum PrimType
{
LINE,PLANE,TRIANGLE,SPHERE,OBBox
};

class Primitive
{
public:
    Primitive(){}
    virtual PrimType type()=0;
};




class Line3D : public Primitive 
{
public:
    Vec3f source;
    Vec3f direction; 
    PrimType type() override
    {
        return PrimType::LINE;
    }
    Line3D(const Vec3f& _source, const Vec3f& _direction):source{_source},direction{_direction}
    {}
    float distance2Point_sqr(const Vec3f& p) const 
    {
        float t=direction.dot(p-source)/direction.norm2();
        Vec3f p_proj=source+t*direction;
        return (p-p_proj).norm2();
    }
    float distance2Point(const Vec3f& p) const 
    {
        return std::sqrt(distance2Point_sqr(p));
    }
};

class Plane: public Primitive
{
public:
    PrimType type() override
    {
        return PrimType::PLANE;
    }
    Plane(const Vec3f& _normal, float _d):normal{_normal.normalVec()},d{_d}
    {}
    Plane(const Vec3f& _normal, const Vec3f& pivot):normal{_normal.normalVec()},d{-pivot.dot(_normal)}
    {
        std::cout<<"normal "<<normal<<std::endl;
        std::cout<<"d "<<d<<std::endl;

    }

    Vec3f normal;
    float d;//distance from source to plane, also the d in ax+by+cz+d=0

    float distance(const MyGeo::Vec3f& r) const 
    {
        return (r-pivot()).dot(normal);
    }

    /**
     * @brief Plane x Plane
     * 
     * @param p2 target plane
     * @param line intersection line
     * @return true 
     * @return false 
     */
    bool intersectPlane(const Plane& p2, Line3D& line)
    {
        Plane& p1=*this;
        line.direction=p1.normal.cross(p2.normal);
        if(line.direction.norm2()<=epsilon_sqr) return false;

        float n1dotn2=p1.normal.dot(p2.normal);
        float n1norm2=p1.normal.norm2();
        float n2norm2=p2.normal.norm2();
        float denom=n1dotn2*n1dotn2-n1norm2*n2norm2;

        float a=-(p2.d*n1dotn2-p1.d*n2norm2)/denom;
        float b=-(p1.d*n1dotn2-p2.d*n2norm2)/denom;

        line.source=a*p1.normal+b*p2.normal;
        return true;
    }    

    //plane intersect line
    bool intersectLine(const Line3D& line, float&t, Vec3f& intersectionPoint) const 
    {
        float denom=line.direction.dot(normal);
        //parallel
        if(std::fabs(denom)<epsilon)
        {
            //line on the plane
            if(std::fabs(-d-line.source.dot(normal))<epsilon)
            {
                t=0;
                return true;
            }
            //line off the plane
            else return false;
        }

        //non-parallel
        t=(-d-normal.dot(line.source))/denom;
        std::cout<<"dira "<<line.direction<<std::endl;
        intersectionPoint=line.source+t*line.direction;
        return true;
    }

    Vec3f pivot() const 
    {
        return -d*normal;
    }

    bool contain(const Vec3f& p) const 
    {
        auto k=(pivot()-p).normalVec().dot(normal);
        if(std::fabs(k)<epsilon){return true;}
        return false;
    }
};

class Sphere: public Primitive
{
public:
    float r;
    Vec3f center;

    PrimType type() override
    {
        return PrimType::SPHERE;
    }
    Sphere(const Vec3f& _center, float _r):center{_center},r{_r}
    {}

    bool intersectSphere(const Sphere& sph1, float& deepness)
    {
        const Sphere& sph0=*this;
        deepness=(sph0.center-sph1.center).norm()-sph0.r-sph1.r;
        if(deepness>0) return false;
        // if((sph0.center-sph1.center).norm2()<(sph0.r+sph1.r)*(sph0.r+sph1.r)) return true;
        return true;
    }


    bool abovePlane(const Plane& plane, float& deepness, MyGeo::Vec3f& contactPoint) const 
    {
        deepness=plane.distance(center)-r;
        if(deepness<0)
        {
            contactPoint=center+deepness*plane.normal;
            return false;
        }
        return true;
        // return plane.distance(center)>r;
    }

};

class Triangle3D: public Primitive
{
public:
    Vec3f v[3];
    PrimType type() override
    {
        return PrimType::TRIANGLE;
    }
    Triangle3D(){}
    Triangle3D(const Vec3f& v0,const Vec3f& v1,const Vec3f& v2):v{v0,v1,v2}
    {}

    Vec3f edge(int i) const 
    {
        return v[(i+1)%3]-v[i];
    }

    /**
     * @brief Triangle3D x Plane
     * 
     * @param p1 target plane
     * @param intersection intersection info
     * @return true 
     * @return false 
     */
    // bool intersectPlane(const Plane p1, Intersection3D& intersection) const
    // {
    //     float d[3];
    //     Vec3f pivot=p1.pivot();
    //     for(auto i=0;i!=3;++i)
    //     {
    //         d[i]=p1.normal.dot(v[i]-pivot);
    //         if(std::fabs(d[i])<=epsilon) d[i]=0.f;
    //     }
        
    //     float d0d1=d[0]*d[1];
    //     float d0d2=d[0]*d[2];

    //     //not intersecting
    //     if(d0d1>0 && d0d2>0) return false;
    //     else if(d0d1<0 && d0d2<0) return false;

    //     //triangle on the plane
    //     if(std::fabs(d[0])+std::fabs(d[1])+std::fabs(d[2])==0)
    //     {
    //         intersection.type=Intersection3D::Plane;
    //         return true;
    //     }

    //     //intersection is a line segment
    //     if(
    //         d[0]>0 && d[1]>0 && d[2]<0 || 
    //         d[0]<0 && d[1]<0 && d[2]>0
    //     )
    //     {
    //         Line3D l1(v[0],v[2]-v[0]);
    //         Line3D l2(v[1],v[2]-v[1]);
    //         float t1,t2;
    //         Vec3f ip1,ip2;
    //         p1.intersectLine(l1,t1,ip1);
    //         p1.intersectLine(l2,t2,ip2);
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         intersection.line={ip1,ip2-ip1};
    //         return true;
    //     }

    //     if(
    //         d[1]>0 && d[2]>0 && d[0]<0 || 
    //         d[1]<0 && d[2]<0 && d[0]>0
    //     )
    //     {
    //         Line3D l1(v[1],v[0]-v[1]);
    //         Line3D l2(v[2],v[0]-v[2]);
    //         float t1,t2;
    //         Vec3f ip1,ip2;
    //         p1.intersectLine(l1,t1,ip1);
    //         p1.intersectLine(l2,t2,ip2);
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         intersection.line={ip1,ip2-ip1};
    //         return true;
    //     }

    //     if(
    //         d[0]>0 && d[2]>0 && d[1]<0 || 
    //         d[0]<0 && d[2]<0 && d[1]>0
    //     )
    //     {
    //         Line3D l1(v[0],v[1]-v[0]);
    //         Line3D l2(v[2],v[1]-v[2]);
    //         float t1,t2;
    //         Vec3f ip1,ip2;
    //         p1.intersectLine(l1,t1,ip1);
    //         p1.intersectLine(l2,t2,ip2);
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         intersection.line={ip1,ip2-ip1};
    //         return true;
    //     }

    //     //one vertex on the plane
    //     if(d[0]==0 && (d[1]>0 && d[2]>0 || d[1]<0 && d[2]<0))
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Point;
    //         intersection.point=v[0];
    //         return true;
    //     }
    //     if(d[1]==0 && (d[0]>0 && d[2]>0 || d[0]<0 && d[2]<0))
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Point;
    //         intersection.point=v[1];
    //         return true;
    //     }
    //     if(d[2]==0 && (d[0]>0 && d[1]>0 || d[0]<0 && d[1]<0))
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Point;
    //         intersection.point=v[2];
    //         return true;
    //     }

    //     //one vertex on the plane but the plane cuts the triangle in the middle
    //     if(d[0]==0 && (d[1]>0 && d[2]<0 || d[1]<0 && d[2]>0))
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         Line3D l{v[2],v[1]-v[2]};
    //         float t;
    //         Vec3f ip;
    //         p1.intersectLine(l,t,ip);
    //         intersection.line.source=v[0];
    //         intersection.line.direction=ip-v[0];
    //         return true;
    //     }

    //     if(d[1]==0 && (d[0]>0 && d[2]<0 || d[0]<0 && d[2]>0))
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         Line3D l{v[0],v[2]-v[0]};
    //         float t;
    //         Vec3f ip;
    //         p1.intersectLine(l,t,ip);
    //         intersection.line.source=v[1];
    //         intersection.line.direction=ip-v[1];
    //         return true;
    //     }

    //     if(d[2]==0 && (d[0]>0 && d[1]<0 || d[0]<0 && d[1]>0))
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         Line3D l{v[0],v[1]-v[0]};
    //         float t;
    //         Vec3f ip;
    //         p1.intersectLine(l,t,ip);
    //         intersection.line.source=v[2];
    //         intersection.line.direction=ip-v[2];
    //         return true;
    //     }

    //     //one edge on the plane
    //     if(d[0]==0 && d[1]==0)
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         intersection.line={v[0],v[1]-v[0]};
    //         return true;
    //     }
    //     if(d[1]==0 && d[2]==0)
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         intersection.line={v[1],v[2]-v[1]};
    //         return true;
    //     }
    //     if(d[0]==0 && d[2]==0)
    //     {
    //         intersection.type=Intersection3D::IntersectionType::Line;
    //         intersection.line={v[0],v[2]-v[0]};
    //         return true;
    //     }


    //     return false;
    // }

    /**
     * @brief Triangle3D x Line3D
     * 
     * @param line target line
     * @param beta baricentric coordinates
     * @param gamma 
     * @param t 
     * @return true 
     * @return false 
     */
    bool intersectLine(const Line3D& line, float& beta, float& gamma, float& t) const 
    {
        Vec3f e1=v[1]-v[0];
        Vec3f e2=v[2]-v[0];
        
        Vec3f s=line.source-v[0];
        Vec3f s1=line.direction.cross(e2);
        Vec3f s2=s.cross(e1);

        float denom=s1.dot(e1);
        if(std::fabs(denom)<epsilon) return false;
        float recidenom=1.f/denom;
        // float recidenom=1.f/s1.dot(e1);
        t=s2.dot(e2)*recidenom;
        beta=s1.dot(s)*recidenom;
        gamma=s2.dot(line.direction)*recidenom;
        if(beta>0 && gamma>0 && beta+gamma<1.f)
        {
            return true;
        }
        return false;
    }

    // bool intersectTriangle3D(const Triangle3D t1, Intersection3D& intersection) const
    // {
    //     const Triangle3D& t0=*this;
    //     //*determine if triangles are degenerate
    //     //*compute t0's plane equation
    //     Vec3f e0[3]={t0.v[1]-t0.v[0],t0.v[2]-t0.v[1],t0.v[0]-t0.v[2]};
    //     Vec3f n0=e0[0].cross(e0[1]).normalVec();
    //     Plane p0{n0,t0.v[0]};//or v[1],v[2]

    //     //*compute signed distances of vertices of t1 to t0,compare signs, if same return false
    //     float dist1_0[3];
    //     for(int i=0;i!=3;++i)
    //     {
    //         dist1_0[i]=p0.distance(t1.v[i]);
    //     }

    //     if(dist1_0[0]>0 && dist1_0[1]>0 && dist1_0[2]>0 
    //     || dist1_0[0]<0 && dist1_0[1]<0 && dist1_0[2]<0)
    //     {
    //         return false;
    //     }

    //     //*compute t1's plane equation
    //     Vec3f e1[3]={t1.v[1]-t1.v[0],t1.v[2]-t1.v[1],t1.v[0]-t1.v[2]};
    //     Vec3f n1=e1[0].cross(e1[1]).normalVec();
    //     Plane p1{n1,t1.v[0]};//or v[1],v[2]

    //     //* p0==p1
    //     if(std::fabs((p0.normal-p1.normal).norm2())<epsilon_sqr && std::fabs(p0.d-p1.d)<epsilon)
    //     {
    //         //* project triangles onto the axis-aligned plane most nearly oriented with the triangles plane, perform 2D triangle intersection test
    //         int k=getMainAxis(p0.normal);
    //         Triangle2D t0_2d=degenerate2D(k);
    //         Triangle2D t1_2d=degenerate2D(k);
    //         return t0_2d.intersectTriangle2D_P(t1_2d);
    //     }

    //     //* p0!=p1
    //     float dist0_1[3];
    //     for(int i=0;i!=3;++i)
    //     {
    //         dist0_1[i]=p1.distance(t0.v[i]);
    //     }

    //     if(dist0_1[0]>0 && dist0_1[1]>0 && dist0_1[2]>0 
    //     || dist0_1[0]<0 && dist0_1[1]<0 && dist0_1[2]<0)
    //     {
    //         return false;
    //     }

    //     //* compute intersection line of p0 and p1
    //     //* compute intervals, if no overlap , return false
    //     //* otherwise, if info is needed, compute it, or just return true;

    //     return true;
    // }
    
    // Triangle2D degenerate2D(int axis) const
    // {
    //     Triangle2D t;
    //     switch (std::abs(axis))
    //     {
    //         case 0:
    //         {
    //             t.v[0]={v[0].y,v[0].z};
    //             t.v[1]={v[1].y,v[1].z};
    //             t.v[2]={v[2].y,v[2].z};
    //             break;
    //         }
    //         case 1:
    //         {
    //             t.v[0]={v[0].z,v[0].x};
    //             t.v[1]={v[1].z,v[1].x};
    //             t.v[2]={v[2].z,v[2].x};
    //             break;
    //         }
    //         case 2:
    //         {
    //             t.v[0]={v[0].x,v[0].y};
    //             t.v[1]={v[1].x,v[1].y};
    //             t.v[2]={v[2].x,v[2].y};
    //             break;
    //         }
    //         default:
    //             break;
    //     }
    //     if(axis<0)
    //     {
    //         std::swap(t.v[1],t.v[2]);
    //     } 
    //     return t;
    // }

};


class Rect3D:public Primitive 
{
public:
    std::array<Triangle3D,2> trianglePair;
    Rect3D(const Vec3f& v0,const Vec3f& v1,const Vec3f& v2,const Vec3f& v3)
    :trianglePair{Triangle3D{v0,v1,v2},Triangle3D{v0,v2,v3}}
    {}

};




}