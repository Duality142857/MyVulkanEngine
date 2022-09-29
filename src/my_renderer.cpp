#include"my_renderer.h"
#include"log.h"

inline VkSubmitInfo submitInfo(VkPipelineStageFlags waitDstStageMask, uint32_t waitSemaphoreCount, VkSemaphore* pwaitSemaphores, 
uint32_t signalSemaphoreCount, VkSemaphore* psignalSemaphores, 
uint32_t cmdbufferCount, VkCommandBuffer* pcmdbuffers)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitDstStageMask;
    submitInfo.waitSemaphoreCount=waitSemaphoreCount;
    submitInfo.pWaitSemaphores=pwaitSemaphores;
    submitInfo.signalSemaphoreCount=signalSemaphoreCount;
    submitInfo.pSignalSemaphores=psignalSemaphores;
    submitInfo.commandBufferCount=cmdbufferCount;
    submitInfo.pCommandBuffers=pcmdbuffers;

    return submitInfo;
}


inline void queueSubmit(VkQueue queue, uint32_t submitCount, VkSubmitInfo* psubmitInfos ,VkFence fence)
{
    if (vkQueueSubmit(queue, submitCount, psubmitInfos, fence) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

inline VkResult queuePresent(VkQueue queue,uint32_t waitSemaphoreCount,VkSemaphore* pwaitSemaphores, uint32_t swapchainCount, VkSwapchainKHR* pswapchains,uint32_t* pimageIndices)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount=1;
    presentInfo.pWaitSemaphores=pwaitSemaphores;
    presentInfo.swapchainCount=swapchainCount;
    presentInfo.pSwapchains=pswapchains;
    presentInfo.pImageIndices=pimageIndices;
    return vkQueuePresentKHR(queue,&presentInfo);

}


void MyRenderer::init()
{
    allocateCommandBuffers();
}

void MyRenderer::allocateCommandBuffers()
{
    myCommandBuffers.resize(rhi->maxFramesInFlight);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = rhi->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) myCommandBuffers.size();
    if (vkAllocateCommandBuffers(rhi->device, &allocInfo, (VkCommandBuffer*)myCommandBuffers.data()) != VK_SUCCESS) 
        throw std::runtime_error("failed to allocate command buffers!");
}

void MyRenderer::renderFrame()
{
    uint32_t imageIndex=startFrame();
    //update data, ui, etc. 
    

    endFrame(imageIndex);
}

uint32_t MyRenderer::startFrame()
{
    vkWaitForFences(rhi->device,1,&rhi->frameFences[currentFrame],VK_TRUE,UINT64_MAX);
    vkResetFences(rhi->device,1,&rhi->frameFences[currentFrame]);
    
    uint32_t imageIndex;
    VkResult result=vkAcquireNextImageKHR(rhi->device, rhi->swapChain, UINT64_MAX, rhi->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);


    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        rhi->recreateSwapChain(); 
        return -1;
    } 
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    return imageIndex;
}

void MyRenderer::endFrame(uint32_t imageIndex)
{

    VkSubmitInfo info=submitInfo(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,1,&rhi->imageAvailableSemaphores[currentFrame],1,&rhi->renderFinishedSemaphores[currentFrame],1,&myCommandBuffers[currentFrame].commandbuffer);
    

    queueSubmit(rhi->graphicsQueue,1,&info,rhi->frameFences[currentFrame]);

    auto result=queuePresent(rhi->presentQueue,1,&rhi->renderFinishedSemaphores[currentFrame],1,&rhi->swapChain,&imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || rhi->mywindow->framebufferResized) 
    {
        WindowResizedEvent event;
        eventDispatcher.dispatch(event);
    } 
    else if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
    currentFrame = (currentFrame + 1) %rhi->maxFramesInFlight;
}


void MyRenderer::onEvent(const Event& event)
{
    if(event.type()==WindowResized_Event)
    {
        rhi->mywindow->framebufferResized = false;
        rhi->recreateSwapChain();
    }
}