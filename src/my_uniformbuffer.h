#pragma once 
#include"my_vulkanRHI.h"

class MyUniformBuffer
{
private:
    MyVulkanRHI* rhi;
public:
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;

    MyUniformBuffer(MyVulkanRHI* _rhi, VkDeviceSize _size,VkMemoryPropertyFlags properties=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT):rhi{_rhi},size{_size}
    {
        rhi->createBuffer(size,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,properties,buffer,memory);
    }

    void updateData(void* srcData, VkDeviceSize _size)
    {
        assert(size==_size);
        void* data;
        vkMapMemory(rhi->device,memory,0,size,0,&data);
        memcpy(data,srcData,size);
        vkUnmapMemory(rhi->device,memory);
    }

};