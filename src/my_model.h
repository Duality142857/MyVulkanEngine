#pragma once
#include<iostream>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#include<assimp/Logger.hpp>
#include<assimp/Exporter.hpp>
#include"my_vertex.h"
#include"my_texture.h"
#include"my_commandbuffer.h"
#include<mygeo/quaternion.h>
#include<mygeo/rigidbody.h>
#include<mygeo/geo.h>
#include<set>




class MyModel 
{
public:
    MyVulkanRHI* rhi;
    MyGeo::Mat4f modelMat;
    // Rotation rotation;
    // MyGeo::Vec3f translation;
    int modelId;
    // RigidBody rigidBody;

    MyModel(MyVulkanRHI* _rhi, const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>()):rhi{_rhi},modelMat{_modelMat}
    {
        
    }

    // MyModel(MyVulkanRHI* _rhi, const Rotation& _rotation, const MyGeo::Vec3f& _translation):rhi{_rhi},rotation{rotation},translation{_translation}
    // {
        
    // }

    void tick(float t_delta, float t_elapsed)
    {
        // rigidBody.tick(t_delta);
        // modelMat=
    }

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
    virtual void bind(MyCommandBuffer& mcmdbuffer, uint32_t binding=0)=0;

    virtual void draw(MyCommandBuffer& mcmdbuffer)=0;
};


/**
 * @brief 
 * model 由 hierachical nodes组成，每个node可包含多个mesh，每个mesh对应一个material，material可包含texture
 * texture：filename    std::vector<Texture> 
 * Mesh: material
 * 
 */

class MyExtModel: public MyModel
{
private:

    uint32_t currentLoadedMeshNum=0;
    MyTexturePool* texturePool;
    uint32_t firstIndex=0;
    
public:

    MyGeo::Vec3f center{0,0,0};
    MyGeo::Vec3f boundingMin;
    MyGeo::Vec3f boundingMax;


    struct MeshInfo
    {
        uint32_t firstIndex;
        uint32_t count;
        uint32_t materialIndex;
    };

    struct NodeInfo
    {
        std::vector<MeshInfo> meshInfos;
    };

    std::vector<NodeInfo> nodeInfos;
    std::set<std::string> textureSet;
    uint32_t numNodes=0;
    std::string modelPathStr;
    

    MyExtModel(const std::string& path, MyVulkanRHI* _rhi, MyTexturePool* _texturePool, const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>()):MyModel{_rhi,_modelMat},texturePool{_texturePool},modelPathStr{path}
    {
        loadModel(path);
    }

    void setTexturePool(MyTexturePool* _pool)
    {   
        texturePool=_pool;

    }
    void loadMatTextures(aiMaterial* mat, aiTextureType type,std::string& firstTextureName);


    void bind(MyCommandBuffer& mcmdbuffer, uint32_t binding=0)
    {
        VkDeviceSize offsets[]={0};
        mcmdbuffer.bindVertexBuffers(binding,1,&vertices.buffer,offsets);
        mcmdbuffer.bindIndexBuffer(indices.buffer,offsets[0]);
    }
// validation layer: Validation Error: [ VUID-vkCmdDrawIndexed-indexSize-00463 ] Object 0: handle = 0x740000000074, type = VK_OBJECT_TYPE_BUFFER; | MessageID = 0xd4f2594c | vkCmdDrawIndexed() index size (4) * (firstIndex (30522) + indexCount (-29712)) + binding offset (0) = an ending offset of 17179872424 bytes, which is greater than the index buffer size (122088). The Vulkan spec states: (indexSize {times} (firstIndex + indexCount) + offset) must be less than or equal to the size of the bound index buffer, with indexSize being based on the type specified by indexType, where the index buffer, indexType, and offset are specified via vkCmdBindIndexBuffer (https://vulkan.lunarg.com/doc/view/1.2.170.0/linux/1.2-extensions/vkspec.html#VUID-vkCmdDrawIndexed-indexSize-00463)
    void draw(MyCommandBuffer& mcmdbuffer)
    {
        // mcmdbuffer.drawIndexed(indices.count,1);
        for(NodeInfo& nodeInfo:nodeInfos)
        {
            for(MeshInfo& meshInfo:nodeInfo.meshInfos)
            {
                mcmdbuffer.drawIndexed(meshInfo.count,1,meshInfo.firstIndex);
            }
        }
    }


    void loadModel(const std::string& path);

    void processNode(aiNode* node, const aiScene* scene, std::vector<MyVertex_Default>& vbuffer, std::vector<uint32_t>& ibuffer,std::unordered_map<MyVertex_Default, uint32_t>& uniqueVertices);

    void processMesh(const aiMesh* mesh, const aiScene* scene,std::vector<MyVertex_Default>& vbuffer, std::vector<uint32_t>& ibuffer,std::unordered_map<MyVertex_Default, uint32_t>& uniqueVertices);
    

};