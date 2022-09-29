#include<filesystem>
#include<iostream>
#include<vector>
#include<mygeo/quaternion.h>
#include<mygeo/rigidbody.h>
#include<mygeo/mat.h>
#include<mygeo/geo.h>
#include<map>
#include<mygeo/component.h>
#include<source_location>
#include"src/bulletWrapper.h"
#include <stdio.h>
#include<mygeo/aabb.h>
#include<mygeo/intersection2d.h>
#include<mygeo/intersection3d.h>
#include"src/my_entity.h"
#include<mygeo/obb.h>

inline float deg2rad(float deg)
{
    return deg*3.14159/180;
}

void testQuaternion()
{
    Quaternion<float> q{1,1,1,1};
    Quaternion<float> d{std::cos(deg2rad(60/2)),std::sin(deg2rad(60/2))*MyGeo::Vec3f{0,0,1}};
    std::cout<<"d "<<d<<std::endl;
    std::cout<<"d normalized: "<<d.noramlQuaternion()<<std::endl;
    std::cout<<d.noramlQuaternion().toMat4()<<std::endl;

    MyGeo::Vec3f p{1,0,0};
    Rotation rotation{{0,1,0},deg2rad(90)};
    std::cout<<rotation(p)<<std::endl;
    std::cout<<rotation.mat4()<<std::endl;
}

void fileSystemTest()
{
    std::filesystem::path path1("../resources/Marry.obj");
    std::cout<<path1.root_path()<<std::endl;
    std::cout<<path1.root_name()<<std::endl;
    std::cout<<path1.relative_path()<<std::endl;
    std::cout<<path1.filename()<<std::endl;
    std::cout<<path1.stem()<<std::endl;
    std::cout<<path1.extension()<<std::endl;
    std::cout<<path1.native()<<std::endl;

    std::cout<<"asdfd"<<std::endl;

    std::filesystem::path resourcePath{"../resources"};
    std::filesystem::path marryPath=resourcePath/"Marry.obj";

    std::cout<<marryPath.relative_path()<<std::endl;

    std::filesystem::exists(resourcePath);
    std::filesystem::is_directory(resourcePath);
    for(auto& entry: std::filesystem::directory_iterator(resourcePath))
    {
        auto x=entry.path();
        std::cout<<x.filename().c_str()<<std::endl;
    }
}


void testRigid()
{
    MyGeo::RigidBody rb;
    rb.init();
    for(int i=0;i!=101;i++)
    {
        if(i%10==0) 
        {
            std::cout<<rb.position<<std::endl;
            std::cout<<"v: "<<rb.velocity<<std::endl;
        }
        rb.tick(0.1f);
    }
}



void testType(Entity _go)
{
    std::shared_ptr<Component> foo=std::make_shared<Foo>();
    std::shared_ptr<Component> bar=std::make_shared<Bar>();
    auto srcLoc=std::source_location::current();


    std::cout<<"line: "<<srcLoc.line()<<std::endl;
    std::cout<<"function: "<<srcLoc.function_name()<<std::endl;
    Entity go;
    go.addComponent(foo);
    go.addComponent(bar);
    go.addComponent(std::make_shared<Gun>());
    auto pgun=go.getComponent<Gun>();
    pgun->shot();   
    std::cout<<typeid(*pgun).name()<<std::endl;
}


void testBullet()
{
    MyDynamicsWorld world;
    world.addShape();
    world.run();
    world.cleanup();
    // world
}
void testDebug()
{
    auto srcLoc=std::source_location::current();

    std::cout<<"line: "<<srcLoc.line()<<std::endl;
    std::cout<<"function: "<<srcLoc.function_name()<<std::endl;

    Entity go;
    testType(go);
}

void testAABB()
{
    AABB a{MyGeo::Vec3f{1,1,1}};
    AABB b{MyGeo::Vec3f{2,2,2}};
    AABB c{MyGeo::Vec3f{0,0,0},MyGeo::Vec3f{1.2,1.3,1.1}};
    Ray ray{MyGeo::Vec3f{-1,-1,-1},MyGeo::Vec3f{0,0,1}};
    auto d=Union(Union(a,b),c);
    auto e=Union(a,b);
    AABB k{MyGeo::Vec3f{2.1,2,2},MyGeo::Vec3f{3,3,3}};
    std::cout<<"intersect: "<<k.intersectP(e)<<std::endl;

    auto f=e.intersect(c);
    std::cout<<f<<std::endl;
    std::cout<<d.hitP(ray)<<std::endl;
    std::cout<<e.Union(f)<<std::endl;
}

void testIntersection2d()
{
    MyGeo::Line2D l0{{0,0},{1,1}};
    MyGeo::Line2D l1{{1,1},{2,2}};
    MyGeo::Vec2f p;
    std::cout<<l0.intersect(l1,p)<<std::endl;
    std::cout<<p<<std::endl;

    MyGeo::LineSegment2D ls0{{0,0},{5,4}};
    MyGeo::LineSegment2D ls1{{1,0},{3,6}};
    MyGeo::Vec2f ip[2];
    auto n=ls0.intersect(ls1,ip);
    std::cout<<"intersectNUM: "<<n<<std::endl;
    for(auto i=0;i!=n;++i)
    {
        std::cout<<ip[i]<<std::endl;
    }

    MyGeo::Triangle2D t0{{-1,0},{0,0},{0,1}};
    MyGeo::Triangle2D t1{{0,-1},{1,0},{0,-0.0001}};
    std::cout<<"tt: "<<t0.intersectTriangle2D_P(t1)<<std::endl;

    MyGeo::Triangle3D t3{{1,0,0},{0,1,0},{0,0,1}};
    std::cout<<t3.degenerate2D(-1);

}

// void testIntersection3d()
// {
//     MyGeo::Plane p1{{0,0,1},-1};
//     MyGeo::Plane p2{{1,0,0},0};
//     MyGeo::Line3D line;
//     p1.intersectPlane(p2,line);
//     std::cout<<line.source<<"->"<<line.direction<<std::endl;

//     std::cout<<p1.contain({1,1,1})<<std::endl;
//     std::cout<<p1.contain({1,1,1.1})<<std::endl;

//     MyGeo::Line3D line1{{1,1,1},{0,1,0}};
//     std::cout<<line1.source<<std::endl;

//     float t;
//     MyGeo::Vec3f p;
//     auto flag=p1.intersectLine(line1,t,p);
//     std::cout<<"flag: "<<flag<<std::endl;
//     std::cout<<"t "<<t<<std::endl;
//     MyGeo::Plane p3{{0,0,1},{0,0,1}};
    
//     std::cout<<p3.distance({0,0,0})<<std::endl;

// }

// void testT()
// {
//     MyGeo::Triangle3D tri{{1,0,0},{0,1,0},{0,0,1}};
//     MyGeo::Line3D l{{0,0,0},{-0.1,1,-0.1}};
//     float beta, gamma, t;
//     bool hitflag=tri.intersectLine(l,beta,gamma,t);
//     std::cout<<"hit "<<hitflag<<std::endl;
//     std::cout<<beta<<' '<<gamma<<' '<<t<<std::endl;
//     std::cout<<l.source+t*l.direction<<std::endl;
//     MyGeo::Plane p{{0,1,0},1};
//     std::cout<<"pivot: "<<p.pivot()<<std::endl;
//     MyGeo::Intersection3D intersection;
//     hitflag=tri.intersectPlane(p,intersection);
//     std::vector<std::string> typeVec{"Point","Line","Plane"};
//     std::cout<<"hit: "<<hitflag<<std::endl;
//     std::cout<<typeVec[intersection.type]<<std::endl;
//     std::cout<<intersection.line.source<<"->"<<intersection.line.direction<<std::endl;

// }


// void testObb()
// {
//     MyGeo::OBB obb;
//     std::vector<MyGeo::Vec3f> points1{
// 		{-10,-10,-2},
// 		{-9,-10,-2},
// 		{10,10,-2},
// 		{-10,-10,2},
// 		{-9,-10,2},
// 		{10,10,2}
// 	};
//     std::vector<MyGeo::Vec3f> points{	
//         {-1, -1, 1},
// 		{1, -1, 1},
// 		{1, 1, 1},
// 		{-1, 1, 1},
// 		{-1, -1, -1},
// 		{1, -1, -1},
// 		{1, 1, -1},
// 		{-1, 1, -1},		
// 		{-0.5, -0.5, 1},
// 		{1, 0.5, -0.5},
// 		{0.5, 0.5, -1},
// 		// {-1, -0.5, 0.5},
//         // {-3,3,0.4},
//         // {-0.5,-4.0,3.0}
// 	};

//     std::vector<MyGeo::Vec3f> points_new(points.size());
//     for(int i=0;i!=points.size();++i)
//     {
//         points_new[i]=points[i]+MyGeo::Vec3f{0,5,0};
//     }
//     obb.gen(points);
//     MyGeo::OBB obb_new;
//     obb_new.gen(points_new);
//     std::cout<<"if intersect: "<<obb.intersectOBB(obb_new)<<std::endl;
//     MyGeo::Plane plane0{{0,0,1},-4};
//     std::cout<<plane0.pivot()<<std::endl;
//     std::cout<<"intersecting plane: "<<obb.abovePlane(plane0)<<std::endl;
//     MyGeo::Sphere sph{{1.5,0,0},0.5};
//     MyGeo::Collision collision;
//     // obb.intersectSphere(sph,collision);
//     obb.intersectSphere(sph,collision);

// }

void testnew()
{
    MyGeo::Triangle3D tri{{1,0,0},{0,1,0},{0,0,1}};
    MyGeo::Plane p{{0,1,0},-0.5};
    MyGeo::IntersectFuncs ifuncs;
    MyGeo::Intersection3D intersection;
    ifuncs.triangle2plane(tri,p,intersection);
    std::cout<<intersection.line.direction<<std::endl;
    std::cout<<intersection.type<<std::endl;

}

int main()
{
    // testBullet();
    // testAABB();
    // testT();
    // testIntersection2d();
    // testObb();
    testnew();
    

    // fileSystemTest();return 0;
    // testQuaternion();
    // testRigid();

}




