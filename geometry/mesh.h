#pragma once
#include<mygeo/vec.h>
#include"space.h"
#include"../core/object.h"
#include"shapes.h"
#include<vector>
#include<algorithm>
#include"../core/myrandom.h"
struct Mesh : Shape
{
    
};

struct Triangle;

struct TriangleMesh: public Shape
{
    int numTriangles,numVertices;
    float area=0;
    std::vector<int> indices;
    std::vector<Point> positions;
    std::vector<Normal> normals;
    std::vector<MyGeo::Vec2f> uvs;
    // std::vector<Triangle> triangles;
    Triangle* triangles;
    TriangleMesh(){}

    TriangleMesh(std::vector<int> indices,std::vector<Point> p,std::vector<Normal> n,std::vector<MyGeo::Vec2f> uv):
    indices{indices},positions{p},normals{n},uvs{uv},numVertices{(int)indices.size()},numTriangles{(int)indices.size()/3} {}

    // virtual float area() const override
    // {
    //     if(area!=0) return area;
    //     for(int i=0;i!=numTriangles;++i)
    //     {
            
    //         // area+=
    //     }
    //     return area;
    // }


    virtual bool hit(const Ray& ray, HitRecord& rec) const override;
    // {
    //     bool flag=false;
    //     for(int i=0;i!=numTriangles;++i)
    //     {
    //         Triangle triangle{*this,indices.data()+i*3};
    //         flag=triangle.hit(ray,rec);
    //     }
    //     return flag;
    // }
    
};


struct Triangle: public Shape
{
    const TriangleMesh& mesh;
    const int* indices;
    Normal normal;//todo


    Triangle(const TriangleMesh& mesh, const int* indices)
    : mesh{mesh},indices{indices}//,normal{mesh.positions[indices[0]].v3.cross(mesh.positions[indices[1]].v3)} 
    {
        const MyGeo::Vec3f& v0=mesh.positions[indices[0]].v3;
        const MyGeo::Vec3f& v1=mesh.positions[indices[1]].v3;
        const MyGeo::Vec3f& v2=mesh.positions[indices[2]].v3;
        normal={(v1-v0).cross(v2-v1).normalize()};
    }

    Triangle(const TriangleMesh& mesh, const int* indices,const Normal& normal)
    : mesh{mesh},indices{indices},normal{normal} 
    {}

    virtual BB3f bound() const override
    {
        const MyGeo::Vec3f& v0=mesh.positions[indices[0]].v3;
        const MyGeo::Vec3f& v1=mesh.positions[indices[1]].v3;
        const MyGeo::Vec3f& v2=mesh.positions[indices[2]].v3;
        return BB3f{
            MyGeo::Vec3f{std::min({v0.x,v1.x,v2.x}),std::min({v0.y,v1.y,v2.y}),std::min({v0.z,v1.z,v2.z})},
            MyGeo::Vec3f{std::max({v0.x,v1.x,v2.x}),std::max({v0.y,v1.y,v2.y}),std::max({v0.z,v1.z,v2.z})}
        };
    }
    
    virtual void sample(HitRecord& rec) const override
    {
        float x=getRand(0.f,1.f);
        float y=getRand(0.f,1.f);
        float alpha=1.f-x;
        float beta=x-x*y;
        float gamma=1.f-alpha-beta;
        rec.position=mesh.positions[indices[0]].v3*alpha+mesh.positions[indices[1]].v3*beta+mesh.positions[indices[2]].v3*gamma;
        rec.normal=normal;
        rec.area=area();
    }

    virtual float area() const override
    {
        const MyGeo::Vec3f& v0=mesh.positions[indices[0]].v3;
        const MyGeo::Vec3f& v1=mesh.positions[indices[1]].v3;
        const MyGeo::Vec3f& v2=mesh.positions[indices[2]].v3;

        MyGeo::Vec3f e1=v1-v0;
        MyGeo::Vec3f e2=v2-v0;
        return e1.cross(e2).norm();
    }

    virtual bool hit(const Ray& ray, HitRecord& rec) const override
    {
        const MyGeo::Vec3f& v0=mesh.positions[indices[0]].v3;
        const MyGeo::Vec3f& v1=mesh.positions[indices[1]].v3;
        const MyGeo::Vec3f& v2=mesh.positions[indices[2]].v3;

        MyGeo::Vec3f e1=v1-v0;
        MyGeo::Vec3f e2=v2-v0;
        MyGeo::Vec3f s=ray.source.v3-v0;
        MyGeo::Vec3f s1=ray.direction.v3.cross(e2);
        MyGeo::Vec3f s2=s.cross(e1);
        auto recidenom=1.0f/s1.dot(e1);
        auto t=s2.dot(e2)*recidenom;
        auto beta=s1.dot(s)*recidenom;
        auto gamma=s2.dot(ray.direction.v3)*recidenom;
        if(t>rec.tmin && t<rec.t && beta>0 && gamma>0 && beta+gamma<1.0f)
        {
            rec.t=t;
            rec.position=ray.at(t);
            rec.normal=normal;
            // rec.fixNormal(ray.direction.v3);
            // std::cout<<"hit!"<<std::endl;
            return true;
        }

        return false;
    }   
    
};

template<class T>
static void vecAppend(std::vector<T>& v,std::initializer_list<T> il)
{
    for(const auto& x:il)
    {
        v.emplace_back(x);
    }
}



struct Rect: TriangleMesh
{
    std::vector<Triangle> trianglesVec;
    float areaVal=0;
    Normal normal;
    Point r0;
    Vect right;
    Vect up;
    Rect(const Point& r0, const Vect& right, const Vect& up):r0{r0},right{right},up{up},areaVal{right.cross(up).v3.norm()},normal{{right.cross(up).v3.normalize()}}
    {
        int ind=0;
        // auto n=Normal{right.cross(up).normalize().v3};
        positions.emplace_back(r0);
        positions.emplace_back(r0+right);        
        positions.emplace_back(r0+up+right);
        positions.emplace_back(r0+up);

        vecAppend(normals,{normal,normal,normal,normal});
        vecAppend(indices,{0,1,2});
        vecAppend(indices,{0,2,3});
        for(int i=0;i!=indices.size();i+=3)
        {
            trianglesVec.emplace_back(Triangle{*this,indices.data()+i});
        }
        triangles=trianglesVec.data();

    }
    MyGeo::Vec3f getCenter() const 
    {
        return 0.5*(positions[0].v3+positions[2].v3);
    }
    virtual bool hit(const Ray& ray, HitRecord& rec) const override
    {
        bool hitflag=false;
        for(auto const& triangle:trianglesVec)
        {
            if(triangle.hit(ray,rec)==true)
            {
                hitflag=true;
            }
        }
        return hitflag;
    }
    BB3f bound() const override 
    {
        return Union(triangles[0].bound(),triangles[1].bound());
    }
    float area() const override 
    {
        return areaVal;
    }
    void sample(HitRecord& rec) const override
    {
        float kx= getRand(0.f,1.f);
        float ky=getRand(0.f,1.f);
        rec.position=r0+kx*right+ky*up;
        rec.normal=normal;
        rec.area=area();
    }
};


struct Cube: TriangleMesh
{
    std::vector<Triangle> trianglesVec;
    MyGeo::Vec3f scale;

    virtual bool hit(const Ray& ray, HitRecord& rec) const override
    {
        if(!bound().hitP(ray)) return false;

        bool hitflag=false;
        for(auto const& triangle:trianglesVec)
        {
            if(triangle.hit(ray,rec)==true)
            {
                hitflag=true;
            }
        }
        return hitflag;
    }

    BB3f bound() const override 
    {
        return BB3f{MyGeo::Vec3f{-scale.x,-scale.y,-scale.z},MyGeo::Vec3f{scale.x,scale.y,scale.z}};
    }

    float area() const override
    {
        return 2*(scale.x*scale.y+scale.x*scale.z+scale.y*scale.z);
    }

    void sample(HitRecord& rec) const override
    {
        
    }

    Cube(const MyGeo::Vec3f& _scale):scale{0.5*_scale}
    {
        int curstart=0;
        //x=-1,1 faces
        for(int k=-1;k!=3;k+=2)
        {
            MyGeo::Vec3f n={(float)k,0,0};
            for(int i=-1;i!=3;i+=2)
            for(int j=-1;j!=3;j+=2)
            {
                positions.emplace_back(Point{(float)k*scale.x,(float)i*scale.y,(float)j*scale.z});
                normals.emplace_back(Normal{n});
            }
        }
        vecAppend<int>(indices,{curstart,curstart+1,curstart+2});//12为公共边，所以刚好反过来
        vecAppend<int>(indices,{curstart+2,curstart+1,curstart+3});
        curstart+=4;
        vecAppend<int>(indices,{curstart,curstart+2,curstart+1});//12为公共边，所以刚好反过来
        vecAppend<int>(indices,{curstart+1,curstart+2,curstart+3});
        curstart+=4;
        //y=-1,1 faces
        for(int k=-1;k!=3;k+=2)
        {
            MyGeo::Vec3f n={0,(float)k,0};
            for(int i=-1;i!=3;i+=2)
            for(int j=-1;j!=3;j+=2)
            {
                // vertices.emplace_back(VertexNT{MyGeo::Vec3f{(float)i*scale.x,(float)k*scale.y,(float)j*scale.z},n,0,0});
                positions.emplace_back(Point{(float)j*scale.x,(float)k*scale.y,(float)i*scale.z});
                normals.emplace_back(Normal{n});
            }
        }
        vecAppend<int>(indices,{curstart,curstart+1,curstart+2});//12为公共边，所以刚好反过来
        vecAppend<int>(indices,{curstart+2,curstart+1,curstart+3});
        curstart+=4;
        vecAppend<int>(indices,{curstart,curstart+2,curstart+1});//12为公共边，所以刚好反过来
        vecAppend<int>(indices,{curstart+1,curstart+2,curstart+3});
        curstart+=4;

        //z=-1,1 faces
        for(int k=-1;k!=3;k+=2)
        {
            MyGeo::Vec3f n={0,0,(float)k};
            for(int i=-1;i!=3;i+=2)
            for(int j=-1;j!=3;j+=2)
            {
                // vertices.emplace_back(VertexNT{MyGeo::Vec3f{(float)i*scale.x,(float)j*scale.y,(float)k*scale.z},n,0,0});
                positions.emplace_back(Point{(float)i*scale.x,(float)j*scale.y,(float)k*scale.z});
                normals.emplace_back(Normal{n});
            }
        }
        vecAppend<int>(indices,{curstart,curstart+1,curstart+2});//12为公共边，所以刚好反过来
        vecAppend<int>(indices,{curstart+2,curstart+1,curstart+3});
        curstart+=4;
        vecAppend<int>(indices,{curstart,curstart+2,curstart+1});//12为公共边，所以刚好反过来
        vecAppend<int>(indices,{curstart+1,curstart+2,curstart+3});
        curstart+=4;
        std::cout<<"numtriangles: "<<indices.size()/3<<std::endl;
        for(int i=0;i!=indices.size();i+=3)
        {
            //Triangle(const TriangleMesh& mesh, const int* indices)
            trianglesVec.emplace_back(Triangle{*this,indices.data()+i});
            // triangles.push_back({vertices[indices[i]].position,vertices[indices[i+1]].position,vertices[indices[i+2]].position,vertices[indices[i]].normal,material});
            
        }
        triangles=trianglesVec.data();
    }

    void print()
    {
        for(auto& p:positions)
        {
            std::cout<<p;
        }

        for(int i=0;i!=indices.size()/3;++i)
        {
            std::cout<<indices[3*i]<<','<<indices[3*i+1]<<','<<indices[3*i+2]<<std::endl;
        }
    }


};


