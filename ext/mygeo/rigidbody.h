#pragma once
#include"quaternion.h"
#include<vector>

namespace MyGeo{
/**
 * @brief 
 * @param velocity velocity
 * @param position barycenter
 * @param I_inv inverse matrix of inertial tensor
 * @param L angular momentum
 * @param q orientation quaternion
 * @param mass mass
 * @param positions positions of vertices
 * @param netforce netforce
 */
class RigidBody
{
public:
    MyGeo::Vec3f velocity{0,0,0};
    //barycenter
    MyGeo::Vec3f position{0,0,0};
    MyGeo::Mat3f I_inv_0;

    MyGeo::Vec3f L{0,0,0};
    Quaternion<float> q{0,0,0,0};
    std::vector<MyGeo::Vec3f> positions;
    float mass;
    MyGeo::Vec3f netforce{0,0,0};
    bool gravityFlag;
    bool dynamicFlag;
    
    RigidBody(bool _dynamicFlag=true, bool _gravityFlag=true):dynamicFlag{_dynamicFlag},gravityFlag{_gravityFlag},mass{std::numeric_limits<float>::max()}
    {}

    RigidBody(std::vector<MyGeo::Vec3f>& _positions, float _mass):mass{_mass}
    {
        std::swap(_positions,positions);
    }

    /**
     * @brief 
     * calculate baricenter, initial I, initial I_inv(which is the one used in simulation), initial L
     * @return 
     */
    void init()
    {
        positions.push_back({1,0,0});
        positions.push_back({0,1,0});
        positions.push_back({0,0,1});
        for(const auto& p:positions)
        {
            position+=p;
        }
        position/=positions.size();
        auto pointMass=mass/positions.size();

        std::cout<<"position: "<<position<<std::endl;
        //inertial 
        MyGeo::Mat3f I{
            MyGeo::Vec3f{0,0,0},
            {0,0,0},
            {0,0,0}
        };
        for(auto& p:positions)
        {
            auto r=p-position;
            p=r;
            I(0,0)+=r.y*r.y+r.z*r.z;
            I(0,1)-=r.x*r.y;
            I(0,2)-=r.x*r.z;
            I(1,0)-=r.y*r.x;
            I(1,1)+=r.x*r.x+r.z*r.z;
            I(1,2)-=r.y*r.z;
            I(2,0)-=r.z*r.x;
            I(2,1)-=r.z*r.y;
            I(2,2)+=r.x*r.x+r.y*r.y;
        }
        I*=pointMass;
        I_inv_0=I.inverse();
        MyGeo::Vec3f omega{0,0,0};
        L=I*omega;
    }

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
};





}