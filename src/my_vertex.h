#pragma once
#include<mygeo/vec.h>
#include<vulkan/vulkan.h>

inline void hash_combine(std::size_t& seed) { }
template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
}

struct MyVertex_Default
{
    MyGeo::Vec3f position;
    MyGeo::Vec3f color;
    MyGeo::Vec3f normal;
    MyGeo::Vec3f texCoord;
    // struct 
    // {
    //     float u;
    //     float v;
    //     int w;
    // } texCoord;
    
    friend std::ostream& operator<<(std::ostream& ostr, MyVertex_Default v)
    {
        // std::cout<<'{'<<'{'<<v.position.x<<','<<v.position.y<<','<<v.position.z<<'}'<<','<<"{1,1,1},"<<'{'<<v.normal<<'}'<<','<<"{-1,-1,-1}},"<<std::endl;
        return ostr<<'{'<<'{'<<v.position.x<<','<<v.position.y<<','<<v.position.z<<'}'<<','<<"{1,1,1},"<<'{'<<v.normal<<'}'<<','<<"{-1,-1,-1}},"<<std::endl;
    }

    bool operator==(const MyVertex_Default& other) const
    {
        return position==other.position && normal==other.normal;
    }
    
    static VkVertexInputAttributeDescription attribDesc(uint32_t binding, uint32_t location,VkFormat format,uint32_t offset)
    {
        VkVertexInputAttributeDescription desc{};
        desc.binding=binding;
        desc.location=location;
        desc.format=format;
        desc.offset=offset;
        return desc;
    }
    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding=0) 
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = binding;
        bindingDescription.stride = sizeof(MyVertex_Default);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding=0,uint32_t startLocation=0) 
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.resize(4);

        attributeDescriptions[0]=attribDesc(binding,startLocation,VK_FORMAT_R32G32B32_SFLOAT,offsetof(MyVertex_Default, position));

        attributeDescriptions[1]=attribDesc(binding,startLocation+1,VK_FORMAT_R32G32B32_SFLOAT,offsetof(MyVertex_Default, color));

        attributeDescriptions[2]=attribDesc(binding,startLocation+2,VK_FORMAT_R32G32B32_SFLOAT,offsetof(MyVertex_Default, normal));

        attributeDescriptions[3]=attribDesc(binding,startLocation+3,VK_FORMAT_R32G32B32_SFLOAT,offsetof(MyVertex_Default, texCoord));

        // attributeDescriptions[4]=attribDesc(binding,startLocation+4,VK_FORMAT_R32_SINT,offsetof(MyVertex_Default, modelId));


        return attributeDescriptions;
    }

};


namespace std 
{
    template<> 
    struct hash<MyVertex_Default> 
    {
        size_t operator()(MyVertex_Default const& v) const 
        {
            size_t seed=3;
            hash_combine(seed,v.position.x,v.position.y,v.position.z);
            return seed;
        }
    };
}