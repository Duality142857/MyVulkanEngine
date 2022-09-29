#include"my_commandbuffer.h"
#include<iostream>
void MyCommandBuffer::reset()
{
    vkResetCommandBuffer(commandbuffer,VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void MyCommandBuffer::begin()
{
    VkCommandBufferBeginInfo cmdbeginInfo{};
    cmdbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(commandbuffer,&cmdbeginInfo);
}

void MyCommandBuffer::beginRenderPass(const VkRenderPassBeginInfo* renderpassBeginInfo_p)
{
    vkCmdBeginRenderPass(commandbuffer, renderpassBeginInfo_p, VK_SUBPASS_CONTENTS_INLINE);
}
    
void MyCommandBuffer::setViewPort(const VkViewport* viewport_p)
{
    vkCmdSetViewport(commandbuffer,0,1,viewport_p);
}

void MyCommandBuffer::setScissor(const VkRect2D* rect_p)
{
    vkCmdSetScissor(commandbuffer,0,1,rect_p);
}

void MyCommandBuffer::setDepthBias(float depthBiasConstantFactor, float  depthBiasClamp, float depthBiasSlopeFactor)
{
    vkCmdSetDepthBias(commandbuffer,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}

void MyCommandBuffer::bindDescriptorSets(
    VkPipelineBindPoint bindpoint, 
    VkPipelineLayout pipelineLayout,
    uint32_t firstSet,
    uint32_t setCount,
    const VkDescriptorSet* descriptorsets_p,
    uint32_t dynamicOffsetCount,
    const uint32_t* pDynamicOffsets)
{
    vkCmdBindDescriptorSets(commandbuffer,bindpoint,pipelineLayout,0,1,descriptorsets_p,dynamicOffsetCount,pDynamicOffsets);
}

void MyCommandBuffer::bindPipeline(VkPipelineBindPoint bindpoint,VkPipeline pipeline)
{
    vkCmdBindPipeline(commandbuffer,bindpoint,pipeline);
}

void MyCommandBuffer::bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
    vkCmdBindVertexBuffers(commandbuffer,firstBinding,bindingCount,pBuffers,pOffsets);
}

void MyCommandBuffer::bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    vkCmdBindIndexBuffer(commandbuffer,buffer,0,indexType);
}

void MyCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    vkCmdDrawIndexed(commandbuffer,indexCount,instanceCount,firstIndex,vertexOffset,firstInstance);
}

void MyCommandBuffer::endRenderPass()
{
    vkCmdEndRenderPass(commandbuffer);
}

void MyCommandBuffer::end()
{
    vkEndCommandBuffer(commandbuffer);
}