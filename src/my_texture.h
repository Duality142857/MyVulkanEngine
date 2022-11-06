#pragma once
#include"my_vulkanRHI.h"
#include<map>
#include<memory>
#include<filesystem>
#include<ktx.h>
#include<ktxvulkan.h>




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

class CubeMapTexture
{
private:
    MyVulkanRHI* rhi;
public:
    CubeMapTexture(MyVulkanRHI* _rhi):rhi{_rhi}{}
    ktxVulkanTexture vtxt;
    VkSampler sampler;
    VkImageView imageView;
    void loadCubemap(std::string filename)
    {
        ktxResult result;
        ktxTexture* ktxt;
        ktx_size_t offset;
        ktx_uint8_t* image;
        ktx_uint32_t level, layer, faceSlice;
        ktxVulkanDeviceInfo vdi;

        ktxVulkanDeviceInfo_Construct(&vdi,rhi->physicalDevice,rhi->device,rhi->graphicsQueue,rhi->commandPool,nullptr);
        
    //KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT
        result=ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktxt);
        assert(result==KTX_SUCCESS);

        

        result=ktxTexture_VkUploadEx(ktxt,&vdi,&vtxt,VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_SAMPLED_BIT,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        
        assert(result==KTX_SUCCESS);    
        std::cout<<"num of faces: "<<ktxt->numFaces<<std::endl;
        // ktxTexture_Destroy(ktxt);
        // ktxVulkanDeviceInfo_Destroy(&vdi);

        //using vtxt 
        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.maxAnisotropy=1.0f;
        samplerCI.magFilter=VK_FILTER_LINEAR;
        samplerCI.minFilter=VK_FILTER_LINEAR;
        samplerCI.mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.mipLodBias=0.f;
        samplerCI.compareOp=VK_COMPARE_OP_NEVER;
        samplerCI.minLod=0.f;
        samplerCI.maxLod=vtxt.levelCount;
        samplerCI.borderColor=VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(rhi->physicalDevice, &properties);

        samplerCI.maxAnisotropy=properties.limits.maxSamplerAnisotropy;
        //! check
        samplerCI.anisotropyEnable=VK_TRUE;
        vkCreateSampler(rhi->device,&samplerCI,nullptr,&sampler);

        VkImageViewCreateInfo viewCI{};
        viewCI.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCI.viewType=VK_IMAGE_VIEW_TYPE_CUBE;
        viewCI.format=VK_FORMAT_R8G8B8A8_UNORM;
        viewCI.components={ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        viewCI.subresourceRange={ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        viewCI.subresourceRange.layerCount=vtxt.layerCount;
        viewCI.subresourceRange.levelCount=vtxt.levelCount;
        viewCI.image=vtxt.image;
        
        vkCreateImageView(rhi->device,&viewCI,nullptr,&imageView);
    }

    VkDescriptorImageInfo descriptorImageInfo(VkImageLayout imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout=imageLayout;
        imageInfo.imageView=imageView;
        imageInfo.sampler=sampler;
        return imageInfo;
    }
};