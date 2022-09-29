#include"src/app.h"
#include"src/my_vulkanUtil.h"
#include"src/my_vertex.h"
#include"src/my_model.h"
// #include<mygeo/mat.h>
#include<mygeo/geo.h>
#include<ImGuizmo.h>
#include"geometry/transforms.h"
#include"src/my_geomodel.h"
#include"src/my_scene.h"
namespace fs=std::filesystem;


struct UBO_model
{
    MyGeo::Mat4f modelMat;
};


struct UBO_scene
{
    alignas(16) MyGeo::Mat4f model;
    alignas(16) MyGeo::Mat4f view;
    alignas(16) MyGeo::Mat4f proj;
    alignas(16) MyGeo::Mat4f lightMVP;
    alignas(16) MyGeo::Vec3f lightPos;
    alignas(16) MyGeo::Vec3f lightColor;
    alignas(16) MyGeo::Vec3f eyePos;
    alignas(16) MyGeo::Vec3f ks;
    alignas(16) MyGeo::Vec3f kd;
};

struct UBO_shadow
{
    MyGeo::Mat4f depthMVP;
    MyGeo::Mat4f sceneModel;
};

class ShadowMapTest: public App 
{

private:
    MyDescriptors descriptors{rhi};
    MyVulkanUtils vulkanUtils{rhi};
    VkPipelineLayout pipelineLayout_scene;
    VkPipelineLayout pipelineLayout_shadow;
    MyPipeline pipeline_scene{rhi};
    MyPipeline pipeline_shadow{rhi};

    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets_scene;
    std::vector<VkDescriptorSet> descriptorSets_shadow;

    std::vector<MyUniformBuffer> uniformBuffers_scene;
    std::vector<MyUniformBuffer> uniformBuffers_shadow;

    UBO_scene ubo_scene{};
    UBO_shadow ubo_shadow{};

    MyGeo::Vec3f camPos=MyGeo::Vec3f{0.4,3,5};
    MyGeo::Vec3f eyePos=MyGeo::Vec3f{0.4,3,5};
    MyGeo::Vec3f lookat=MyGeo::Vec3f{0,2.5,0};
    MyGeo::Camera cam{camPos,lookat,{0,1,0}};
    MyGeo::Vec3f lightPos={0.4,3,5};//{0,7,3};
    MyGeo::Mat4f modelMat=MyGeo::Eye<float,4>();

    MyGeo::Camera lightCam{lightPos,{0,0,0},{0,1,0}};

    float lightIntensity=10;
    float lightColor[3]={1,1,1};

    float depthBiasConstant=1.25f;
    float depthBiasSlope=1.75f;
    MyTexturePool texturePool{rhi};
    MyModel* model;
    


    // MyModel model{"../resources/Marry.obj",rhi};
    // MyModel model{"../resources/iphonex/Iphone seceond version finished.obj",rhi};
    // MyModel model{"../resources/cube/cube.obj",rhi};//lost_empire.obj
    // MyModel model{"../resources/lostEmpire/lost_empire_tri.obj",rhi};//lost_empire.obj
    // MyModel model{"../resources/lpshead/head_triangle.obj",rhi};//lost_empire.obj
    // MyModel model{"../resources/tex-models/bunny.obj",rhi};//lost_empire.obj
    // MyModel model{"../resources/holodeck/holodeck_tri.obj",rhi};//lost_empire.obj





    MyRect rectmodel{rhi,{10,10}};
    MyBox mybox{rhi,{2,5,1}};
    MyIcosSphere sphere{rhi,{3}};
    // MySphere spheremodel{rhi,{0.5}};
    // MyTexture mytexture{rhi,"../resources/MC003_Kozakura_Mari.png"};   //iphone-x-screens-status-bar.jpg
    // MyTexture mytexture{rhi,"../resources/iphonex/iphone-x-screens-status-bar.jpg"};
    // MyTexture mytexture{rhi,"../resources/cube/default.png"}; 
    // MyTexture mytexture{rhi,"../resources/lostEmpire/lost_empire-RGBA.png"}; 
    // MyTexture mytexture{rhi,"../resources/lpshead/lambertian.jpg"}; 
    // MyTexture mytexture{rhi,"../resources/tex-models/bunny-atlas.jpg"}; 
    // MyTexture mytexture{rhi,"../resources/holodeck/screen.png"}; 

void loadAllTextures(const std::string& pathstr)
{
    fs::path resourcesPath(pathstr);
    if(!fs::is_directory(resourcesPath)) return;
    for(auto& x:fs::directory_iterator(resourcesPath))
    {
        auto entryPath=x.path();

        loadAllTextures(entryPath);
        
        std::string extensionStr=entryPath.extension().c_str();
        
        if(extensionStr==std::string(".jpg") || extensionStr==std::string(".png"))
        {
            std::cout<<"load texture..."<<std::endl;
            texturePool.addTexture(entryPath.c_str());
        }
        
    }
}


public:
    ShadowMapTest(MyWindow* _mywindow, MyRenderer* _renderer):App{_mywindow,_renderer}
    {
        descriptorSets_shadow.resize(rhi->imageCount);
        descriptorSets_scene.resize(rhi->imageCount);
        std::cout<<"asdf"<<std::endl;


        // loadAllTextures("../resources");

        texturePool.addTexture("../resources/cube/default.png");
        // texturePool.addTexture("../resources/MC003_Kozakura_Mari.png");
        // texturePool.addTexture("../resources/lpshead/lambertian.jpg");
        // texturePool.addTexture("../resources/MC003_Kozakura_Mari.png");
        // std::cout<<"asdf "<<texturePool.indexMap["../resources/lpshead/lambertian.jpg"]<<std::endl;
        // model=new MyExtModel{"../resources/lpshead/head_triangle.obj",rhi,&texturePool};
        // model=new MyModel{"../resources/cornellbox/CornellBox-Water.obj",rhi,&texturePool};

        // model=new MyExtModel{"../resources/Marry.obj",rhi,&texturePool};

        // model=new MyModel{"../resources/tex-models/feline.obj",rhi,&texturePool};

        // model=new MyModel{"../resources/cube/cube.obj",rhi,&texturePool};
        // model=new MyModel{"../resources/lostEmpire/lost_empire_tri.obj",rhi,&texturePool};
        model=new MyExtModel{"../resources/tex-models/bunny.obj",rhi,&texturePool};
        // model=new MyModel{"../resources/sibenik/sibenik.obj",rhi,&texturePool};

        // model=new MyModel{"../resources/cornellbox/water.obj",rhi,&texturePool};
        // model=new MyModel{"../resources/cornellbox/CornellBox-Sphere.obj",rhi,&texturePool};
        // model=new MyModel{"../resources/sportsCar.obj",rhi,&texturePool};





    // MyModel model{"../resources/Marry.obj",rhi};


    }

    virtual void cleanup() override
    {
        std::cout<<"cleanup"<<std::endl;
    }

    void renderpassCommands(MyCommandBuffer& mycommandbuffer, uint32_t imageIndex) override
    {
        //! shadow pass commands
        {
            VkRenderPassBeginInfo shadowRenderPassBI= renderpassBeginInfo(rhi->shadowRenderpass,rhi->shadowFrameBuffer,rhi->shadowBufferExtent);

            std::array<VkClearValue, 1> clearValues{};
            clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValues[0].depthStencil = {1.0f, 0};
            shadowRenderPassBI.clearValueCount=1;
            shadowRenderPassBI.pClearValues=clearValues.data();
            
            mycommandbuffer.beginRenderPass(&shadowRenderPassBI);
            
            auto viewport=viewport_vk(rhi->shadowBufferExtent);
            auto rect=rect2D_vk(rhi->shadowBufferExtent);
            mycommandbuffer.setViewPort(&viewport);
            mycommandbuffer.setScissor(&rect);
            mycommandbuffer.setDepthBias(depthBiasConstant,0.f,depthBiasSlope);

            mycommandbuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_shadow,0,1,&descriptorSets_shadow[imageIndex]);

            mycommandbuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline_shadow.pipeline);

            model->bind(mycommandbuffer,0);
            model->draw(mycommandbuffer);

            // mybox.bind(mycommandbuffer,0);
            // mybox.draw(mycommandbuffer);

            // spheremodel.bind(mycommandbuffer);
            // spheremodel.draw(mycommandbuffer);

            rectmodel.bind(mycommandbuffer);
            rectmodel.draw(mycommandbuffer);
            // sphere.bind(mycommandbuffer);
            // sphere.draw(mycommandbuffer);


            mycommandbuffer.endRenderPass();

        }


        //! scene pass commands
        {
            VkRenderPassBeginInfo sceneRenderPassBI= renderpassBeginInfo(rhi->renderPass,rhi->framebuffers[imageIndex],rhi->swapChainExtent);

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color={ { 0.025f, 0.025f, 0.025f, 1.0f } };
            clearValues[1].depthStencil={1.f,0};
            sceneRenderPassBI.clearValueCount=2;
            sceneRenderPassBI.pClearValues=clearValues.data();

            mycommandbuffer.beginRenderPass(&sceneRenderPassBI);

            auto viewport=viewport_vk(rhi->swapChainExtent);
            auto rect=rect2D_vk(rhi->swapChainExtent);
            mycommandbuffer.setViewPort(&viewport);
            mycommandbuffer.setScissor(&rect);

            mycommandbuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_scene,0,1,&descriptorSets_scene[imageIndex]);

            mycommandbuffer.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline_scene.pipeline);

            model->bind(mycommandbuffer);
            model->draw(mycommandbuffer);

            // mybox.bind(mycommandbuffer,0);
            // mybox.draw(mycommandbuffer);

            // spheremodel.bind(mycommandbuffer);
            // spheremodel.draw(mycommandbuffer);
            // sphere.bind(mycommandbuffer);
            // sphere.draw(mycommandbuffer);

            rectmodel.bind(mycommandbuffer);
            rectmodel.draw(mycommandbuffer);
            ImGui_ImplVulkan_RenderDrawData(gui.drawData,mycommandbuffer.commandbuffer);
            mycommandbuffer.endRenderPass();
        }
    }

    void prepareData() override
    {
        //* load models(creating vertexbuffers and indexbuffers)

        //* create uniform buffers
        VkDeviceSize bufferSize_scene = sizeof(UBO_scene);
        VkDeviceSize bufferSize_shadow=sizeof(UBO_shadow);
        for(int i=0;i!=rhi->images.size();++i) 
        {
            uniformBuffers_scene.push_back({rhi,bufferSize_scene});
            uniformBuffers_shadow.push_back({rhi,bufferSize_shadow});
        } 
        std::cout<<"finish prepredata"<<std::endl;
    }

    /**
     * @brief 
     * update frame data and write descriptors
     * 
     */
    void updateData(uint32_t imageIndex) override 
    {
        //* update uniform buffers
        mywindow->tick();
        ubo_scene.ks={0.2,0.2,0.2};
        ubo_scene.kd={0.8,0.8,0.8};

        static auto startTime=mytime::now();
        auto currentTime = mytime::now();
        float time_elapsed=mytime::getDuration<std::chrono::milliseconds>(startTime,currentTime)*0.0001f;

        if(!ImGuizmo::IsUsing() &&!gui.anyWindowFocused() && mywindow->leftmousePressed && mywindow->dragAngle>0.1f) 
        {//
            cam.position=rotationMat(mywindow->dragAxis,mywindow->dragAngle)(MyGeo::Vec4f({cam.position-cam.lookat,1.f})).head+cam.lookat;
            cam.lookdirection=(cam.lookat-cam.position).normalVec();
            mywindow->dragAngle=0;
        }

        static MyGeo::Vec3f xaxis,yaxis,zaxis;
        zaxis=(cam.position-cam.lookat).normalVec();
        yaxis={0,1,0};
        xaxis=yaxis.cross(zaxis).normalVec();
        yaxis=zaxis.cross(xaxis);

        if(false && !ImGuizmo::IsUsing() && !gui.anyWindowFocused() && mywindow->rightmousePressed && mywindow->rightDragVec!=MyGeo::Vec2f{0,0}) 
        {
            MyGeo::Vec3f rightDragVec={mywindow->rightDragVec,0};
            camPos+=0.2*(xaxis*(rightDragVec.x/(float)rhi->swapChainExtent.width)-yaxis*(rightDragVec.y/(float)rhi->swapChainExtent.height));
            lookat+=0.2*(xaxis*(rightDragVec.x/(float)rhi->swapChainExtent.width)-yaxis*(rightDragVec.y/(float)rhi->swapChainExtent.height));
            cam.lookat=lookat;
            cam.position=camPos;
        }

        cam.setNearFar(-0.1f,-50.f);
        // ubo_scene.near=-0.1f;
        // ubo_scene.far=-50.f;

        float fov=40-2*mywindow->mousescrollVal;
        cam.setFov(fov, rhi->swapChainExtent.width / (float)  rhi->swapChainExtent.height);
        cam.updateMat();
        ubo_scene.model=modelMat;
        ubo_scene.view=cam.viewMat;
        ubo_scene.proj=cam.projMat;
        ubo_scene.lightColor={lightIntensity*lightColor[0],lightIntensity*lightColor[1],lightIntensity*lightColor[2]};
        lightPos={std::sin(time_elapsed*3)*3,std::cos(time_elapsed*2)*3+6.f,std::cos(time_elapsed*3)*3};
        
        ubo_scene.lightPos=lightPos;
        ubo_scene.eyePos=cam.position;
        // void* data;

        lightCam.position=lightPos;
        lightCam.lookdirection=(lightCam.lookat-lightCam.position).normalVec();
        lightCam.setNearFar(-0.1,-50);
        lightCam.setFov(120,1);
        lightCam.updateMat();
        
        //model= identity
        ubo_shadow.depthMVP=lightCam.projMat*lightCam.viewMat;
        // ubo_shadow.depthMVP=lightCam.viewMat;

        ubo_shadow.sceneModel=ubo_scene.model;
        ubo_scene.lightMVP=ubo_shadow.depthMVP;

        uniformBuffers_scene[imageIndex].updateData(&ubo_scene,sizeof(ubo_scene));
        uniformBuffers_shadow[imageIndex].updateData(&ubo_shadow,sizeof(ubo_shadow));
    }

    void createPipelines() override
    {
        vulkanUtils.createPipelineLayout(descriptorSetLayout,&pipelineLayout_scene);
        vulkanUtils.createPipelineLayout(descriptorSetLayout,&pipelineLayout_shadow);

        pipeline_scene.createGraphicsPipeline("../shaders/scene.vert.spv","../shaders/scene.frag.spv",pipelineLayout_scene, texturePool.indexMap.size());

        pipeline_shadow.createdepthPipeline("../shaders/shadow.vert.spv",pipelineLayout_shadow,1);
        std::cout<<"finish createpipelines"<<std::endl;
    }



    void updateDescriptorSets(MyCommandBuffer& mycommmandbuffer, uint32_t imageIndex) override
    {
        uint32_t setIndex=imageIndex % rhi->imageCount;

        //这些info中包含了具体的buffer的信息，比如buffer句柄，descriptor大小和在buffer中的offset，如果buffer只对应一个descriptor则offset为0
        //将这些info传给VkWriteDescriptorSet结构体作为参数，和descriptorset联系起来
        VkDescriptorBufferInfo uniformBufferInfo_scene=descriptors.bufferInfo(uniformBuffers_scene[imageIndex].buffer,sizeof(UBO_scene));
        VkDescriptorBufferInfo uniformBufferInfo_shadow=descriptors.bufferInfo(uniformBuffers_shadow[imageIndex].buffer,sizeof(UBO_shadow));
        //! texturePool相关imageinfos直接在下面结构体给出

        VkDescriptorImageInfo shadowDepthImageInfo=descriptors.imageInfo(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,rhi->shadowImageView,rhi->shadowImageSampler);

        
        std::array<VkWriteDescriptorSet,3> descriptorWrites_scene{};
        descriptorWrites_scene[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_scene[0].dstSet = descriptorSets_scene[setIndex];
        descriptorWrites_scene[0].dstBinding = 0;
        descriptorWrites_scene[0].dstArrayElement = 0;
        descriptorWrites_scene[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites_scene[0].descriptorCount = 1;
        descriptorWrites_scene[0].pBufferInfo = &uniformBufferInfo_scene;

        //! for texturePool : an texture array
        descriptorWrites_scene[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_scene[1].dstSet = descriptorSets_scene[setIndex];
        descriptorWrites_scene[1].dstBinding = 1;
        descriptorWrites_scene[1].dstArrayElement = 0;
        descriptorWrites_scene[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites_scene[1].descriptorCount = texturePool.textures.size();
        descriptorWrites_scene[1].pImageInfo = texturePool.descriptorImageInfos.data();

        descriptorWrites_scene[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_scene[2].dstSet = descriptorSets_scene[setIndex];
        descriptorWrites_scene[2].dstBinding = 2;
        descriptorWrites_scene[2].dstArrayElement = 0;
        descriptorWrites_scene[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites_scene[2].descriptorCount = 1;
        descriptorWrites_scene[2].pImageInfo = &shadowDepthImageInfo;

        vkUpdateDescriptorSets(rhi->device,descriptorWrites_scene.size(),descriptorWrites_scene.data(),0,nullptr);

        std::array<VkWriteDescriptorSet,1> descriptorWrites_shadow{};
        descriptorWrites_shadow[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_shadow[0].dstSet = descriptorSets_shadow[setIndex];
        descriptorWrites_shadow[0].dstBinding = 0;
        descriptorWrites_shadow[0].dstArrayElement = 0;
        descriptorWrites_shadow[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites_shadow[0].descriptorCount = 1;
        descriptorWrites_shadow[0].pBufferInfo = &uniformBufferInfo_shadow;
        vkUpdateDescriptorSets(rhi->device,descriptorWrites_shadow.size(),descriptorWrites_shadow.data(),0,nullptr);
    }

    void createDescriptorSets() override
    {
        uint32_t textureNum=texturePool.textures.size();
        std::cout<<"textureNum: "<<textureNum<<std::endl;
        
        //!create pool
        std::vector<VkDescriptorPoolSize> poolsizes;
        poolsizes.push_back(descriptors.poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1000));
        poolsizes.push_back(descriptors.poolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1000));
        descriptors.createDescriptorPool(poolsizes,1000);

        //!create descriptorSetLayout
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.push_back(descriptors.setLayoutBinding(0,1,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));
        layoutBindings.push_back(descriptors.setLayoutBinding(1,textureNum,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
        layoutBindings.push_back(descriptors.setLayoutBinding(2,1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
        descriptors.createDescriptorSetLayout(descriptorSetLayout,layoutBindings);

        //!allocateDescriptorsets
        std::vector<VkDescriptorSetLayout> setLayouts(rhi->imageCount,descriptorSetLayout);
        descriptors.allocateDescriptorSets(setLayouts,descriptorSets_scene);
        descriptors.allocateDescriptorSets(setLayouts,descriptorSets_shadow);
    }

    void updateUi() override
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);

        ImGuiIO& io=ImGui::GetIO();
        bool t_open=false;
        ImGui::StyleColorsDark();
        ImGui::Begin("defaultTitle",nullptr,ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("%.1f fps", gui.fps);

	    ImGui::PushItemWidth(110.0f);
        int itemIndex;

        if(ImGui::CollapsingHeader("Load",ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* items[]={"extModel","geoModel"};
            // ImGui::Combo("type",&itemIndex,items,2,2);
            static const char* currentItem=items[0];
            if(ImGui::BeginCombo("Load Type",currentItem,ImGuiComboFlags_PopupAlignLeft))
            {
                bool selected=false;
                for(int i=0;i!=IM_ARRAYSIZE(items);++i)
                {
                    if(ImGui::Selectable(items[i],&selected))
                    {
                        currentItem=items[i];
                    }
                }
                ImGui::EndCombo();

            }

        }
        
        
        ImGui::PopItemWidth();
        ImGui::ColorEdit3("Light Color", lightColor, ImGuiColorEditFlags_NoAlpha);
        ImGui::SliderFloat("LightIntensity", &lightIntensity, 1.0f, 50.0f);

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
        
        int width,height;
        glfwGetFramebufferSize(mywindow->window,&width,&height);
        
        ImGuizmo::SetRect(0.f,0.f,(float)width,(float)height);
        auto view=cam.viewMat.col[0].data.data();
        auto proj=(cam.projMat*MyGeo::scaleMatrix({1,-1,1})).col[0].data.data();

        // ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::TRANSLATE,ImGuizmo::MODE::LOCAL,modelMat.col[0].data.data());
        // ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::ROTATE,ImGuizmo::MODE::LOCAL,modelMat.col[0].data.data());
        // ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::SCALE,ImGuizmo::MODE::LOCAL,modelMat.col[0].data.data());

        // ImGuizmo::ViewManipulate(view,)
        ImGuizmo::AllowAxisFlip(false);

        ImGui::End();
        ImGui::Render();
        gui.drawData=ImGui::GetDrawData();

    }


};


int main()
{
    // testModel();return 0;
    MyWindow mywindow(800,600);
    MyVulkanRHI rhi{&mywindow};
    MyRenderer renderer{&rhi};
    std::cout<<"starting init app"<<std::endl;
    ShadowMapTest app(&mywindow,&renderer);
    app.run();
}