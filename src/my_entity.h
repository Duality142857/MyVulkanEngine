#pragma once 
#include<mygeo/component.h>
#include"../src/my_model.h"
#include"../src/my_geomodel.h"
#include<mygeo/primitive.h>
#include<mygeo/rigidbody.h>
#include<mygeo/obb.h>


/**
 * @brief 
 * id
 * @arg id 
 * @arg renderModel: mesh to render, bind and draw functions
 * @arg collider: geometric description and intersection methods of primitive
 * @arg rigidbody: macroscopic states of rigidbody, including position, orientation, mass, velocity ....
 */
class MyEntity
{
public:
    int id=-1;
    std::shared_ptr<MyModel> renderModel;
    std::shared_ptr<MyGeo::Primitive> collider;
    std::shared_ptr<MyGeo::RigidBody> rigidBody;

    MyGeo::Vec3f translation{0,0,0};
    MyGeo::Rotation rotation{MyGeo::Vec3f{0,1,0},0}; 

    MyGeo::Vec3f velocity{0,0,0};
    //barycenter
    MyGeo::Vec3f position{0,0,0};
    MyGeo::Mat3f I_inv_0;

    MyGeo::Vec3f L{0,0,0};
    // MyGeo::Quaternion<float> q{0,0,0,0};
    std::vector<MyGeo::Vec3f> positions;
    float mass;
    MyGeo::Vec3f netforce{0,0,0};
    bool gravityFlag;
    bool dynamicFlag;

    MyEntity(bool _dynamicFlag=true, bool _gravityFlag=true):dynamicFlag{_dynamicFlag},gravityFlag{_gravityFlag},mass{std::numeric_limits<float>::max()}
    {}

    /**
     * @brief update velocity position and orientation(as quaternion)
     * 
     * @param deltaT 
     */
    void tick(float deltaT)
    {   
        if(!dynamicFlag)
        {
            return;
        }
        //! linear
        static const float g=0.002f;
        if(gravityFlag)
        {
            MyGeo::Vec3f gravityForce=mass*g*MyGeo::Vec3f{0,-1,0};//N
            netforce=gravityForce;
        }


        velocity+=(deltaT/mass)*netforce;
        position+=deltaT*velocity;
        std::cout<<"position "<<position<<std::endl;
        std::cout<<deltaT<<" "<<velocity<<std::endl;

        //! angular
        MyGeo::Mat3f R=q.toMat3();
        // MyGeo::Vec3f tau{0,0,0};
        // for(auto& r:positions)
        // {
        //     MyGeo::Vec3f f{0,0,0};
        //     tau+=(R*r).cross(f);
        // }
        
        auto I_inv=R*I_inv_0*R.transpose();
        // L+=deltaT*tau;

        MyGeo::Vec3f omega=I_inv*L;
        q+=Quaternion<float>{0,0.5f*deltaT*omega}*q;
        q=q.noramlQuaternion();
        // std::cout<<"L: "<<L<<std::endl;
        // std::cout<<"position: "<<position<<"L: "<<omega.norm()<<std::endl;
        // std::cout<<"omega: "<<omega<<std::endl;


    }


    MyEntity(){}
    MyEntity(const MyGeo::Vec3f& _translation, const MyGeo::Rotation& _rotation):translation{_translation},rotation{_rotation}
    {
        
    }

    void addModel(std::shared_ptr<MyModel> model)
    {
        renderModel=model;
    }
    void addRigidBody(std::shared_ptr<MyGeo::RigidBody> _rigidBody)
    {
        rigidBody=_rigidBody;
    }
    void addCollisionPrimitive(std::shared_ptr<MyGeo::Primitive> _collider)
    {
        collider=_collider;
    }
};

struct RigidBodyCreatInfo
{
    float mass=1.f;
    MyGeo::Vec3f position{0,0,0};
    MyGeo::Vec3f velocity{0,0,0};
    MyGeo::Vec3f L{0,0,0};
    MyGeo::Quaternion<float> q{1,0,0,0};
    // bool gravityFlag=false;
};

/**
 * @brief 
 * 
 * 
 */
class DynamicWorld
{
    MyVulkanRHI* rhi;
    MyTexturePool* texturePool;
public:
    std::vector<std::shared_ptr<MyEntity>> entities;
    // std::vector<MyGeo::Mat4f> modelMats;

    DynamicWorld(MyVulkanRHI* _rhi, MyTexturePool* _texturePool):rhi{_rhi},texturePool{_texturePool}
    {}

    void addBox(const std::initializer_list<float>& scales,const RigidBodyCreatInfo& rbCI,const MyGeo::Vec3f& translation, const MyGeo::Rotation& rotation)
    {
        assert(scales.size()==3);
        auto ptr=scales.begin();
        MyGeo::Vec3f extent{*ptr,*(ptr+1),*(ptr+2)};

        auto entity=std::make_shared<MyEntity>(translation, rotation);
        
        auto modelMat=MyGeo::translateMatrix(translation)*rotation.mat4();
        

        //add renderable
        entity->addModel(std::make_shared<BoxModel>(rhi,scales,modelMat));

        //add rigidbody(without collision shape)
        //setting rigidbody states
        auto rb= std::make_shared<MyGeo::RigidBody>();
        rb->mass=rbCI.mass;
        rb->position=rbCI.position;
        rb->velocity=rbCI.velocity;
        rb->L=rbCI.L;
        rb->q=rbCI.q;
        rb->I_inv_0=MyGeo::Mat3f{
            MyGeo::Vec3f{extent.y*extent.y+extent.z*extent.z,0,0},
            {0,extent.x*extent.x+extent.z*extent.z,0},
            {0,0,extent.x*extent.x+extent.y*extent.y}
        };
        rb->I_inv_0*=rbCI.mass/3.f;
        entity->addRigidBody(rb);

        //add collider
        auto obb=std::make_shared<MyGeo::OBB>();
        obb->extent=extent;
        obb->center=rbCI.position;
        obb->axis[0]=MyGeo::Vec3f{1,0,0};
        obb->axis[1]=MyGeo::Vec3f{0,1,0};
        obb->axis[2]=MyGeo::Vec3f{0,0,1};

        entity->addCollisionPrimitive(obb);

        entities.push_back(entity);

    }

    void addPlane(const MyGeo::Vec3f& _normal, float d)
    {
        //rotation axis
        auto normal=_normal.normalVec();
        std::cout<<"normal: "<<normal<<std::endl;

        MyGeo::Vec3f a=-normal.cross(MyGeo::Vec3f{0,1,0});

        std::cout<<a<<std::endl;

        if(a.norm2()<epsilon_sqr)
        {
            a={0,1,0};

        }
        else 
        {
            a.normalize();
        }

        //!get rotation matrix given start axis and end axis
        auto rotMat=MyGeo::rotationMatrix4f(MyGeo::Vec3f{0,1,0},normal);

        auto modelMat=MyGeo::translateMatrix(-d*normal)*rotMat;
        
        std::cout<<modelMat<<std::endl;
        // modelMat.col[3]=MyGeo::Vec4f{-d*normal,1};
        std::cout<<"disp: "<<-d*normal<<std::endl;
        std::cout<<modelMat<<std::endl;

        auto entity=std::make_shared<MyEntity>();
        std::initializer_list<float> scales={8,8};
        entity->addModel(std::make_shared<RectModel>(rhi,scales,modelMat));

        auto rb= std::make_shared<MyGeo::RigidBody>(false,false);
        // rb->mass=rbCI.mass;
        rb->position=-d*normal;
        // rb->q=q;
        // rb->I_inv_0*=rbCI.mass/3.f;
        entity->addRigidBody(rb);

        auto obb=std::make_shared<MyGeo::OBB>();

        entity->addCollisionPrimitive(obb);

        entities.push_back(entity);
    }

    void addRect(const std::initializer_list<float>& scales,RigidBodyCreatInfo& rbCI,const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>())
    {
        assert(scales.size()==2);
        auto entity=std::make_shared<MyEntity>();
        entity->addModel(std::make_shared<RectModel>(rhi,scales,_modelMat));
        auto rb= std::make_shared<MyGeo::RigidBody>(false);
        // rb->mass=rbCI.mass;
        rb->position=rbCI.position;
        // rb->velocity=rbCI.velocity;
        // rb->L=rbCI.L;
        rb->q=rbCI.q;
        // rb->I_inv_0=MyGeo::Mat3f{
        //     MyGeo::Vec3f{extent.y*extent.y+extent.z*extent.z,0,0},
        //     {0,extent.x*extent.x+extent.z*extent.z,0},
        //     {0,0,extent.x*extent.x+extent.y*extent.y}
        // };
        rb->I_inv_0*=rbCI.mass/3.f;
        entity->addRigidBody(rb);

        auto obb=std::make_shared<MyGeo::OBB>();

        entity->addCollisionPrimitive(obb);

        entities.push_back(entity);

    }

    void draw(MyCommandBuffer& myCommandbuffer, VkPipelineLayout pipelineLayout)
    {
        for(auto& entity:entities)
        {
            vkCmdPushConstants(
                myCommandbuffer.commandbuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(PushConstantStruct),
                entity->renderModel->modelMat.col[0].data.data()
                );
            entity->renderModel->bind(myCommandbuffer);
            entity->renderModel->draw(myCommandbuffer);
        }
    }

    void tick(float deltaT)
    {
        for(int i=0;i!=entities.size();++i)
        {
            // std::cout<<"tick "<<i<<std::endl;
            auto& rmat=entities[i]->renderModel->modelMat;
            entities[i]->rigidBody->tick(deltaT);
            
  
            // switch (entities[i]->collider->type())
            // {
            //     case MyGeo::PrimType::OBBox:
            //     {
            //         auto obb=std::static_pointer_cast<MyGeo::OBB>(entities[i]->collider);
            //         MyGeo::Plane p{MyGeo::Vec3f{0,1,0},0};
            //         auto flag=obb->abovePlane(p);
            //         if(flag)
            //         {
            //             std::cout<<"collide!"<<std::endl;
            //             entities[i]->rigidBody->velocity=-entities[i]->rigidBody->velocity;
            //         }
            //         break;
            //     }
            
            //     default:
            //         break;
            // }

            if(entities[i]->rigidBody->dynamicFlag) 
            {
                rmat=MyGeo::translateMatrix(entities[i]->rigidBody->position)*rmat*entities[i]->rigidBody->q.toMat4();
                std::cout<<rmat<<std::endl;
            }    
            
        }
    }
};


