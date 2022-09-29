#pragma once

#include"my_shader.h"
#include"my_vertex.h"

class MyPipeline
{
public:
    VkPipeline pipeline;
    MyVulkanRHI* rhi;
    std::string vertexShaderFile;
    std::string fragmentShaderFile;
    std::string computeShaderFile;

    MyPipeline(MyVulkanRHI* _rhi):rhi{_rhi}{}

    void createGraphicsPipeline(const std::string& _vertexShaderFile, const std::string& _fragmentShaderFile, VkPipelineLayout pipelineLayout, int textureArraySize=10)
    {
        vertexShaderFile=_vertexShaderFile;
        fragmentShaderFile=_fragmentShaderFile;
        MyShader vertexShader{rhi,vertexShaderFile,VK_SHADER_STAGE_VERTEX_BIT};
        MyShader fragmentShader{rhi,fragmentShaderFile,VK_SHADER_STAGE_FRAGMENT_BIT};

        auto vertShaderStageInfo=vertexShader.getShaderStageCreateInfo();
        auto fragShaderStageInfo=fragmentShader.getShaderStageCreateInfo();

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{vertShaderStageInfo, fragShaderStageInfo};

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{MyVertex_Default::getBindingDescription()};

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions=MyVertex_Default::getAttributeDescriptions(0,0);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo=PipelineConfigFunctions::vertexInputStateCreateInfo(bindingDescriptions,attributeDescriptions);

        VkPipelineInputAssemblyStateCreateInfo inputAssembly=PipelineConfigFunctions::inputAssemblyStateCreateInfo();

        VkPipelineViewportStateCreateInfo viewportStateCI{};
        viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCI.viewportCount = 1;
        viewportStateCI.scissorCount = 1;

        std::vector<VkDynamicState> dynamicStates={VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo=PipelineConfigFunctions::dynamicStateCreateInfo(dynamicStates);

        VkPipelineRasterizationStateCreateInfo rasterizerStateCI=PipelineConfigFunctions::rasterizerStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE);

        VkPipelineMultisampleStateCreateInfo multisampling=PipelineConfigFunctions::multisampleCreateInfo();

        VkPipelineDepthStencilStateCreateInfo depthStencil=PipelineConfigFunctions::depthStencilStateCreateInfo();

        VkPipelineColorBlendAttachmentState colorBlendAttachment=PipelineConfigFunctions::colorBlendAttachmentState();

        VkPipelineColorBlendStateCreateInfo colorBlendingStateCI=PipelineConfigFunctions::colorBlendingStateCreateInfo(1,&colorBlendAttachment);

        uint32_t enablePCF=textureArraySize;
        VkSpecializationMapEntry sme{};
        sme.constantID=0;
        sme.offset=0;
        sme.size=sizeof(uint32_t);

        VkSpecializationInfo specializationInfo{};
        specializationInfo.mapEntryCount=1;
        specializationInfo.pMapEntries=&sme;
        specializationInfo.dataSize=sizeof(uint32_t);
        specializationInfo.pData=&enablePCF;
        shaderStages[1].pSpecializationInfo=&specializationInfo;


        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportStateCI;
        pipelineInfo.pRasterizationState = &rasterizerStateCI;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlendingStateCI;
        pipelineInfo.layout = pipelineLayout;
        //! remember to add dynamicstate later!
        pipelineInfo.pDynamicState=&dynamicStateCreateInfo;
        pipelineInfo.renderPass = rhi->renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(rhi->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");
        
        
        
    }
    
    void createdepthPipeline(const std::string& vertexShaderFile,VkPipelineLayout pipelineLayout,uint32_t pcf)
    {
        MyShader vertexShader{rhi,vertexShaderFile,VK_SHADER_STAGE_VERTEX_BIT};
        auto vertShaderStageInfo=vertexShader.getShaderStageCreateInfo();
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertShaderStageInfo};

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{MyVertex_Default::getBindingDescription()};

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions=MyVertex_Default::getAttributeDescriptions(0,0);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo=PipelineConfigFunctions::vertexInputStateCreateInfo(bindingDescriptions,attributeDescriptions);
        
        VkPipelineInputAssemblyStateCreateInfo inputAssembly=PipelineConfigFunctions::inputAssemblyStateCreateInfo();
        
        VkPipelineViewportStateCreateInfo viewportStateCI{};
        viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCI.viewportCount = 1;
        viewportStateCI.scissorCount = 1;
        
        std::vector<VkDynamicState> dynamicStates={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_DEPTH_BIAS, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo=PipelineConfigFunctions::dynamicStateCreateInfo(dynamicStates);

        
        
        VkPipelineRasterizationStateCreateInfo rasterizerStateCI=PipelineConfigFunctions::rasterizerStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE,VK_TRUE); 
        
        VkPipelineMultisampleStateCreateInfo multisampling=PipelineConfigFunctions::multisampleCreateInfo();

        VkPipelineDepthStencilStateCreateInfo depthStencil=PipelineConfigFunctions::depthStencilStateCreateInfo();

        VkPipelineColorBlendAttachmentState colorBlendAttachment=PipelineConfigFunctions::colorBlendAttachmentState();

        VkPipelineColorBlendStateCreateInfo colorBlendingStateCI=PipelineConfigFunctions::colorBlendingStateCreateInfo(0,&colorBlendAttachment);

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportStateCI;
        pipelineInfo.pRasterizationState = &rasterizerStateCI;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlendingStateCI;
        pipelineInfo.layout = pipelineLayout;
        //! remember to add dynamicstate later!
        pipelineInfo.pDynamicState=&dynamicStateCreateInfo;
        pipelineInfo.renderPass = rhi->shadowRenderpass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(rhi->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");

    }




    
};