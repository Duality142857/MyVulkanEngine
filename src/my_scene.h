#pragma once
#include"my_model.h"
#include"my_geomodel.h"
#include"my_vulkanUtil.h"
#include"my_entity.h"
class MyScene 
{
    MyVulkanRHI* rhi;
    MyTexturePool* texturePool;
public:
    MyScene(MyVulkanRHI* _rhi , MyTexturePool* _texturePool):rhi{_rhi},texturePool{_texturePool}{}
    std::vector<std::shared_ptr<MyModel>> models;
    // std::vector<std::shared_ptr<MyEntity>> entities;

    std::vector<MyGeo::Mat4f> modelMats;

    void addExtModel(const std::string& modelpath,const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>())
    {
        models.push_back(std::make_shared<MyExtModel>(modelpath,rhi,texturePool,_modelMat));
        models[models.size()-1]->modelId=models.size()-1;
        // modelMats.push_back(_modelMat);
    }

    void addBox(const std::initializer_list<float>& scales,const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>())
    {
        models.push_back(std::make_shared<BoxModel>(rhi,scales,_modelMat));
        models[models.size()-1]->modelId=models.size()-1;
        // modelMats.push_back(_modelMat);
    }

    void addSphere(const std::initializer_list<float>& scales,const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>())
    {
        models.push_back(std::make_shared<IcosSphereModel>(rhi,scales,_modelMat));
        models[models.size()-1]->modelId=models.size()-1;
        // modelMats.push_back(_modelMat);
    }

    void addRect(const std::initializer_list<float>& scales,const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>())
    {
        models.push_back(std::make_shared<RectModel>(rhi,scales,_modelMat));
        models[models.size()-1]->modelId=models.size()-1;
        // modelMats.push_back(_modelMat);
    }
    // void addGeoModel(std::shared_ptr<MyGeoModel>())

    void tick(float t_delta, float t_elapsed)
    {
        for(auto& model:models)
        {
            model->tick(t_delta, t_elapsed);
        }
    }

    void draw(MyCommandBuffer& myCommandbuffer, VkPipelineLayout pipelineLayout)
    {
        for(auto& model:models)
        {
            vkCmdPushConstants(
                myCommandbuffer.commandbuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(PushConstantStruct),
                model->modelMat.col[0].data.data()
                );
            model->bind(myCommandbuffer);
            model->draw(myCommandbuffer);
        }
    }

};


// //drawvertices, drawindexed; different topologies

// class RenderObject 
// {
// public:
//     uint32_t firstIndex;
//     uint32_t indexCount;
//     uint32_t materialIndex;
    
// };

// struct UBO_Camera
// {
//     MyGeo::Mat4f view;
//     MyGeo::Mat4f proj;
// };

// struct UBO_model 
// {
//     MyGeo::Mat4f model;
// };

// class Material;

// class MyScene
// {

// public:
//     struct 
//     {
//         uint32_t count;
//         VkBuffer buffer;
//         VkDeviceMemory memory;
//     } vertices;

//     struct 
//     {
//         uint32_t count;
//         VkBuffer buffer;
//         VkDeviceMemory memory;
//     } indices;

//     std::vector<RenderObject> renderObjects;//numObjects
//     //! material includes ks kd ka... and texture
//     std::vector<Material*> materials;//numMaterials
//     std::vector<UBO_model> modelMatrices;//numObjects
    
//     void bind(MyCommandBuffer& mcmdbuffer, uint32_t binding=0)
//     {
//         VkDeviceSize offsets[]={0};
//         mcmdbuffer.bindVertexBuffers(binding,1,&vertices.buffer,offsets);
//         mcmdbuffer.bindIndexBuffer(indices.buffer, offsets[0]);
//     }

//     void draw(MyCommandBuffer& mcmdbuffer)
//     {
//         for(auto& renderObject: renderObjects)
//         {
//             mcmdbuffer.drawIndexed(renderObject.indexCount,1,renderObject.firstIndex);

//         }
//     }
    

// };

