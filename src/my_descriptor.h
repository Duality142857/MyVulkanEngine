#pragma once
#include"my_vulkanRHI.h"

class MyDescriptors
{
public:
    MyVulkanRHI* rhi;
    VkDescriptorPool pool;

    MyDescriptors(MyVulkanRHI* _rhi):rhi{_rhi}{}

    
    VkDescriptorPoolSize poolSize(VkDescriptorType type, uint32_t count)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type=type;
        poolSize.descriptorCount=count;
        return poolSize;
    }
    
    void createDescriptorPool(std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxSets)
    {
        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount=poolSizes.size();
        poolCreateInfo.pPoolSizes=poolSizes.data();
        poolCreateInfo.maxSets=maxSets;

        if (vkCreateDescriptorPool(rhi->device, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS) 
            throw std::runtime_error("failed to create descriptor pool!");
    }

    VkDescriptorSetLayoutBinding setLayoutBinding(uint32_t binding,uint32_t descriptorCount,VkDescriptorType descriptorType,VkShaderStageFlags stageFlags=VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding=binding;
        layoutBinding.descriptorCount=descriptorCount;
        layoutBinding.descriptorType=descriptorType;
        layoutBinding.pImmutableSamplers=nullptr;
        layoutBinding.stageFlags=stageFlags;
        return layoutBinding;
    }

    void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout,std::vector<VkDescriptorSetLayoutBinding>& layoutBindings)
    {
        VkDescriptorSetLayoutCreateInfo layoutCI{};
        layoutCI.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCI.bindingCount=layoutBindings.size();
        layoutCI.pBindings=layoutBindings.data();
        if (vkCreateDescriptorSetLayout(rhi->device, &layoutCI, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
            throw std::runtime_error("failed to create descriptor set layout!");
    }

    void allocateDescriptorSets(std::vector<VkDescriptorSetLayout>& setLayouts, std::vector<VkDescriptorSet>& descriptorSets)
    {
        descriptorSets.resize(setLayouts.size());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool=pool;
        allocInfo.descriptorSetCount=setLayouts.size();
        allocInfo.pSetLayouts=setLayouts.data();

        if(vkAllocateDescriptorSets(rhi->device,&allocInfo,descriptorSets.data())!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }

    VkDescriptorBufferInfo bufferInfo(const VkBuffer& buffer, int bufferSize, int offset=0)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = bufferSize;
        return bufferInfo;
    }

    VkDescriptorImageInfo imageInfo(VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout=imageLayout;
        imageInfo.imageView=imageView;
        imageInfo.sampler=sampler;
        return imageInfo;
    }

    void cleanup()
    {
        vkDestroyDescriptorPool(rhi->device,pool,nullptr);
    }
};

