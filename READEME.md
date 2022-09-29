旧结构：
- my_window.h (glfw3.h)  
initWindow, input callbacks

- my_device.h (glfw3.h my_window.h my_buffer.h)  
包含QueueFamilyIndices, SwapChainSupportDetails定义
依赖my_window.h, 创建instance, 设置debugmessenger, 创建surface, pick physical device, 创建logical device, 创建commandpool  
常驻功能：  createBuffer, copyBuffer, copyBuffer2host, beginSingleTimeCommands, endSingleTimeCommands, createImageViews, createImageView, createDataBuffer, updateDataBuffer, findQueueFamilies, findMemoryType, copyBufferToImage等等。。

- my_swapchain.h (my_window.h my_device.h)  
创建swapchain(计算出imageCount), renderpass(定义attachments如colorattachment, depthattachment; subpass; dependency;)不同应用可能需要定义不同的renderpass， 创建imageViews，创建depth resources(包括depthImages, depthImageMemorys, depthImageViews), 创建framebuffers()， 创建一些同步原语...

- my_buffer.h  
引用MyDevice, 包含VkBuffer, VkDeviceMemory,     void create(VkDeviceSize size,VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
void updateData(void* srcData,VkDeviceSize size);

- my_pipeline.h( my_swapchain.h my_descriptors.h my_model.h)  
引用MyDevice, MySwapChain
创建pipelinelayout, 创建pipeline(读取shader文件)

- my_renderer.h  
引用MyDevkice, MySwapChain  
接收windowresize事件，并recreateSwapchain  
allocateCommandBuffers()  
uint32_t startFrame() 等待fence，获取可用imageIndex,返回imageIndex  
void endFrame(imageIndex) 提交commandbuffer, queuePresent , currentFrame+1  

- descriptor.h
引用MyDevice  
VkDescriptorPoolSize poolSize(VkDescriptorType type, uint32_t count)  

创建descriptorpool(根据poolsizes和maxSets)  

VkDescriptorSetLayoutBinding setLayoutBinding(uint32_t binding,uint32_t descriptorCount,VkDescriptorType descriptorType,VkShaderStageFlags stageFlags=VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)  

创建descriptorsetlayout  allocatedescriptorsets  

VkDescriptorBufferInfo bufferInfo(const MyBuffer& mybuffer, int bufferSize, int offset=0)  
VkDescriptorImageInfo imageInfo(VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler)  


Imaging:
1. 如何描述场景
选择网格拓扑方式：点、线、三角形等(VkPipelineInputAssemblyStateCreateInfo结构体的topology，在pipeline构建时设置)  

给定模型与位置，模型包含原创几何体与加载的网格模型  
给定光源与位置  
给定camera参数  

选择阴影方法等  

my_scene.h 读取场景脚本，生成数据如vertexbuffer, indexbuffer, ubo, ssbo等以及配置项如光源拓扑方式、阴影方法、光源等，my_renderer.h将会对这些数据进行渲染。

2. 图形后端
例：初始化vulkan(初始化device, swapchain...)

3. 主app

```c++
void run()
{
    init(); //createDescriptos, create pipelines, init gui, create models 等
    mainloop();  
    cleanup();  
}

void mainloop()
{
    while(!windowshouldclose)
    {
        poolevents();
        render();
        waitIdle(device);
    }  
}
```


4. 如何绘制场景 renderer

```c++

void render()
{
    startframe();
    updateData();//包括descriptorsets等  
    updateUi();
    recordCommands();
    endframe();
}

```




5. 如何与场景交互

组件：
- Window系统
- Input系统
- 绘制系统


- 模型系统
- 物理系统
- Device
- Swapchain
- Pipeline
- Descriptor
- Gui
- Scene