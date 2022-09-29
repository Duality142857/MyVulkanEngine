#pragma once 
#include<vulkan/vulkan.h>
class MyCommandBuffer
{
public:
    VkCommandBuffer commandbuffer;
public:
    void reset();
    void begin();
    void beginRenderPass(const VkRenderPassBeginInfo* renderpassBeginInfo_p);
    
    void setViewPort(const VkViewport* viewport_p);

    void setScissor(const VkRect2D* rect_p);

    void setDepthBias(float depthBiasConstantFactor, float  depthBiasClamp, float depthBiasSlopeFactor);

    void bindDescriptorSets(
        VkPipelineBindPoint bindpoint, 
        VkPipelineLayout pipelineLayout,
        uint32_t firstSet,
        uint32_t setCount,
        const VkDescriptorSet* descriptorsets_p,
        uint32_t dynamicOffsetCount=0,
        const uint32_t* pDynamicOffsets=nullptr);

    void bindPipeline(VkPipelineBindPoint bindpoint,VkPipeline pipeline);

    void bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,const VkBuffer* buffers_p, const VkDeviceSize* pOffsets);

    void bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType=VK_INDEX_TYPE_UINT32);

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex=0, int32_t vertexOffset=0, uint32_t firstInstance=0);

    void endRenderPass();

    void end();
    
};