
#pragma once

#include"my_scene.h"
#include"my_vulkanRHI.h"
#include"my_vulkanUtil.h"
#include"my_commandbuffer.h"

/**
 * @brief draw models
 * raw model data:mesh/raw geometric primitive, vertexbuffer, indexbuffer， instancing data
 * 
 */

class MyRenderer
{
public:
    MyVulkanRHI* rhi;
    uint32_t currentFrame=0;
    // MyScene* myscene;
    std::vector<MyCommandBuffer> myCommandBuffers;
    Dispatcher eventDispatcher;


public:
    MyRenderer(MyVulkanRHI* _rhi):rhi{_rhi}
    {
        init();
        eventDispatcher.subscribe(WindowResized_Event,[this](const Event& event){onEvent(event);});
    }

    void init();
    void onEvent(const Event& event);

    void allocateCommandBuffers();

    void renderFrame();

    uint32_t startFrame();

    void endFrame(uint32_t imageIndex);


    // void setScene(MyScene* _myscene)
    // {
    //     myscene=_myscene;
    // }
    
};