#pragma once 
#include<mygeo/component.h>
#include"../src/my_model.h"
#include"../src/my_geomodel.h"
#include<mygeo/primitive.h>
#include<mygeo/rigidbody.h>
#include<mygeo/obb.h>
#include"../core/myrandom.h"
#include"../src/my_gui.h"
#include<map>


extern float ga;
extern float eta;

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
    // bool Lflag=false;
    int id=-1;
    std::shared_ptr<MyModel> renderModel;
    std::shared_ptr<MyGeo::Primitive> collider;
    bool dealt=false;
    // std::shared_ptr<MyGeo::RigidBody> rigidBody;

    // MyGeo::Vec3f translation{0,0,0};

    MyGeo::Rotation rotation{MyGeo::Vec3f{0,1,0},0}; 

    MyGeo::Vec3f velocity{0,0,0};
    //!barycenter, care that barycenter may not be model local center, here we assume they are the same for now
    MyGeo::Vec3f position{0,0,0};
    MyGeo::Mat3f I_inv_0;
    MyGeo::Vec3f L{0,0,0};
    // MyGeo::Quaternion<float> q{0,0,0,0};
    // std::vector<MyGeo::Vec3f> positions;
    float mass;
    MyGeo::Vec3f netforce{0,0,0};
    bool gravityFlag;
    bool dynamicFlag;
    bool selectable=true;



    MyEntity(){}
    MyEntity(
        const MyGeo::Vec3f& _position, 
        const MyGeo::Rotation& _rotation, 
        bool _dynamicFlag=true, 
        bool _gravityFlag=true
    ):
    position{_position},
    rotation{_rotation},
    dynamicFlag{_dynamicFlag},
    gravityFlag{_gravityFlag},
    mass{1.f}
    {}

    /**
     * @brief update velocity position and orientation(as quaternion)
     * 
     * @param deltaT 
     */
    void tick(float deltaT,const MyGeo::Vec3f& force)
    {   
        if(!dynamicFlag)
        {
            return;
        }
        netforce=force;
        //! linear

        if(gravityFlag)
        {
            MyGeo::Vec3f gravityForce=mass*ga*MyGeo::Vec3f{0,-1,0};//N
            netforce+=gravityForce;
        }

        // L*=0.999;
        // velocity*=0.999;

        velocity+=(deltaT/mass)*netforce;
        position+=deltaT*velocity;
        //! angular
        MyGeo::Mat3f R=rotation.mat3();
    
        auto I_inv=R*I_inv_0*R.transpose();
        MyGeo::Vec3f omega=I_inv*L;

        rotation.q+=MyGeo::Quaternion<float>{0,0.5f*deltaT*omega}*rotation.q;
        rotation.q.normalize();
        // std::cout<<"velocity: "<<velocity<<std::endl;


    }

    void addModel(std::shared_ptr<MyModel> model)
    {
        renderModel=model;
    }
    // void addRigidBody(std::shared_ptr<MyGeo::RigidBody> _rigidBody)
    // {
    //     rigidBody=_rigidBody;
    // }

    void addCollisionPrimitive(std::shared_ptr<MyGeo::Primitive> _collider)
    {
        collider=_collider;
    }
};


static MyGeo::Vec3f elasticCollide(MyEntity& e1, MyEntity& e2)
{
    if(e1.dealt) return MyGeo::Vec3f{0,0,0};
    e1.dealt=true;
    e2.dealt=true;
    auto momentum0=e1.velocity*e1.mass+e2.velocity*e2.mass;
    auto energy0=e1.velocity.norm2()*e1.mass+e2.velocity.norm2()*e2.mass;
    float t1=2.f*e2.mass/(e1.mass+e2.mass);
    float t2=2.f*e1.mass/(e1.mass+e2.mass);

    MyGeo::Vec3f v12=e1.velocity-e2.velocity;
    MyGeo::Vec3f r12=e1.position-e2.position;

    float d2_reci=1.f/r12.norm2();
    auto vt=v12.dot(r12)*d2_reci*r12;
    auto v1new=e1.velocity-2.f*e2.mass/(e1.mass+e2.mass)*vt;
    auto v2new=e2.velocity+2.f*e1.mass/(e1.mass+e2.mass)*vt;
    MyGeo::Vec3f dp=e1.mass*(v1new-e1.velocity);
    e1.velocity=v1new;
    e2.velocity=v2new;
    
    auto momentum1=e1.velocity*e1.mass+e2.velocity*e2.mass;

    auto energy1=e1.velocity.norm2()*e1.mass+e2.velocity.norm2()*e2.mass;

    return dp;

}


struct RigidBodyCreatInfo
{
    float mass=1.f;
    MyGeo::Vec3f position{0,0,0};
    MyGeo::Vec3f velocity{0,0,0};
    MyGeo::Vec3f L{0,0,0};
    MyGeo::Quaternion<float> q{1,0,0,0};
    // bool gravityFlag=false;
};

struct CollisionRecord
{
    MyGeo::Vec3f point;
    MyGeo::Vec3f normal;
};


static void elasticCollide(MyEntity& e1, MyEntity& e2, CollisionRecord& rec)
{
    MyGeo::Mat3f R1=e1.rotation.mat3();
    MyGeo::Mat3f R2=e2.rotation.mat3();

    auto I_inv_1=R1*e1.I_inv_0*R1.transpose();
    auto I_inv_2=R2*e2.I_inv_0*R2.transpose();
    
    auto nominater=-(1+eta)*(e1.velocity-e2.velocity).dot(rec.normal);
    auto denom1=1.f/e1.mass+1.f/e2.mass;
    auto Ra=rec.point-e1.position;
    auto Rb=rec.point-e2.position;
    auto denom2=rec.normal.dot(I_inv_1*(Ra.cross(rec.normal)).cross(Ra));
    auto denom3=rec.normal.dot(I_inv_2*(Rb.cross(rec.normal)).cross(Rb));

    auto J=nominater/(denom1+denom2+denom3)*rec.normal;
    
    e1.velocity+=1.f/e1.mass*J;
    e2.velocity-=1.f/e2.mass*J;

    e1.L+=Ra.cross(J);
    e2.L+=Rb.cross(J);
}

static void elasticCollide_Plane(MyEntity& e1, CollisionRecord& rec)
{
    if(e1.velocity.dot(rec.normal)<0.05f && e1.velocity.dot(rec.normal)>-0.05f)
    {
        e1.L=e1.L.dot(rec.normal)*rec.normal;
        e1.L*=eta;
        e1.velocity=e1.velocity-e1.velocity.dot(rec.normal)*rec.normal;
        e1.velocity*=eta;
        return;
    }
    MyGeo::Mat3f R1=e1.rotation.mat3();
    auto I_inv_1=R1*e1.I_inv_0*R1.transpose();

    auto nominater=-(1+eta)*e1.velocity.dot(rec.normal);
    
    auto denom1=1.f/e1.mass;
    auto Ra=rec.point-e1.position;
    auto denom2=rec.normal.dot(I_inv_1*(Ra.cross(rec.normal)).cross(Ra));

    auto J=nominater/(denom1+denom2)*rec.normal;
    
    e1.velocity+=1.f/e1.mass*J;
    e1.L+=Ra.cross(J);
    if(e1.L.norm2()<0.001f) 
    {
        e1.L.x=0;e1.L.y=0;e1.L.z=0;
    }

}

/**
 * @brief 
 * 
 * 
 */
class DynamicWorld
{
    MyVulkanRHI* rhi;
    MyTexturePool* texturePool;
    MyGui* gui;

public:
    std::vector<std::shared_ptr<MyEntity>> entities;
    uint32_t selectedId=0;
    // std::vector<MyGeo::Mat4f> modelMats;
    std::map<int,std::string> objInfoMap;


    DynamicWorld(MyVulkanRHI* _rhi, MyTexturePool* _texturePool, MyGui* _gui):rhi{_rhi},texturePool{_texturePool},gui{_gui}
    {}

    void addBox(const std::initializer_list<float>& scales,const RigidBodyCreatInfo& rbCI,const MyGeo::Vec3f& translation, const MyGeo::Rotation& rotation)
    {
        assert(scales.size()==3);
        auto ptr=scales.begin();
        MyGeo::Vec3f extent{*ptr,*(ptr+1),*(ptr+2)};

        auto entity=std::make_shared<MyEntity>(translation, rotation);
        
        entity->velocity=randomVec3(-5.f,5.f);


        auto modelMat=MyGeo::translateMatrix(translation)*rotation.mat4();

        entity->I_inv_0=MyGeo::Mat3f{
            MyGeo::Vec3f{extent.y*extent.y+extent.z*extent.z,0,0},
            {0,extent.x*extent.x+extent.z*extent.z,0},
            {0,0,extent.x*extent.x+extent.y*extent.y}
        };
        entity->I_inv_0*=rbCI.mass/3.f;
        entity->I_inv_0=entity->I_inv_0.inverse();

        // randomVec3(0.f,200.f);
        // entity->L=randomVec3(0.f,1.f);
        // entity->L=MyGeo::Vec3f{30,100,60};
        entity->L=MyGeo::Vec3f{0,0,0};

        entity->mass=rbCI.mass;

        //add renderable
        entity->addModel(std::make_shared<BoxModel>(rhi,scales,modelMat));

        //add rigidbody(without collision shape)
        //setting rigidbody states
        // auto rb= std::make_shared<MyGeo::RigidBody>();
        // rb->mass=rbCI.mass;
        // rb->position=rbCI.position;
        // rb->velocity=rbCI.velocity;
        // rb->L=rbCI.L;
        // rb->q=rbCI.q;
        // rb->I_inv_0=MyGeo::Mat3f{
        //     MyGeo::Vec3f{extent.y*extent.y+extent.z*extent.z,0,0},
        //     {0,extent.x*extent.x+extent.z*extent.z,0},
        //     {0,0,extent.x*extent.x+extent.y*extent.y}
        // };
        // rb->I_inv_0*=rbCI.mass/3.f;
        // entity->addRigidBody(rb);

        //add collider
        auto obb=std::make_shared<MyGeo::OBB>();
        obb->extent=0.5*extent;
        // obb->center=rbCI.position;
        obb->center=entity->position;
        obb->axis[0]=MyGeo::Vec3f{1,0,0};
        obb->axis[1]=MyGeo::Vec3f{0,1,0};
        obb->axis[2]=MyGeo::Vec3f{0,0,1};

        entity->addCollisionPrimitive(obb);

        entities.push_back(entity);
        entity->id=entities.size();
        objInfoMap.insert(std::make_pair(entity->id,"box"));
        
        // std::cout<<"entity pos: "<<entity->position<<std::endl;

    }

    void addSphere(const std::initializer_list<float>& scales,const RigidBodyCreatInfo& rbCI,const MyGeo::Vec3f& translation)
    {
        

        auto ptr=scales.begin();
        auto entity=std::make_shared<MyEntity>(translation,MyGeo::Rotation{{0,1,0},0});
        entity->velocity=randomVec3(-4.f,4.f);
        // entity->velocity=MyGeo::Vec3f{0,0,0};
        // entity->velocity.y=0.f;

        auto modelMat=MyGeo::translateMatrix(translation);
        
        // std::cout<<*ptr<<" "<<translation<<std::endl;

        entity->I_inv_0=MyGeo::Mat3f{
            MyGeo::Vec3f{1,0,0},
            {0,1,0},
            {0,0,1}
        };
        entity->I_inv_0*=rbCI.mass;
        entity->L=MyGeo::Vec3f{0,0,0};
        entity->mass=rbCI.mass;
        entity->addModel(std::make_shared<IcosSphereModel>(rhi,scales,modelMat));

        auto sphere=std::make_shared<MyGeo::Sphere>(translation,*ptr);

        entity->addCollisionPrimitive(sphere);

        entities.push_back(entity);
        entity->id=entities.size();

        objInfoMap.insert(std::make_pair(entity->id,"sphere"));
        
    }

    void addPlane(const MyGeo::Vec3f& _normal, float d, const MyGeo::Vec2f& _size={10,10})
    {
        //rotation axis
        auto normal=_normal.normalVec();
        

        MyGeo::Vec3f a=-normal.cross(MyGeo::Vec3f{0,1,0});

        if(a.norm2()<epsilon_sqr)
        {
            a={0,1,0};

        }
        else 
        {
            a.normalize();
        }

        //!get rotation matrix given start axis and end axis
        // auto rotMat=MyGeo::rotationMatrix4f(MyGeo::Vec3f{0,1,0},normal);

        MyGeo::Rotation rotation{MyGeo::Vec3f{0,1,0},normal};

        auto modelMat=MyGeo::translateMatrix(-d*normal)*rotation.mat4();

        // auto entity=std::make_shared<MyEntity>(normal,rotation,false,false);
        auto entity=std::make_shared<MyEntity>(-d*normal, rotation, false,false);


        std::initializer_list<float> scales={_size.x,_size.y};

        entity->addModel(std::make_shared<RectModel>(rhi,scales,modelMat));

        auto rb= std::make_shared<MyGeo::RigidBody>(false,false);
        // rb->mass=rbCI.mass;
        rb->position=-d*normal;
        // rb->q=q;
        // rb->I_inv_0*=rbCI.mass/3.f;
        // entity->addRigidBody(rb);

        // auto obb=std::make_shared<MyGeo::OBB>();
        
        auto plane=std::make_shared<MyGeo::Plane>(normal,d);

        entity->addCollisionPrimitive(plane);
        entity->id=entities.size()+1;
        entity->selectable=false;
        entities.push_back(entity);
        objInfoMap.insert(std::make_pair(entity->id,"plane"));

        
    }

    // void addRect(const std::initializer_list<float>& scales,RigidBodyCreatInfo& rbCI,const MyGeo::Mat4f& _modelMat=MyGeo::Eye<float,4>())
    // {
    //     assert(scales.size()==2);
    //     auto entity=std::make_shared<MyEntity>(false,false);
    //     entity->addModel(std::make_shared<RectModel>(rhi,scales,_modelMat));
    //     auto rb= std::make_shared<MyGeo::RigidBody>(false);
    //     // rb->mass=rbCI.mass;
    //     rb->position=rbCI.position;
    //     // rb->velocity=rbCI.velocity;
    //     // rb->L=rbCI.L;
    //     rb->q=rbCI.q;
    //     // rb->I_inv_0=MyGeo::Mat3f{
    //     //     MyGeo::Vec3f{extent.y*extent.y+extent.z*extent.z,0,0},
    //     //     {0,extent.x*extent.x+extent.z*extent.z,0},
    //     //     {0,0,extent.x*extent.x+extent.y*extent.y}
    //     // };
    //     rb->I_inv_0*=rbCI.mass/3.f;
    //     entity->addRigidBody(rb);

    //     auto obb=std::make_shared<MyGeo::OBB>();

    //     entity->addCollisionPrimitive(obb);

    //     entities.push_back(entity);

    // }

    void draw(MyCommandBuffer& myCommandbuffer, VkPipelineLayout pipelineLayout)
    {
        
        for(auto& entity:entities)
        {
            
            PushConstantStruct pushConstant;
            pushConstant.modelMat=MyGeo::translateMatrix(entity->position)*entity->rotation.mat4();
            pushConstant.mousePos=rhi->mywindow->cursorPos();
            pushConstant.objId=entity->id;
            if(rhi->mywindow->rightmousePressed) selectedId=0;
            pushConstant.selectedId= gui->anyWindowFocused() || rhi->mywindow->leftmousePressed || !entity->selectable ? 0:selectedId;
            
            vkCmdPushConstants(
                myCommandbuffer.commandbuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushConstantStruct),
                &pushConstant
                // (MyGeo::translateMatrix(entity->position)*entity->rotation.mat4()).col[0].data.data()
                );
            entity->renderModel->bind(myCommandbuffer);
            entity->renderModel->draw(myCommandbuffer);

            // std::cout<<MyGeo::translateMatrix(entity->position)*entity->rotation.mat4()<<std::endl;
                // entity->renderModel->modelMat.col[0].data.data()


        }
    }

    static MyGeo::Vec3f baseAxis(int i)
    {
        MyGeo::Vec3f a{0,0,0};
        a[i]=1;
        return a;
    }



    void tick(float deltaT,float t_elapsed)
    {
        for(auto& e:entities)
        {
            e->dealt=false;
        }
        for(int i=0;i!=entities.size();++i)
        {
            auto force=MyGeo::Vec3f{0,0,0};
            
            MyGeo::Vec3f averageContactPoint{0,0,0};
            MyGeo::Vec3f dp{0,0,0};
            
            switch (entities[i]->collider->type())
            {
                case MyGeo::PrimType::OBBox:
                {
                    auto obb=std::static_pointer_cast<MyGeo::OBB>(entities[i]->collider);
                    obb->center=entities[i]->position;
                    

                    for(int j=0;j!=3;++j)
                    {
                        obb->axis[j]=entities[i]->rotation.mat3()*baseAxis(j);
                    }

                    for(int k=0;k!=entities.size();++k)
                    {
                        if(i==k) continue;
                        auto& e=entities[k];
                        switch(e->collider->type())
                        {
                            case MyGeo::PrimType::PLANE:
                            {
                                auto plane=std::static_pointer_cast<MyGeo::Plane>(e->collider);
                                float deepness;
                                auto flag=obb->abovePlane(*plane,deepness,averageContactPoint);
                                if(!flag)
                                {
                                    CollisionRecord rec;
                                    rec.point=averageContactPoint;
                                    rec.normal=plane->normal;

                                    elasticCollide_Plane(*entities[i],rec);

                                    entities[i]->position+=(-deepness)*plane->normal;
                                    obb->center=entities[i]->position;
                                    // auto& v=entities[i]->velocity;
                                    // auto v_perp=v.dot(plane->normal)*plane->normal;
                                    // auto v_para=v-v_perp;
                                    // v=-eta*(v_perp+v_para);
                                    // if(v_perp.norm2()<0.01f) v_perp=MyGeo::Vec3f{0,0,0};

                                    // dp=-(eta+1)*v_perp*entities[i]->mass;
                                    // MyGeo::Mat3f R0=entities[i]->rotation.mat3();
                                    
                                    // MyGeo::Vec3f contactPoint_local0=averageContactPoint-entities[i]->position;
                                    
                                    // entities[i]->L+=contactPoint_local0.cross(dp);
                                }
                                break;

                            }

                            case MyGeo::PrimType::OBBox:
                            {
                                auto obb1=std::static_pointer_cast<MyGeo::OBB>(e->collider);
                                
                                auto flag=obb->intersectOBB(*obb1,averageContactPoint);
                                if(flag)
                                {
                                    CollisionRecord rec;
                                    rec.point=averageContactPoint;
                                    rec.normal=(obb->center-obb1->center).normalVec();
                                    //dp=
                                    elasticCollide(*entities[i],*entities[k],rec);
                                    auto n0=(obb->center-obb1->center).normalVec();
                                    entities[i]->position+=0.03*n0;
                                    obb->center=entities[i]->position;
                                    obb1->center=entities[k]->position;
                                    
                                    // MyGeo::Mat3f R0=entities[i]->rotation.mat3();
                                    // MyGeo::Mat3f R1=entities[k]->rotation.mat3();
                                    // MyGeo::Vec3f contactPoint_local0=averageContactPoint-entities[i]->position;
                                    // MyGeo::Vec3f contactPoint_local1=averageContactPoint-entities[k]->position;

                                    // entities[i]->L+=R0*contactPoint_local0.cross(dp);
                                    // entities[k]->L+=R1*contactPoint_local1.cross(-dp);


                                }
                                break;
                            }

                            case MyGeo::PrimType::SPHERE:
                            {
                                auto sphere1=std::static_pointer_cast<MyGeo::Sphere>(e->collider);
                                float deepness;

                                auto flag=obb->intersectSphere(*sphere1,deepness);
                                if(flag)
                                {
                                    elasticCollide(*entities[i],*entities[k]);

                                    auto n0=(obb->center-sphere1->center).normalVec();
                                    entities[i]->position+=-deepness*n0;
                                    // auto& v0=entities[i]->velocity;
                                    // auto& v1=e->velocity;
                                    // auto v0_perp=v0.dot(n0)*n0*eta;
                                    // auto v0_para=v0-v0_perp;
                                    // v0=-v0_perp+v0_para;

                                    // auto v1_perp=v1.dot(-n0)*(-n0)*eta;
                                    // auto v1_para=v1-v1_perp;
                                    // v1=-v1_perp+v1_para;
                                    obb->center=entities[i]->position;
                                    sphere1->center=entities[k]->position;
                                    
                                }
                                break;
                            }
                            default:break;
                        }
                    }

                    break;
                }
                case MyGeo::PrimType::SPHERE:
                {
                    auto sphere=std::static_pointer_cast<MyGeo::Sphere>(entities[i]->collider);
                    sphere->center=entities[i]->position;

                    // for(const auto& e:entities)
                    for(int k=0;k!=entities.size();++k)
                    {
                        if(k==i) continue;
                        auto& e=entities[k];
                        switch(e->collider->type())
                        {
                            case MyGeo::PrimType::PLANE:
                            {
                                auto plane=std::static_pointer_cast<MyGeo::Plane>(e->collider);
                                float deepness;
                                auto flag=sphere->abovePlane(*plane,deepness,averageContactPoint);
                                if(!flag)
                                {
                                    CollisionRecord rec;
                                    rec.point=averageContactPoint;
                                    rec.normal=plane->normal;
                                    elasticCollide_Plane(*entities[i],rec);
                                    
                                    entities[i]->position+=(-deepness+epsilon)*plane->normal;
                                    sphere->center=entities[i]->position;
                                    // auto& v=entities[i]->velocity;
                                    // auto v_perp=v.dot(plane->normal)*plane->normal*eta;
                                    // auto v_para=v-v_perp;
                                    // v=-v_perp+v_para;
                                }
                                break;

                            }
                            case MyGeo::PrimType::SPHERE:
                            {
                                auto sphere1=std::static_pointer_cast<MyGeo::Sphere>(e->collider);
                                float deepness;

                                auto flag=sphere->intersectSphere(*sphere1,deepness);
                                if(flag)
                                {
                                    elasticCollide(*entities[i],*entities[k]);

                                    auto n0=(sphere->center-sphere1->center).normalVec();
                                    std::cout<<"deepness: "<<deepness<<std::endl;
                                    entities[i]->position+=(epsilon-deepness)*n0;
                                    // auto& v0=entities[i]->velocity;
                                    // auto& v1=e->velocity;
                                    // auto v0_perp=v0.dot(n0)*n0*eta;
                                    // auto v0_para=v0-v0_perp;
                                    // v0=-v0_perp+v0_para;

                                    // auto v1_perp=v1.dot(-n0)*(-n0)*eta;
                                    // auto v1_para=v1-v1_perp;
                                    // v1=-v1_perp+v1_para;
                                    sphere->center=entities[i]->position;
                                    sphere1->center=entities[k]->position;
                                }
                                break;
                            }
                            default:break;
                        }
                    }
                    break;
                }
                default:
                    break;
            } 
            entities[i]->tick(deltaT,force);
            
            // auto force=MyGeo::Vec3f{0,0,0};
            // entities[i]->tick(deltaT,force,averageContactPoint,dp);
        }
    }
};


