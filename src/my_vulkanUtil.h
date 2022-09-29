#pragma once
#include<vulkan/vulkan.h>
#include"my_vulkanRHI.h"
#include<mygeo/mat.h>
struct PushConstantStruct
{
    MyGeo::Mat4f modelMat;
};
class MyVulkanUtils
{
private:
    MyVulkanRHI* rhi;
public:
    MyVulkanUtils(MyVulkanRHI* _rhi):rhi{_rhi}{}
    void createPipelineLayout(VkDescriptorSetLayout dsLayout, VkPipelineLayout* pLayout);

};

#pragma once
#include"my_shader.h"
struct PipelineConfigFunctions
{   
    static VkViewport viewport(float width, float height,float x=0.f,float y=0.f, float minDepth=0.f, float maxDepth=1.f)
    {
        return VkViewport{x,y,width,height,minDepth,maxDepth};
    }

    static VkRect2D scissor(VkExtent2D extent,VkOffset2D offset={0,0})
    {
        return VkRect2D{offset,extent};
    }

    static VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo(VkPrimitiveTopology topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology=topology;
        inputAssembly.primitiveRestartEnable=VK_FALSE;
        return inputAssembly;
    }

    static std::vector<VkVertexInputAttributeDescription> mergeAttributeDescriptions(std::vector<VkVertexInputAttributeDescription> d1,std::vector<VkVertexInputAttributeDescription> d2)
    {
        d1.insert(d1.end(),d2.begin(),d2.end());
        return d1;
    }

    static VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(
        std::vector<VkVertexInputBindingDescription>& bindingDescriptions,
        std::vector<VkVertexInputAttributeDescription>& attributeDescriptions
        )
    {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount= bindingDescriptions.size();
        vertexInputInfo.vertexAttributeDescriptionCount=attributeDescriptions.size();
        vertexInputInfo.pVertexBindingDescriptions=bindingDescriptions.data();
        vertexInputInfo.pVertexAttributeDescriptions=attributeDescriptions.data();
        return vertexInputInfo;
    }


//{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    static VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo(std::vector<VkDynamicState>& dynamicStates)
    {
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pDynamicStates=dynamicStates.data();
        dynamicStateCreateInfo.dynamicStateCount=dynamicStates.size();
        dynamicStateCreateInfo.flags=0;
        return dynamicStateCreateInfo;
    }


    static VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo(
        VkPolygonMode polygonMode=VK_POLYGON_MODE_FILL,
        VkCullModeFlags cullMode=VK_CULL_MODE_BACK_BIT,
        VkFrontFace frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VkBool32 depthBiasEnable=VK_FALSE
        )
    {
        VkPipelineRasterizationStateCreateInfo rasterizerStateCI{};
        rasterizerStateCI.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerStateCI.depthClampEnable = VK_FALSE;
        rasterizerStateCI.rasterizerDiscardEnable = VK_FALSE;
        rasterizerStateCI.polygonMode = polygonMode;
        rasterizerStateCI.lineWidth = 1.0f;
        rasterizerStateCI.cullMode = cullMode;
        rasterizerStateCI.frontFace = frontFace;
        rasterizerStateCI.depthBiasEnable = depthBiasEnable;
        return rasterizerStateCI;
    }

    static VkPipelineMultisampleStateCreateInfo multisampleCreateInfo()
    {
        VkPipelineMultisampleStateCreateInfo ms{};
        ms.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms.sampleShadingEnable=VK_FALSE;
        ms.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
        return ms;
    }

    static VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo(VkCompareOp depthCompareOp=VK_COMPARE_OP_LESS,VkBool32 stencilTestEnable=VK_FALSE)
    {
        VkPipelineDepthStencilStateCreateInfo ds{};
        ds.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ds.depthTestEnable=VK_TRUE;
        ds.depthWriteEnable=VK_TRUE;
        ds.depthCompareOp=depthCompareOp;
        ds.depthBoundsTestEnable=VK_FALSE;
        ds.stencilTestEnable=stencilTestEnable;
        return ds;
    }

    static VkPipelineColorBlendAttachmentState colorBlendAttachmentState(VkColorComponentFlags colorComponentFlags=VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,VkBool32 blendEnable=VK_FALSE)
    {
        VkPipelineColorBlendAttachmentState cba{};
        cba.colorWriteMask=colorComponentFlags;
        cba.blendEnable=blendEnable;
        return cba;
    }

    static VkPipelineColorBlendStateCreateInfo colorBlendingStateCreateInfo(uint32_t attachmentCount, VkPipelineColorBlendAttachmentState* cba)
    {
        VkPipelineColorBlendStateCreateInfo cbs{};
        cbs.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        cbs.logicOpEnable=VK_FALSE;
        cbs.logicOp=VK_LOGIC_OP_COPY;
        cbs.attachmentCount=1;
        cbs.pAttachments=cba;
        cbs.blendConstants[0]=0.f;
        cbs.blendConstants[1]=0.f;
        cbs.blendConstants[2]=0.f;
        cbs.blendConstants[3]=0.f;  
        return cbs;
    }
    
};