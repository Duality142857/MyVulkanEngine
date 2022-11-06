#pragma once

#include"my_window.h"
#include"my_renderer.h"
#include"my_vulkanRHI.h"
#include"../time/mytime.h"
#include"my_commandbuffer.h"
#include"my_vulkanStructUtil.h"
#include"my_descriptor.h"
#include"my_pipeline.h"
#include"my_gui.h"
#include"my_uniformbuffer.h"
#include"my_geomodel.h"

class App
{
public:
    float fps=60.f;
    float frameInterval=1000.f/fps;
    float t_elapsed=0;
    float t_delta=0;
public: 
    MyWindow* mywindow;
    MyRenderer* renderer;
    MyCommandBuffer* commandbuffer;
    MyVulkanRHI* rhi;
    MyGui gui{rhi};

    App(MyWindow* _mywindow, MyRenderer* _renderer):mywindow{_mywindow},renderer{_renderer},rhi{renderer->rhi}{}
    
    void run()
    {
        prepare();
        mainloop();  
        cleanup();  
    }

    /**
     * @brief prepare data(model)
     * 
     */
    void prepare()
    {
        createDescriptorSets();
        prepareData();
        createPipelines();
        gui.init();
    }
    virtual void cleanup()=0;

    virtual void prepareData()=0;
    virtual void createPipelines()=0;

    virtual void createDescriptorSets()=0;
    virtual void updateDescriptorSets(MyCommandBuffer& mycommmandbuffer, uint32_t imageIndex)=0;
    virtual void tick(float t_delta, float t_elapsed)=0;


        
    void mainloop()
    {
        int i=0;
        while(!mywindow->windowShouldClose())
        {
            auto t0=mytime::now();
            mywindow->pollEvents();
            uint32_t imageIndex=renderer->startFrame();
            updateData(imageIndex);
            updateUi();     
            updateDescriptorSets(renderer->myCommandBuffers[renderer->currentFrame],imageIndex);
            recordCommands(renderer->myCommandBuffers[renderer->currentFrame],imageIndex);
            renderer->endFrame(imageIndex);
            auto t1=mytime::now();
            auto dt=mytime::getDuration<std::chrono::microseconds>(t0,t1);
            if(dt<frameInterval*1000)
            {
                gui.fps=fps;
                mytime::sleep(frameInterval-dt/1000);
                t_elapsed+=frameInterval*0.001;
            }
            else 
            {
                gui.fps=1e6f/dt;
                t_elapsed+=dt*1e-6;
            }
            auto t2=mytime::now();
            // t_delta=mytime::getDuration<std::chrono::seconds>(t0,t2);
            std::chrono::duration<float> td=t2-t0;
            t_delta=td.count();
            tick(t_delta,t_elapsed);
        }  
        vkDeviceWaitIdle(rhi->device);
    }

    virtual void updateData(uint32_t imageIndex)=0;

    virtual void updateUi()=0;

    void recordCommands(MyCommandBuffer& mycommandbuffer, uint32_t imageIndex)
    {
        mycommandbuffer.reset();
        mycommandbuffer.begin();  
        renderpassCommands(mycommandbuffer, imageIndex);   
        mycommandbuffer.end();
    }

    /**
     * @brief containes 1 or more renderpasses with commands, for example bind commands and draw commands
     * implemented by concrete app
     * 
     * @param imageIndex 
     */
    virtual void renderpassCommands(MyCommandBuffer& mycommandbuffer,uint32_t imageIndex)=0;
};