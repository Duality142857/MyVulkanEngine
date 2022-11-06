#include"my_vulkanUtil.h"
#include<mygeo/mat.h>



void MyVulkanUtils::createPipelineLayout(VkDescriptorSetLayout dsLayout, VkPipelineLayout* pLayout)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts =&dsLayout;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags=VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset=0;
    pushConstantRange.size=sizeof(PushConstantStruct);

    pipelineLayoutInfo.pushConstantRangeCount=1;
    pipelineLayoutInfo.pPushConstantRanges=&pushConstantRange;

    if (vkCreatePipelineLayout(rhi->device, &pipelineLayoutInfo, nullptr, pLayout) != VK_SUCCESS) 
        throw std::runtime_error("failed to create pipeline layout!");
}
