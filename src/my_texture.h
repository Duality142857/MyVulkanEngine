#pragma once
#include"my_vulkanRHI.h"
#include<map>
#include<memory>
#include<filesystem>




class MyTexture;

class MyTexturePool
{
    MyVulkanRHI* rhi;
public:
    MyTexturePool(MyVulkanRHI* _rhi):rhi{_rhi}{}
    std::vector<std::shared_ptr<MyTexture>> textures;
    std::map<std::string, int> indexMap;
    std::vector<VkDescriptorImageInfo> descriptorImageInfos;

    void addTexture(const std::string& path);
    std::shared_ptr<MyTexture> getTexture(const std::string& path);
};

class MyTexture
{
private:
    MyVulkanRHI* rhi;
public:
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    std::string path;
    // std::string name;

    MyTexture(MyVulkanRHI* _rhi, const std::string& _path):path{_path},rhi{_rhi}
    {
        createTextureImage(path);
        createTextureImageView();
        createTextureSampler();
    }
    void createTextureImage(const std::string& filename);
    
    void createTextureImageView();

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    
    void createTextureSampler();

    VkDescriptorImageInfo descriptorImageInfo(VkImageLayout imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout=imageLayout;
        imageInfo.imageView=textureImageView;
        imageInfo.sampler=textureSampler;
        return imageInfo;
    }
};

