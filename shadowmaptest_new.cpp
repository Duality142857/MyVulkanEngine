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
#include<source_location>
#include<mygeo/quaternion.h>
#include"../src/my_primitive.h"
#include"../src/my_entity.h"
namespace fs=std::filesystem;


float ga=10.f;
float eta=0.99;

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
    // MyGeo::Mat4f sceneModel;
};

static constexpr int storagebufferLen=512;
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
    // MyStorageBuffer storageBuffer{rhi,sizeof(int)*storagebufferLen};
    std::vector<MyStorageBuffer> storageBuffers_scene;
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
    // MyScene myscene{rhi,&texturePool};
    DynamicWorld dynamicWorld{rhi,&texturePool,&gui};
    MyModel* model;
    


    // MyModel model{"../resources/Marry.obj",rhi};
    // MyModel model{"../resources/iphonex/Iphone seceond version finished.obj",rhi};
    // MyModel model{"../resources/cube/cube.obj",rhi};//lost_empire.obj
    // MyModel model{"../resources/lostEmpire/lost_empire_tri.obj",rhi};//lost_empire.obj
    // MyModel model{"../resources/lpshead/head_triangle.obj",rhi};//lost_empire.obj
    // MyModel model{"../resources/tex-models/bunny.obj",rhi};//lost_empire.obj
    // MyModel model{"../resources/holodeck/holodeck_tri.obj",rhi};//lost_empire.obj

    RectModel rectmodel{rhi,{10,10}};
    BoxModel mybox{rhi,{2,5,1}};
    IcosSphereModel sphere{rhi,{3}};
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
        // loadAllTextures("../resources");

        texturePool.addTexture("../resources/mitsuba-icon.png");
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
        RigidBodyCreatInfo rbCI{};
        rbCI.L=MyGeo::Vec3f{0,0,0};
        rbCI.position=MyGeo::Vec3f{0,3,0};
        // rbCI.q.updateFromRotation(My);
        //basic box has lengths 1,1,1
        // dynamicWorld.addBox({1,1,1},rbCI,MyGeo::Vec3f{0,3,0},MyGeo::Rotation{MyGeo::Vec3f{1,2,3}.normalVec(),1});//,MyGeo::translate-Matrix({0,5,0})
        // dynamicWorld.addSphere({0.25},rbCI,MyGeo::Vec3f{1,3,0});
        // dynamicWorld.addSphere({0.3},rbCI,MyGeo::Vec3f{2,4,0});

        for(int i=0;i!=0;++i)
        {
            auto a1=getRand(0.2f,0.5f);
            auto a2=getRand(0.3f,0.6f);
            auto a3=getRand(0.3f,0.6f);
            // a1=1.f;
            rbCI.mass=a1*a2*a3;
            //MyGeo::Rotation{randomVec3(0.f,1.f),1}

            dynamicWorld.addBox({a1,a2,a3},rbCI,MyGeo::Vec3f{getRand(-4.f,4.f),getRand(1.f,4.f),getRand(-4.f,4.f)},MyGeo::Rotation{{1,1,1},getRand(0.f,1.f)});

            auto r=getRand(0.2f,0.5f);
            rbCI.mass=4.f*r*r*r;
            
            dynamicWorld.addSphere(
                {r},
                rbCI,
                MyGeo::Vec3f{getRand(-5.f,5.f),getRand(0.f,5.f),getRand(-5.f,5.f)}
                );

        }




        // dynamicWorld.addPlane({-1,-3,0},0);

        // dynamicWorld.addPlane({-1,-4,0},0);

        // dynamicWorld.addPlane({-1,0,0},1);

        // myscene.addBox({1,3,1},MyGeo::translateMatrix({-2,1.5,0}));//,MyGeo::translateMatrix({0,5,0})
        
        // myscene.addRect({10,10});
        // myscene.addSphere({1},MyGeo::translateMatrix({0,3,0}));
        // myscene.addExtModel("../resources/Marry.obj",MyGeo::translateMatrix({2,0,0}));
        // myscene.addExtModel("../resources/Marry.obj",MyGeo::translateMatrix({-2,0,0}));
        // myscene.addExtModel("../resources/lpshead/head_triangle.obj",MyGeo::translateMatrix({0,0,2}));
        // myscene.addExtModel("../resources/tex-models/bunny.obj",MyGeo::translateMatrix({0,0,-2}));
        // myscene.addExtModel("../resources/lostEmpire/lost_empire_tri.obj",MyGeo::translateMatrix({2,0,0}));

        // myscene.addExtModel("../resources/lpshead/head_triangle.obj",MyGeo::translateMatrix({0,0,-2}));
        // myscene.addExtModel("../resources/tex-models/bunny.obj",MyGeo::translateMatrix({0,0,2}));

        // myscene.addExtModel("../resources/Marry.obj",MyGeo::translateMatrix({-2,0,0}));
        // myscene.addExtModel("../resources/Marry.obj",MyGeo::translateMatrix({0,0,2}));
        // myscene.addExtModel("../resources/Marry.obj",MyGeo::translateMatrix({0,0,-2}));

        // model=new MyExtModel{"../resources/tex-models/bunny.obj",rhi,&texturePool};
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

            // model->bind(mycommandbuffer,0);
            // model->draw(mycommandbuffer);

            // mybox.bind(mycommandbuffer,0);
            // mybox.draw(mycommandbuffer);

            // spheremodel.bind(mycommandbuffer);
            // spheremodel.draw(mycommandbuffer);

            dynamicWorld.draw(mycommandbuffer,pipelineLayout_scene);

            // rectmodel.bind(mycommandbuffer);
            // rectmodel.draw(mycommandbuffer);
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

            // model->bind(mycommandbuffer);
            // model->draw(mycommandbuffer);

            // mybox.bind(mycommandbuffer,0);
            // mybox.draw(mycommandbuffer);

            // spheremodel.bind(mycommandbuffer);
            // spheremodel.draw(mycommandbuffer);
            // sphere.bind(mycommandbuffer);
            // sphere.draw(mycommandbuffer);
            // myscene.tick(t_delta,t_elapsed);
            dynamicWorld.draw(mycommandbuffer,pipelineLayout_scene);


            // rectmodel.bind(mycommandbuffer);
            // rectmodel.draw(mycommandbuffer);
            ImGui_ImplVulkan_RenderDrawData(gui.drawData,mycommandbuffer.commandbuffer);
            mycommandbuffer.endRenderPass();
        }
    }

    void tick(float t_delta, float t_elapsed) override 
    {
        // myscene.tick(t_delta,t_elapsed);
        dynamicWorld.tick(t_delta,t_elapsed);
        std::vector<uint32_t> d(storagebufferLen,0);
        void* data;
        storageBuffers_scene[0].map(&data);
        int* q=(int*)data;
        
        for(int i=0;i!=storagebufferLen;++i)
        {
            if(q[i]!=0)
            {
                if(rhi->mywindow->leftmousePressed)
                {
                    // std::cout<<"entity: "<<q[i]<<" selected!"<<std::endl;
                    dynamicWorld.selectedId=q[i];
                }
                break;
            }
        }
        std::memset(data,0,storagebufferLen*sizeof(uint32_t));
        storageBuffers_scene[0].unmap();


        // rhi->copyBuffer2host(storageBuffers_scene[0].buffer,d.data(),storageBuffers_scene[0].size);
        // for(auto& x:d) 
        // {
        //     if(x!=0)
        //     {
        //         if(rhi->mywindow->leftmousePressed)
        //         {
        //             std::cout<<"entiti: "<<x<<" selected!"<<std::endl;
        //         }
        //         break;
        //     }    
        // }
    }
    void addEnvironment()
    {
        dynamicWorld.addPlane({0,1,0},0);
        dynamicWorld.addPlane({0,-1,0},10);

        dynamicWorld.addPlane({0,0,1},5);
        dynamicWorld.addPlane({0,0,-1},5);

        dynamicWorld.addPlane({1,0,0},5);
        dynamicWorld.addPlane({-1,0,0},5);
    }

    void prepareData() override
    {
        //* load models(creating vertexbuffers and indexbuffers)
        addEnvironment();
        //* create uniform buffers
        VkDeviceSize bufferSize_scene = sizeof(UBO_scene);
        VkDeviceSize bufferSize_shadow=sizeof(UBO_shadow);
        VkDeviceSize storageBufferSize=sizeof(uint32_t)*storagebufferLen;
        for(int i=0;i!=rhi->images.size();++i) 
        {
            uniformBuffers_scene.push_back({rhi,bufferSize_scene});
            uniformBuffers_shadow.push_back({rhi,bufferSize_shadow});
            storageBuffers_scene.push_back({rhi,storageBufferSize});
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
        if(!gui.anyWindowFocused())
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

        // ubo_shadow.sceneModel=ubo_scene.model;
        ubo_scene.lightMVP=ubo_shadow.depthMVP;

        uniformBuffers_scene[imageIndex].updateData(&ubo_scene,sizeof(ubo_scene));
        uniformBuffers_shadow[imageIndex].updateData(&ubo_shadow,sizeof(ubo_shadow));


        
    }

    void createPipelines() override
    {
        vulkanUtils.createPipelineLayout(descriptorSetLayout,&pipelineLayout_scene);
        vulkanUtils.createPipelineLayout(descriptorSetLayout,&pipelineLayout_shadow);

        pipeline_scene.createGraphicsPipeline("../shaders/scene_new.vert.spv","../shaders/scene_new.frag.spv",pipelineLayout_scene, texturePool.indexMap.size());

        pipeline_shadow.createdepthPipeline("../shaders/shadow_new.vert.spv",pipelineLayout_shadow,1);
        std::cout<<"finish createpipelines"<<std::endl;
    }



    void updateDescriptorSets(MyCommandBuffer& mycommmandbuffer, uint32_t imageIndex) override
    {
        uint32_t setIndex=imageIndex % rhi->imageCount;

        //??????info?????????????????????buffer??????????????????buffer?????????descriptor????????????buffer??????offset?????????buffer???????????????descriptor???offset???0
        //?????????info??????VkWriteDescriptorSet???????????????????????????descriptorset????????????
        VkDescriptorBufferInfo uniformBufferInfo_scene=descriptors.bufferInfo(uniformBuffers_scene[imageIndex].buffer,sizeof(UBO_scene));
        VkDescriptorBufferInfo uniformBufferInfo_shadow=descriptors.bufferInfo(uniformBuffers_shadow[imageIndex].buffer,sizeof(UBO_shadow));

        VkDescriptorBufferInfo storageBufferInfo_scene=descriptors.bufferInfo(storageBuffers_scene[imageIndex].buffer,storageBuffers_scene[imageIndex].size);

        //! texturePool??????imageinfos??????????????????????????????

        VkDescriptorImageInfo shadowDepthImageInfo=descriptors.imageInfo(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,rhi->shadowImageView,rhi->shadowImageSampler);

        
        std::array<VkWriteDescriptorSet,4> descriptorWrites_scene{};
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

        descriptorWrites_scene[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites_scene[3].dstSet = descriptorSets_scene[setIndex];
        descriptorWrites_scene[3].dstBinding = 3;
        descriptorWrites_scene[3].dstArrayElement = 0;
        descriptorWrites_scene[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites_scene[3].descriptorCount = 1;
        descriptorWrites_scene[3].pBufferInfo = &storageBufferInfo_scene;  

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
        poolsizes.push_back(descriptors.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,10));

        descriptors.createDescriptorPool(poolsizes,1000);

        //!create descriptorSetLayout
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.push_back(descriptors.setLayoutBinding(0,1,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));
        layoutBindings.push_back(descriptors.setLayoutBinding(1,textureNum,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));
        layoutBindings.push_back(descriptors.setLayoutBinding(2,1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT));

        layoutBindings.push_back(descriptors.setLayoutBinding(3,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));

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
        ImGui::SliderFloat("gravity",&ga,0.f,200.f);
        ImGui::SliderFloat("eta",&eta,0.f,1.f);

        RigidBodyCreatInfo rbCI{};


        if(ImGui::Button("Add Sphere"))
        {
            // auto r=getRand(0.2f,0.5f);
            auto r=0.5f;
            rbCI.mass=4.f/3*r*r*r;
            dynamicWorld.addSphere({r},rbCI,
                MyGeo::Vec3f{0.f,4.f,0.f});
        }

        ImGui::SameLine();

        if(ImGui::Button("Add Box"))
        {
            auto a1=getRand(0.2f,0.5f);
            auto a2=getRand(0.3f,0.6f);
            auto a3=getRand(0.3f,0.6f);
            rbCI.mass=a1*a2*a3;
            dynamicWorld.addBox({a1,a2,a3},rbCI,
                MyGeo::Vec3f{0.f,5.f,0.f},MyGeo::Rotation{{0,1,0},getRand(0.f,1.f)});
        }

        ImGui::SameLine();
        if(ImGui::Button("Clear objects"))
        {
            dynamicWorld.entities.clear();
            addEnvironment();
        }

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
        
        int width,height;
        glfwGetFramebufferSize(mywindow->window,&width,&height);
        
        ImGuizmo::SetRect(0.f,0.f,(float)width,(float)height);
        auto view=cam.viewMat.col[0].data.data();
        auto proj=(cam.projMat*MyGeo::scaleMatrix({1,-1,1})).col[0].data.data();

        // ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::TRANSLATE,ImGuizmo::MODE::LOCAL,myscene.models[0]->modelMat.col[0].data.data());
        // ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::ROTATE,ImGuizmo::MODE::LOCAL,myscene.models[0]->modelMat.col[0].data.data());
        // ImGuizmo::Manipulate(view,proj,ImGuizmo::OPERATION::SCALE,ImGuizmo::MODE::LOCAL,modelMat.col[0].data.data());

        // ImGuizmo::ViewManipulate(view,)
        ImGuizmo::AllowAxisFlip(false);
        bool showDemoWindow=true;
        ImGui::ShowDemoWindow(&showDemoWindow);


        ImGui::End();
        ImGui::Render();
        gui.drawData=ImGui::GetDrawData();

    }


};


int main()
{

    // MyEntity e1,e2;
    // e1.mass=2;e2.mass=1;
    // e1.velocity=MyGeo::Vec3f{1,0,0};
    // e2.velocity=MyGeo::Vec3f{-1,0,0};
    // e1.position=MyGeo::Vec3f{0.8,0,1.2};
    // e2.position=MyGeo::Vec3f{1.2,0,0.8};

    // e1.I_inv_0=MyGeo::Eye<float,3>();
    // e2.I_inv_0=MyGeo::Eye<float,3>();


    
    
    // CollisionRecord record;
    // record.normal=MyGeo::Vec3f{1,0,-1}.normalVec();
    // record.point=MyGeo::Vec3f{1,1};

    // elasticCollide(e1,e2,record); return 0;
    // elasticCollide(e1,e2);
    // std::cout<<e1.velocity<<" "<<e2.velocity<<std::endl; return 0;
    



    MyWindow mywindow(1200,800);
    MyVulkanRHI rhi{&mywindow};

    MyRenderer renderer{&rhi};
    std::cout<<"starting init app"<<std::endl;
    ShadowMapTest app(&mywindow,&renderer);
    app.run();
}