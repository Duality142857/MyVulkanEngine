#pragma once
#include"vec.h"
#include"geoConstants.h"
#include"primitive.h"
#include"intersection2d.h"
// #include"obb.h"
#include"obb.h"

namespace MyGeo{

//project point on axis aligned plane, given a normal n, to which the plane is most perpendicular
static inline Vec3f projectAA(const Vec3f& v, const Vec3f& n)
{
    int k=getMainAxis(n);
    Vec3f res{v};
    res[k]=0;
    return res;
}

struct Intersection3D
{
    enum IntersectionType:int
    {
        Point=0,Line,Plane
    };
    IntersectionType type;
    Vec3f point;
    Line3D line;
};


class IntersectFuncs
{
public:
    bool line2plane(const Line3D& line, const Plane& plane, float& t, Vec3f& intersectionPoint)
    {
        float denom=line.direction.dot(plane.normal);
        //parallel
        if(std::fabs(denom)<epsilon)
        {
            //line on the plane
            if(std::fabs(-plane.d-line.source.dot(plane.normal))<epsilon)
            {
                t=0;
                return true;
            }
            //line off the plane
            else return false;
        }

        //non-parallel
        t=(-plane.d-plane.normal.dot(line.source))/denom;
        std::cout<<"dira "<<line.direction<<std::endl;
        intersectionPoint=line.source+t*line.direction;
        return true;
    }
    bool plane2plane(const Plane& p1, const Plane& p2, Line3D& line)
    {
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

    bool sphere2sphere(const Sphere& sph0, const Sphere& sph1)
    {
        if((sph0.center-sph1.center).norm2()<(sph0.r+sph1.r)*(sph0.r+sph1.r)) return true;
        return false;
    }

    bool triangle2plane(const Triangle3D& tri, const Plane& p1, Intersection3D& intersection)
    {
        float d[3];
        Vec3f pivot=p1.pivot();
        for(auto i=0;i!=3;++i)
        {
            d[i]=p1.normal.dot(tri.v[i]-pivot);
            if(std::fabs(d[i])<=epsilon) d[i]=0.f;
        }
        
        float d0d1=d[0]*d[1];
        float d0d2=d[0]*d[2];

        //not intersecting
        if(d0d1>0 && d0d2>0) return false;
        else if(d0d1<0 && d0d2<0) return false;

        //triangle on the plane
        if(std::fabs(d[0])+std::fabs(d[1])+std::fabs(d[2])==0)
        {
            intersection.type=Intersection3D::Plane;
            return true;
        }

        //intersection is a line segment
        if(
            d[0]>0 && d[1]>0 && d[2]<0 || 
            d[0]<0 && d[1]<0 && d[2]>0
        )
        {
            Line3D l1(tri.v[0],tri.v[2]-tri.v[0]);
            Line3D l2(tri.v[1],tri.v[2]-tri.v[1]);
            float t1,t2;
            Vec3f ip1,ip2;
            p1.intersectLine(l1,t1,ip1);
            p1.intersectLine(l2,t2,ip2);
            intersection.type=Intersection3D::IntersectionType::Line;
            intersection.line={ip1,ip2-ip1};
            return true;
        }

        if(
            d[1]>0 && d[2]>0 && d[0]<0 || 
            d[1]<0 && d[2]<0 && d[0]>0
        )
        {
            Line3D l1(tri.v[1],tri.v[0]-tri.v[1]);
            Line3D l2(tri.v[2],tri.v[0]-tri.v[2]);
            float t1,t2;
            Vec3f ip1,ip2;
            p1.intersectLine(l1,t1,ip1);
            p1.intersectLine(l2,t2,ip2);
            intersection.type=Intersection3D::IntersectionType::Line;
            intersection.line={ip1,ip2-ip1};
            return true;
        }

        if(
            d[0]>0 && d[2]>0 && d[1]<0 || 
            d[0]<0 && d[2]<0 && d[1]>0
        )
        {
            Line3D l1(tri.v[0],tri.v[1]-tri.v[0]);
            Line3D l2(tri.v[2],tri.v[1]-tri.v[2]);
            float t1,t2;
            Vec3f ip1,ip2;
            p1.intersectLine(l1,t1,ip1);
            p1.intersectLine(l2,t2,ip2);
            intersection.type=Intersection3D::IntersectionType::Line;
            intersection.line={ip1,ip2-ip1};
            return true;
        }

        //one vertex on the plane
        if(d[0]==0 && (d[1]>0 && d[2]>0 || d[1]<0 && d[2]<0))
        {
            intersection.type=Intersection3D::IntersectionType::Point;
            intersection.point=tri.v[0];
            return true;
        }
        if(d[1]==0 && (d[0]>0 && d[2]>0 || d[0]<0 && d[2]<0))
        {
            intersection.type=Intersection3D::IntersectionType::Point;
            intersection.point=tri.v[1];
            return true;
        }
        if(d[2]==0 && (d[0]>0 && d[1]>0 || d[0]<0 && d[1]<0))
        {
            intersection.type=Intersection3D::IntersectionType::Point;
            intersection.point=tri.v[2];
            return true;
        }

        //one vertex on the plane but the plane cuts the triangle in the middle
        if(d[0]==0 && (d[1]>0 && d[2]<0 || d[1]<0 && d[2]>0))
        {
            intersection.type=Intersection3D::IntersectionType::Line;
            Line3D l{tri.v[2],tri.v[1]-tri.v[2]};
            float t;
            Vec3f ip;
            p1.intersectLine(l,t,ip);
            intersection.line.source=tri.v[0];
            intersection.line.direction=ip-tri.v[0];
            return true;
        }

        if(d[1]==0 && (d[0]>0 && d[2]<0 || d[0]<0 && d[2]>0))
        {
            intersection.type=Intersection3D::IntersectionType::Line;
            Line3D l{tri.v[0],tri.v[2]-tri.v[0]};
            float t;
            Vec3f ip;
            p1.intersectLine(l,t,ip);
            intersection.line.source=tri.v[1];
            intersection.line.direction=ip-tri.v[1];
            return true;
        }

        if(d[2]==0 && (d[0]>0 && d[1]<0 || d[0]<0 && d[1]>0))
        {
            intersection.type=Intersection3D::IntersectionType::Line;
            Line3D l{tri.v[0],tri.v[1]-tri.v[0]};
            float t;
            Vec3f ip;
            p1.intersectLine(l,t,ip);
            intersection.line.source=tri.v[2];
            intersection.line.direction=ip-tri.v[2];
            return true;
        }

        //one edge on the plane
        if(d[0]==0 && d[1]==0)
        {
            intersection.type=Intersection3D::IntersectionType::Line;
            intersection.line={tri.v[0],tri.v[1]-tri.v[0]};
            return true;
        }
        if(d[1]==0 && d[2]==0)
        {
            intersection.type=Intersection3D::IntersectionType::Line;
            intersection.line={tri.v[1],tri.v[2]-tri.v[1]};
            return true;
        }
        if(d[0]==0 && d[2]==0)
        {
            intersection.type=Intersection3D::IntersectionType::Line;
            intersection.line={tri.v[0],tri.v[2]-tri.v[0]};
            return true;
        }


        return false;
    }

    bool line2triangle(const Line3D& line, const Triangle3D& tri, float& beta, float& gamma, float& t)
    {
        Vec3f e1=tri.v[1]-tri.v[0];
        Vec3f e2=tri.v[2]-tri.v[0];
        
        Vec3f s=line.source-tri.v[0];
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

    //!not complete yet!
    bool triangle2triangle(const Triangle3D& t0, const Triangle3D& t1, Intersection3D& intersection)
    {
        // const Triangle3D& t0=*this;
        // //*determine if triangles are degenerate
        // //*compute t0's plane equation
        // Vec3f e0[3]={t0.v[1]-t0.v[0],t0.v[2]-t0.v[1],t0.v[0]-t0.v[2]};
        // Vec3f n0=e0[0].cross(e0[1]).normalVec();
        // Plane p0{n0,t0.v[0]};//or v[1],v[2]

        // //*compute signed distances of vertices of t1 to t0,compare signs, if same return false
        // float dist1_0[3];
        // for(int i=0;i!=3;++i)
        // {
        //     dist1_0[i]=p0.distance(t1.v[i]);
        // }

        // if(dist1_0[0]>0 && dist1_0[1]>0 && dist1_0[2]>0 
        // || dist1_0[0]<0 && dist1_0[1]<0 && dist1_0[2]<0)
        // {
        //     return false;
        // }

        // //*compute t1's plane equation
        // Vec3f e1[3]={t1.v[1]-t1.v[0],t1.v[2]-t1.v[1],t1.v[0]-t1.v[2]};
        // Vec3f n1=e1[0].cross(e1[1]).normalVec();
        // Plane p1{n1,t1.v[0]};//or v[1],v[2]

        // //* p0==p1
        // if(std::fabs((p0.normal-p1.normal).norm2())<epsilon_sqr && std::fabs(p0.d-p1.d)<epsilon)
        // {
        //     //* project triangles onto the axis-aligned plane most nearly oriented with the triangles plane, perform 2D triangle intersection test
        //     int k=getMainAxis(p0.normal);
        //     Triangle2D t0_2d=degenerate2D(k);
        //     Triangle2D t1_2d=degenerate2D(k);
        //     return t0_2d.intersectTriangle2D_P(t1_2d);
        // }

        // //* p0!=p1
        // float dist0_1[3];
        // for(int i=0;i!=3;++i)
        // {
        //     dist0_1[i]=p1.distance(t0.v[i]);
        // }

        // if(dist0_1[0]>0 && dist0_1[1]>0 && dist0_1[2]>0 
        // || dist0_1[0]<0 && dist0_1[1]<0 && dist0_1[2]<0)
        // {
        //     return false;
        // }

        // //* compute intersection line of p0 and p1
        // //* compute intervals, if no overlap , return false
        // //* otherwise, if info is needed, compute it, or just return true;

        return true;
    }

    bool obb2triangle(const OBB& obb,const Triangle3D& tri)
    {

        return true;
    }


    //auxillary
    Triangle2D degenerate2D(const Triangle3D& tri, int axis) const
    {
        Triangle2D t;
        switch (std::abs(axis))
        {
            case 0:
            {
                t.v[0]={tri.v[0].y,tri.v[0].z};
                t.v[1]={tri.v[1].y,tri.v[1].z};
                t.v[2]={tri.v[2].y,tri.v[2].z};
                break;
            }
            case 1:
            {
                t.v[0]={tri.v[0].z,tri.v[0].x};
                t.v[1]={tri.v[1].z,tri.v[1].x};
                t.v[2]={tri.v[2].z,tri.v[2].x};
                break;
            }
            case 2:
            {
                t.v[0]={tri.v[0].x,tri.v[0].y};
                t.v[1]={tri.v[1].x,tri.v[1].y};
                t.v[2]={tri.v[2].x,tri.v[2].y};
                break;
            }
            default:
                break;
        }
        if(axis<0)
        {
            std::swap(t.v[1],t.v[2]);
        } 
        return t;
    }
    
};



}

