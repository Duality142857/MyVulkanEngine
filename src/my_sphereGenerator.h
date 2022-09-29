#pragma once
// #include<array>
// #include<vector>
// #include<map>
// #include<set>
// #include<iostream>
#include<mygeo/vec.h>
#include"../geometry/transforms.h"


struct Vertex_PNC
{
    Vertex_PNC(const MyGeo::Vec3f& v):position{v}{}
    Point position;
    Normal normal;
    MyGeo::Vec3f color{1,1,1};
};

struct SimpleMesh
{
    std::vector<Vertex_PNC> vertices;
    std::vector<uint32_t> indices;
    void addTriangle(uint32_t i, uint32_t j, uint32_t k)
    {
		indices.emplace_back(i);
		indices.emplace_back(j);
		indices.emplace_back(k);
    }
    friend std::ostream& operator<<(std::ostream& ostr,const SimpleMesh& mesh)
    {
        for(auto const& v:mesh.vertices)
        {
            ostr<<v.position.v3<<"  "<<v.normal<<std::endl;
        }
        ostr<<std::endl;
        return ostr;
    }
};

struct SimpleEdge
{
	uint32_t v0;
	uint32_t v1;

	SimpleEdge(uint32_t v0, uint32_t v1)
		: v0(v0 < v1 ? v0 : v1)
		, v1(v0 < v1 ? v1 : v0)
	{
	}

	bool operator <(const SimpleEdge &rhs) const
	{
		return v0 < rhs.v0 || (v0 == rhs.v0 && v1 < rhs.v1);
	}
};

static void Icosahedron(SimpleMesh& mesh, float scale)
{
    const float t=(1.f+std::sqrt(5.f))*0.5f;
    float reciL=1.f/std::sqrt(1+t*t);
    float a=1.f*reciL*scale;
    float b=t*reciL*scale;
    mesh.vertices.reserve(12);
	mesh.vertices.emplace_back(Vertex_PNC{MyGeo::Vec3f{-a,  b, 0.f}});
	mesh.vertices.emplace_back(MyGeo::Vec3f{ a,  b, 0.f});
	mesh.vertices.emplace_back(MyGeo::Vec3f{-a, -b, 0.f});
	mesh.vertices.emplace_back(MyGeo::Vec3f{ a, -b, 0.f});
	mesh.vertices.emplace_back(MyGeo::Vec3f{0.f, -a,  b});
	mesh.vertices.emplace_back(MyGeo::Vec3f{0.f,  a,  b});
	mesh.vertices.emplace_back(MyGeo::Vec3f{0.f, -a, -b});
	mesh.vertices.emplace_back(MyGeo::Vec3f{0.f,  a, -b});
	mesh.vertices.emplace_back(MyGeo::Vec3f{ b, 0.f, -a});
	mesh.vertices.emplace_back(MyGeo::Vec3f{ b, 0.f,  a});
	mesh.vertices.emplace_back(MyGeo::Vec3f{-b, 0.f, -a});
	mesh.vertices.emplace_back(MyGeo::Vec3f{-b, 0.f,  a});

	mesh.addTriangle(0, 11, 5);
	mesh.addTriangle(0, 5, 1);
	mesh.addTriangle(0, 1, 7);
	mesh.addTriangle(0, 7, 10);
	mesh.addTriangle(0, 10, 11);
	mesh.addTriangle(1, 5, 9);
	mesh.addTriangle(5, 11, 4);
	mesh.addTriangle(11, 10, 2);
	mesh.addTriangle(10, 7, 6);
	mesh.addTriangle(7, 1, 8);
	mesh.addTriangle(3, 9, 4);
	mesh.addTriangle(3, 4, 2);
	mesh.addTriangle(3, 2, 6);
	mesh.addTriangle(3, 6, 8);
	mesh.addTriangle(3, 8, 9);
	mesh.addTriangle(4, 9, 5);
	mesh.addTriangle(2, 4, 11);
	mesh.addTriangle(6, 2, 10);
	mesh.addTriangle(8, 6, 7);
	mesh.addTriangle(9, 8, 1);

    //generate normals for vertices, from center to position
    for(auto& v:mesh.vertices)
    {
        v.normal={v.position.v3.normalVec()};
    }

}

static uint32_t subdivideEdge(uint32_t f0, uint32_t f1, const MyGeo::Vec3f &v0, const MyGeo::Vec3f &v1, SimpleMesh &io_mesh, std::map<SimpleEdge, uint32_t> &io_divisions)
{
	const SimpleEdge edge(f0, f1);
	auto it = io_divisions.find(edge);
	if (it != io_divisions.end())
	{
		return it->second;
	}

	const MyGeo::Vec3f v = (0.5 * (v0 + v1)).normalVec()*v0.norm();
	const uint32_t f = io_mesh.vertices.size();
	io_mesh.vertices.emplace_back(v);
	io_divisions.emplace(edge, f);
	return f;
}

static void SubdivideMesh(const SimpleMesh &meshIn, SimpleMesh &meshOut)
{
	meshOut.vertices = meshIn.vertices;

	std::map<SimpleEdge, uint32_t> divisions; // Edge -> new vertex

	for (uint32_t i = 0; i < meshIn.indices.size()/3; ++i)
	{
		const uint32_t f0 = meshIn.indices[i * 3];
		const uint32_t f1 = meshIn.indices[i * 3 + 1];
		const uint32_t f2 = meshIn.indices[i * 3 + 2];

		const MyGeo::Vec3f v0 = meshIn.vertices[f0].position.v3;
		const MyGeo::Vec3f v1 = meshIn.vertices[f1].position.v3;
		const MyGeo::Vec3f v2 = meshIn.vertices[f2].position.v3;

		const uint32_t f3 = subdivideEdge(f0, f1, v0, v1, meshOut, divisions);
		const uint32_t f4 = subdivideEdge(f1, f2, v1, v2, meshOut, divisions);
		const uint32_t f5 = subdivideEdge(f2, f0, v2, v0, meshOut, divisions);

		meshOut.addTriangle(f0, f3, f5);
		meshOut.addTriangle(f3, f1, f4);
		meshOut.addTriangle(f4, f2, f5);
		meshOut.addTriangle(f3, f4, f5);
	}
    for(auto& v:meshOut.vertices)
    {
        v.normal={v.position.v3.normalVec()};
    }
}
