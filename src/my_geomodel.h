#pragma once
#include"my_vulkanRHI.h"
#include"my_vertex.h"
#include"my_commandbuffer.h"
#include"my_sphereGenerator.h"
#include"my_model.h"


static const MyGeo::Vec3f noTextureUV{-1,-1,-1};


class GeoModel: public MyModel
{
public:
    struct 
    {
        uint32_t count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    } vertices;

    struct 
    {
        uint32_t count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    } indices;

    GeoModel(MyVulkanRHI* _rhi,const std::initializer_list<float>& scales, const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>()):MyModel{_rhi,_modelMat}
    {}

    void bind(MyCommandBuffer& mcmdbuffer, uint32_t binding=0);

    void draw(MyCommandBuffer& mcmdbuffer);


};

class RectModel: public GeoModel
{
public:
    RectModel(MyVulkanRHI* _rhi,const std::initializer_list<float>& scales, const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>()):GeoModel{_rhi,scales,_modelMat}
    {
        std::vector<MyVertex_Default> vbuffer;
        std::vector<uint32_t> ibuffer;
        vbuffer.resize(4);
        ibuffer.resize(6);
        auto p=scales.begin();
        float a=*p*0.5f;
        float b=*(p+1)*0.5f;
        int modelId=0;
        vbuffer={
            {
                {-a,0,b},
                {1,1,1},
                {0,1,0},
                {-1,-1,-1}
            },
            {
                {a,0,b},
                {1,1,1},
                {0,1,0},
                {-1,-1,-1}
            },
            {
                {a,0,-b},
                {1,1,1},
                {0,1,0},
                {-1,-1,-1}
            },
            {
                {-a,0,-b},
                {1,1,1},
                {0,1,0},
                {-1,-1,-1}
            }
            };
        ibuffer={0,1,2,
                0,2,3};

        rhi->createDataBuffer(vbuffer.data(),sizeof(vbuffer[0])*vbuffer.size(),vertices.buffer,vertices.memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        rhi->createDataBuffer(ibuffer.data(),sizeof(ibuffer[0])*ibuffer.size(),indices.buffer,indices.memory,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        vertices.count=vbuffer.size();
        indices.count=ibuffer.size();

        // for(const auto& v: vbuffer)
        // {
        //     rigidBody.positions.push_back(v.position);
        // }

    }
};

class BoxModel: public GeoModel 
{
public: 
    BoxModel(MyVulkanRHI* _rhi,const std::initializer_list<float>& scales, const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>()):GeoModel{_rhi,scales,_modelMat}
    {
        std::vector<MyVertex_Default> vbuffer;
        std::vector<uint32_t> ibuffer;
        vbuffer.resize(24);
        ibuffer.resize(24);
        
        vbuffer={
            {{-0.5,0.5,-0.5},{1,1,1},{0,1,0,},{-1,-1,-1}},
            {{-0.5,0.5,0.5},{1,1,1},{0,1,0,},{-1,-1,-1}},
            {{0.5,0.5,0.5},{1,1,1},{0,1,0,},{-1,-1,-1}},
            {{0.5,0.5,-0.5},{1,1,1},{0,1,0,},{-1,-1,-1}},
            {{-0.5,0.5,-0.5},{1,1,1},{-1,0,0,},{-1,-1,-1}},
            {{-0.5,-0.5,-0.5},{1,1,1},{-1,0,0,},{-1,-1,-1}},
            {{-0.5,-0.5,0.5},{1,1,1},{-1,0,0,},{-1,-1,-1}},
            {{-0.5,0.5,0.5},{1,1,1},{-1,0,0,},{-1,-1,-1}},
            {{0.5,0.5,0.5},{1,1,1},{1,0,0,},{-1,-1,-1}},
            {{0.5,-0.5,0.5},{1,1,1},{1,0,0,},{-1,-1,-1}},
            {{0.5,-0.5,-0.5},{1,1,1},{1,0,0,},{-1,-1,-1}},
            {{0.5,0.5,-0.5},{1,1,1},{1,0,0,},{-1,-1,-1}},
            {{0.5,0.5,-0.5},{1,1,1},{0,0,-1,},{-1,-1,-1}},
            {{0.5,-0.5,-0.5},{1,1,1},{0,0,-1,},{-1,-1,-1}},
            {{-0.5,-0.5,-0.5},{1,1,1},{0,0,-1,},{-1,-1,-1}},
            {{-0.5,0.5,-0.5},{1,1,1},{0,0,-1,},{-1,-1,-1}},
            {{-0.5,0.5,0.5},{1,1,1},{0,0,1,},{-1,-1,-1}},
            {{-0.5,-0.5,0.5},{1,1,1},{0,0,1,},{-1,-1,-1}},
            {{0.5,-0.5,0.5},{1,1,1},{0,0,1,},{-1,-1,-1}},
            {{0.5,0.5,0.5},{1,1,1},{0,0,1,},{-1,-1,-1}},
            {{-0.5,-0.5,0.5},{1,1,1},{0,-1,0,},{-1,-1,-1}},
            {{-0.5,-0.5,-0.5},{1,1,1},{0,-1,0,},{-1,-1,-1}},
            {{0.5,-0.5,-0.5},{1,1,1},{0,-1,0,},{-1,-1,-1}},
            {{0.5,-0.5,0.5},{1,1,1},{0,-1,0,},{-1,-1,-1}}
        };

        ibuffer={0,1,2,0,2,3,4,5,6,4,6,7,8,9,10,8,10,11,12,13,14,12,14,15,16,17,18,16,18,19,20,21,22,20,22,23};

        if(scales.size()>0)
        {
            for(auto& v: vbuffer)
            {
                for(auto i=0;i!=scales.size();++i)
                {
                    v.position[i]*=*(scales.begin()+i);
                }
            }
        }

        rhi->createDataBuffer(vbuffer.data(),sizeof(vbuffer[0])*vbuffer.size(),vertices.buffer,vertices.memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        rhi->createDataBuffer(ibuffer.data(),sizeof(ibuffer[0])*ibuffer.size(),indices.buffer,indices.memory,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        vertices.count=vbuffer.size();
        indices.count=ibuffer.size();

        // for(const auto& v: vbuffer)
        // {
        //     rigidBody.positions.push_back(v.position);
        // }
    }
};



class IcosSphereModel: public GeoModel 
{
public:

    IcosSphereModel(MyVulkanRHI* _rhi,const std::initializer_list<float>& scales, const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>()):GeoModel{_rhi,scales,_modelMat}
    {
        std::cout<<"init sphere"<<std::endl;
        std::vector<MyVertex_Default> vbuffer;
        std::vector<uint32_t> ibuffer;

        SimpleMesh mesh[2];
        Icosahedron(mesh[0],*scales.begin());
        static int n=6;
        SimpleMesh& mesh0=mesh[0];
        SimpleMesh& mesh1=mesh[1];
        for(int i=0;i!=n;++i)
        {
            SubdivideMesh(mesh0,mesh1);
            std::swap(mesh0,mesh1);
        }

        vbuffer.resize(mesh1.vertices.size());
        ibuffer.resize(mesh1.indices.size());

        for(auto i=0;i!=vbuffer.size();++i)
        {
            auto& meshv=mesh1.vertices[i];
            vbuffer[i].position=meshv.position.v3;
            vbuffer[i].color=meshv.color;
            vbuffer[i].normal=meshv.normal.v3;
            vbuffer[i].texCoord={-1,-1,-1};
        }
        for(auto i=0;i!=mesh1.indices.size();++i)
        {
            ibuffer[i]=mesh1.indices[i];
        }
        rhi->createDataBuffer(vbuffer.data(),sizeof(vbuffer[0])*vbuffer.size(),vertices.buffer,vertices.memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        rhi->createDataBuffer(ibuffer.data(),sizeof(ibuffer[0])*ibuffer.size(),indices.buffer,indices.memory,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        vertices.count=vbuffer.size();
        indices.count=ibuffer.size();
        std::cout<<vbuffer.size()<<" "<<ibuffer.size()<<std::endl;

        // for(const auto& v: vbuffer)
        // {
        //     rigidBody.positions.push_back(v.position);
        // }

    }
};






