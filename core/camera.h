#pragma once
#include<mygeo/vec.h>
#include"../geometry/ray.h"
#include"../geometry/transforms.h"
#include"../geometry/bbox.h"


struct Film
{
    MyGeo::Vec2i resolution;
};

struct Camera
{
    virtual Ray genRay(float x, float y) const =0;

};
struct CameraSample 
{
    MyGeo::Vec2f pFilm;
    MyGeo::Vec2f pLens;
    float time;
};

struct PerspectiveCamera : public Camera
{
    float fov;
    float aspectRatio;
    float lensRadius;
    float focalDistance;
    BB2f screen;
    Point position;
    Vect zaxis,xaxis,yaxis;

    MatrixTransform cam2world;
    MatrixTransform world2cam;
    MatrixTransform scr2cam;
    // MatrixTransform cam2scr;
    MatrixTransform scr2rst;
    MatrixTransform rst2scr;
    // MatrixTransform rst2cam;

    static BB2f computeScreen(float fov, float aspectRatio, float focalDistance)
    {
        float tan=std::tanf(toRad(fov)*0.5);
        float halfY=tan*focalDistance;
        float halfX=halfY*aspectRatio;
        std::cout<<"halfx "<<halfX<<" "<<halfY<<std::endl;
        return BB2f{{-halfX,-halfY},{halfX,halfY}};
    }

    PerspectiveCamera(
        Point position,
        Point lookat,
        Vect up,
        float fov,
        float aspectRatio,
        // const MatrixTransform& cam2world, 
        // const MatrixTransform& cam2scr,
        // const BB2f& screen,
        const Film& film,
        float lensRadius,
        float focalDistance
        ):
        position{position},
        fov{fov},
        aspectRatio{aspectRatio},
        lensRadius{lensRadius},
        focalDistance{focalDistance},
        screen{computeScreen(fov,aspectRatio,focalDistance)},
        zaxis{(position-lookat).normalize()},
        xaxis{up.cross(zaxis).normalize()},
        yaxis{zaxis.cross(xaxis)},
        cam2world{
            MatrixTransform{
                translateMat(position.v3)*
                MyGeo::Mat4f{
                    xaxis.v4,yaxis.v4,zaxis.v4,{0,0,0,1}
                }
            }
        },
        world2cam{MatrixTransform{
            MyGeo::Mat4f{
            MyGeo::Vec4f{xaxis.x,yaxis.x,zaxis.x,0},
            MyGeo::Vec4f{xaxis.y,yaxis.y,zaxis.y,0},
            MyGeo::Vec4f{xaxis.z,yaxis.z,zaxis.z,0},
            MyGeo::Vec4f{0,0,0,1}
            }//,BB2f{{-1.f,-1.f},{1.f,1.f}}
        }*translateMat(-position.v3)},
        //camera to screen(view frumtum to box)
        // cam2scr{getPerspectiveTransform(fov,-10.f,-10000.f)},
        scr2cam{translateMat(MyGeo::Vec3f{0.f,0.f,-focalDistance})},
        //physical screen coord to resolution coord, move upleft to source, and then scale
        scr2rst{
            scaleMat(MyGeo::Vec3f{(float)film.resolution.x,(float)film.resolution.y,1.f})
            *scaleMat(MyGeo::Vec3f{1.f/(screen.max.x-screen.min.x),1.f/(screen.min.y-screen.max.y),1.f})
            *translateMat(MyGeo::Vec3f{-screen.min.x,-screen.max.y,0.f})
            },
        //resolution coord to physical screen coord
        rst2scr{scr2rst.inverse()}
        //resolution to camera
        // rst2cam{cam2scr.inverse()*rst2scr}
        {
            // std::cout<<" cam2scr: "<<std::endl;
            // std::cout<<cam2scr.m<<std::endl;
            // std::cout<<" scr2rst "<<std::endl;
            // std::cout<<scr2rst.m<<std::endl;

            // std::cout<<" rst2scr "<<std::endl;
            // std::cout<<rst2scr.m<<std::endl;

            // std::cout<<" rst2cam "<<std::endl;
            // std::cout<<rst2cam.m<<std::endl;
        }

    Ray genRay(float x, float y) const override
    {
        // std::cout<<"asdf "<<cam2scr(Point{-1,1,-100}).v3<<std::endl;
        Point rstPoint{x+getRand(0.f,1.f),y+getRand(0.f,1.f),0.f};
        
        // std::cout<<"camPoint: "<<(scr2cam*rst2scr)(rstPoint)<<std::endl;
        return Ray{position,cam2world(Vect{(scr2cam*rst2scr)(rstPoint).v3}).normalize()} ;
        // return Ray{position,(cam2world*scr2cam*rst2scr)(rstPoint).v3.normalize()} ;
        // scr2camera
        // std::cout<<rst2cam.m*filmPoint.v4<<std::endl;
        // std::cout<<"scrpoint "<<rst2scr(filmPoint).v3<<std::endl;
        // std::cout<<"campoint "<<rst2cam(filmPoint).v3<<std::endl;
        // return cam2world(Ray{{0,0,0},Vect{rst2cam(filmPoint).v3}.normalize()});
    }     

    MatrixTransform getPerspectiveTransform(float fov, float n, float f) const 
    {
        MyGeo::Mat4f per{
            MyGeo::Vec4f{-1,0,0,0},
            {0,-1,0,0},
            {0,0,f/(f-n),1},
            {0,0,f*n/(n-f),0}
        };

        float reciTan=1.f/std::tan(toRad(fov)/2);
        float scaleFactor=reciTan*focalDistance;
        // std::cout<<(scaleMat({reciTan,reciTan,1.f})*MatrixTransform{per}).m;
        return scaleMat({scaleFactor,scaleFactor,1.f})*MatrixTransform{per};
    }

};
