#pragma once

#include<vulkan/vulkan.h>

static VkRenderPassBeginInfo renderpassBeginInfo(VkRenderPass renderpass, VkFramebuffer framebuffer,VkExtent2D extent)
{
    VkRenderPassBeginInfo renderpassBeginInfo{};
    renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBeginInfo.renderPass = renderpass;
    renderpassBeginInfo.framebuffer = framebuffer;
    renderpassBeginInfo.renderArea.offset = {0, 0};
    renderpassBeginInfo.renderArea.extent = extent;
    return renderpassBeginInfo;
}

static VkViewport viewport_vk(VkExtent2D extent, float minDepth=0.f, float maxDepth=1.f)
{
    VkViewport viewport{};
    viewport.width=extent.width;
    viewport.height=extent.height;
    viewport.minDepth=0.f;
    viewport.maxDepth=1.f;
    return viewport;
}

static VkRect2D rect2D_vk(VkExtent2D extent)
{
    VkRect2D rect{};
    rect.extent.width=extent.width;
    rect.extent.height=extent.height;
    rect.offset.x=0.f;
    rect.offset.y=0.f;
    return rect;
}