#pragma once
#include"vec.h"
#include<vector>
#include"mat.h"
// #include"intersection3d.h"
// #include"sphere.h"
#include"collision.h"
#include"primitive.h"
// #define NDEBUG

namespace MyGeo{

class OBB : public Primitive
{
public:
    Vec3f center;
    Vec3f axis[3];
    Vec3f extent;
    PrimType type() override
    {
        return PrimType::OBBox;
    }
    OBB()
    {

    }

    std::vector<Vec3f> getVertices() const
    {
        std::vector<Vec3f> vertices(8);
        vertices[0]=center+extent[0]*axis[0]+extent[1]*axis[1]+extent[2]*axis[2];
        vertices[1]=center+extent[0]*axis[0]+extent[1]*axis[1]-extent[2]*axis[2];
        vertices[2]=center+extent[0]*axis[0]-extent[1]*axis[1]+extent[2]*axis[2];
        vertices[3]=center-extent[0]*axis[0]+extent[1]*axis[1]+extent[2]*axis[2];
        vertices[4]=center-extent[0]*axis[0]-extent[1]*axis[1]+extent[2]*axis[2];
        vertices[5]=center-extent[0]*axis[0]+extent[1]*axis[1]-extent[2]*axis[2];
        vertices[6]=center+extent[0]*axis[0]-extent[1]*axis[1]-extent[2]*axis[2];
        vertices[7]=center-extent[0]*axis[0]-extent[1]*axis[1]-extent[2]*axis[2];

        return vertices;
    }
    
    void gen(const std::vector<Vec3f>& points)
    {
        if(points.size()<=0)
        {
            std::cout<<"empty set!"<<std::endl;
            return;
        }
        //100  010  001  111  11-1  1-11  -111
        
        std::vector<Vec3f> minPoints(7,{0,0,0});
        std::vector<Vec3f> maxPoints(7,{0,0,0});
        std::vector<float> minProj(7,std::numeric_limits<float>::max());
        std::vector<float> maxProj(7,std::numeric_limits<float>::lowest());

        std::vector<float> minIndices(7,-1);
        std::vector<float> maxIndices(7,-1);

        //find all pairs of extrem points along each chosen axis, recording the point indices
        for(int i=0;i!=points.size();++i)
        {
            //100
            float proj=points[i].x;
            if(proj<minProj[0])
            {
                minProj[0]=proj;
                minIndices[0]=i;
            }
            if(proj>maxProj[0])
            {
                maxProj[0]=proj;
                maxIndices[0]=i;
            }
            //010
            proj=points[i].x;
            if(proj<minProj[1])
            {
                minProj[1]=proj;
                minIndices[1]=i;
            }
            if(proj>maxProj[1])
            {
                maxProj[1]=proj;
                maxIndices[1]=i;
            }
            //001
            proj=points[i].x;
            if(proj<minProj[2])
            {
                minProj[2]=proj;
                minIndices[2]=i;
            }
            if(proj>maxProj[2])
            {
                maxProj[2]=proj;
                maxIndices[2]=i;
            }

            //111
            proj=points[i].x+points[i].y+points[i].z;
            if(proj<minProj[3])
            {
                minProj[3]=proj;
                minIndices[3]=i;
            }
            if(proj>maxProj[3])
            {
                maxProj[3]=proj;
                maxIndices[3]=i;
            }
            //11-1
            proj=points[i].x+points[i].y-points[i].z;
            if(proj<minProj[4])
            {
                minProj[4]=proj;
                minIndices[4]=i;
            }
            if(proj>maxProj[4])
            {
                maxProj[4]=proj;
                maxIndices[4]=i;
            }
            //1-11
            proj=points[i].x-points[i].y+points[i].z;
            if(proj<minProj[5])
            {
                minProj[5]=proj;
                minIndices[5]=i;
            }
            if(proj>maxProj[5])
            {
                maxProj[5]=proj;
                maxIndices[5]=i;
            }
            //-111
            proj=-points[i].x+points[i].y+points[i].z;
            if(proj<minProj[6])
            {
                minProj[6]=proj;
                minIndices[6]=i;
            }
            if(proj>maxProj[6])
            {
                maxProj[6]=proj;
                maxIndices[6]=i;
            }
        }

        //assign the extrem point pairs according to the obtained indices
        for(int i=0;i!=7;++i)
        {
            maxPoints[i]=points[maxIndices[i]];
            minPoints[i]=points[minIndices[i]];
#ifndef NDEBUG
            std::cout<<"max: "<<maxPoints[i]<<std::endl;  
            std::cout<<"min: "<<minPoints[i]<<std::endl<<std::endl;   
#endif
        }

        float maxd=0.f;
        int maxdInd;
        for(int i=0;i!=7;++i)
        {
            float d=(maxPoints[i]-minPoints[i]).norm2();
            if(d>maxd)
            {
                maxd=d;
                maxdInd=i;
            }
        }
#ifndef NDEBUG
        std::cout<<"maxd: "<<maxd<<std::endl;  
#endif

        //first edge of the main triangle
        Line3D l0{minPoints[maxdInd],maxPoints[maxdInd]-minPoints[maxdInd]};
        //finding the most distant point to l0
        int tind;
        maxd=0.f;
        for(int i=0;i!=points.size();++i)
        {
            float d=l0.distance2Point_sqr(points[i]);
            if(d>maxd)
            {
                maxd=d;
                tind=i;
            }
        }

        //construct main triangle
        Triangle3D mainTriangle{minPoints[maxdInd],maxPoints[maxdInd],points[tind]};

        float bestVal=std::numeric_limits<float>::max();
        
        Vec3i minInds;
        Vec3i maxInds;
        //construct 3 obbs and their surface area
        genBestObbFromTriangle(mainTriangle,points,bestVal,minInds,maxInds);

        //find most distant points above and below main triangle
        Vec3f mainNormal=mainTriangle.edge(0).cross(mainTriangle.edge(1));
        Plane mainPlane{mainNormal,mainTriangle.v[0]};
        float mind=std::numeric_limits<float>::max();
        maxd=std::numeric_limits<float>::lowest();
        int minInd,maxInd;
        for(int i=0;i!=points.size();++i)
        {
            float d=mainPlane.distance(points[i]);
            if(i==0)
            {
                mind=d;
                maxd=d;
                minInd=0;
                maxInd=0;
            }
            if(d<mind)
            {
                mind=d;
                minInd=i;
            }
            if(d>maxd)
            {
                maxd=d;
                maxInd=i;
            }
        }
        Vec3f vup=points[maxInd];
        Vec3f vdown=points[minInd];

        for(int i=0;i!=3;++i)
        {
            //!CCW 
            Triangle3D triup{mainTriangle.v[i],mainTriangle.v[(i+1)%3],vup};
            Triangle3D tridown{mainTriangle.v[i],vdown,mainTriangle.v[(i+1)%3]};
            genBestObbFromTriangle(triup,points,bestVal,minInds,maxInds);
            genBestObbFromTriangle(tridown,points,bestVal,minInds,maxInds);
        }
        std::cout<<"minInds,maxInds: "<<minInds<<' '<<maxInds<<std::endl;
        for(int i=0;i!=3;++i)
        {
            center[i]=0.5f*(points[minInds[i]]+points[maxInds[i]]).dot(axis[i]);
        }
        std::cout<<"center: "<<center<<std::endl;

#ifndef NDEBUG
        std::cout<<"bestVal: "<<bestVal<<std::endl;
        std::cout<<"extent: "<<extent<<std::endl;
        std::cout<<"axis: "<<axis[0]<<'-'<<axis[1]<<'-'<<axis[2]<<std::endl;
#endif

        //got extent, 
    }

    void genBestObbFromTriangle(const Triangle3D& tri, const std::vector<Vec3f>& points, float& bestVal, Vec3i& minInds, Vec3i& maxInds)
    {
        //temp extent
        Vec3f textent;
        //triangle edges
        Vec3f e[3]={tri.edge(0),tri.edge(1),tri.edge(2)};
        Vec3f n=e[0].cross(e[1]);
        Vec3f taxis[3];
        taxis[1]=n.normalVec();

        Vec3i tminInds;
        Vec3i tmaxInds;
        textent[1]=getExtremProj(taxis[1],points,tminInds[1],tmaxInds[1]);

        for(int i=0;i!=3;++i)
        {
            taxis[0]=e[i].normalVec();
            taxis[2]=e[i].cross(n).normalVec();
            textent[0]=getExtremProj(taxis[0],points,tminInds[0],tmaxInds[0]);
            textent[2]=getExtremProj(taxis[2],points,tminInds[2],tmaxInds[2]);
            float curVal=textent[0]*textent[1]+textent[0]*textent[2]+textent[1]*textent[2];
            if(curVal<bestVal)
            {
                bestVal=curVal;
#ifndef NDEBUG
                std::cout<<"extent "<<textent<<std::endl;
                std::cout<<"val "<<curVal<<std::endl;
#endif
                extent=textent;
                axis[0]=taxis[0];
                axis[1]=taxis[1];
                axis[2]=taxis[2];
                minInds=tminInds;
                maxInds=tmaxInds;
            }
        }
    }

    static float getExtremProj(const Vec3f& dir, const std::vector<Vec3f>& points, int& minInd, int& maxInd)
    {
        float minProj=std::numeric_limits<float>::max();
        float maxProj=std::numeric_limits<float>::lowest();

        for(int i=0;i!=points.size();++i)
        {
            float proj=points[i].dot(dir);
            if(proj<minProj)
            {
                minProj=proj;
                minInd=i;
            }
            if(proj>maxProj)
            {
                maxProj=proj;
                maxInd=i;
            }
        }
        return 0.5f*(maxProj-minProj);
    }

    //get min and max proj of points along dir
    static Vec2f getExtremProjPair(const Vec3f& dir, const std::vector<Vec3f>& points, int& minIndex)
    {
        float minProj=std::numeric_limits<float>::max();
        float maxProj=std::numeric_limits<float>::lowest();

        for(int i=0;i!=points.size();++i)
        {
            float proj=points[i].dot(dir);
            if(proj<minProj)
            {
                minProj=proj;
                minIndex=i;
            }
            if(proj>maxProj)
            {
                maxProj=proj;
            }
        }
        return {minProj,maxProj};
    }

    // //narrow phase
    // void intersectOBB_narrow(const OBB& b1) const 
    // {
    //     const OBB& b0=*this;
        
    // }
    //broad phase
    bool intersectOBB(const OBB& b1, Vec3f& averageContactPoint) const 
    {
        int minIndex;
        const OBB& b0=*this;
        auto verts0=b0.getVertices();
        auto verts1=b1.getVertices();
        //face normals
        for(int i=0;i!=3;++i)
        {
            Vec3f n0=b0.axis[i].cross(b0.axis[(i+1)%3]);


            Vec2f pair1=getExtremProjPair(n0,verts1,minIndex);
            Vec2f pair0=getExtremProjPair(n0,verts0,minIndex);
            if(pair0[0]>pair1[1] || pair0[1]<pair1[0]) return false;
            

            Vec3f n1=b1.axis[i].cross(b1.axis[(i+1)%3]);

            pair1=getExtremProjPair(n1,verts1,minIndex);
            pair0=getExtremProjPair(n1,verts0,minIndex);
            if(pair0[0]>pair1[1] || pair0[1]<pair1[0]) return false;
        }
        //cross products of edges
        for(int i=0;i!=3;++i)
        for(int j=0;j!=3;++j)
        {
            Vec3f a=b0.axis[i].cross(b1.axis[j]);
            Vec2f pair1=getExtremProjPair(a,verts1,minIndex);
            Vec2f pair0=getExtremProjPair(a,verts0,minIndex);
            if(pair0[0]>pair1[1] || pair0[1]<pair1[0]) return false;
        }

        bool contactArray[8]={1,1,1,1,1,1,1,1};

        for(int k=0;k!=8;++k)
        {
            for(int i=0;i!=3;++i)
            {
                auto proj=verts0[k].dot(b1.axis[i]);
                if(proj<(center[i]-extent[i]) && proj>(center[i]+extent[i]))
                {
                    contactArray[k]=0;
                    break;
                }
            }
        }

        int count=0;
        for(int i=0;i!=8;++i)
        {
            if(contactArray[i])
            {
                averageContactPoint+=verts0[i];
                count+=1;
            }
        }
        averageContactPoint/=(float)count;


        return true;
    }

    /**
     * @brief intersect with plane
     * 
     * @param p target plane
     * @return true 
     * @return false 
     */
    bool abovePlane(const Plane& p, float& deepness, MyGeo::Vec3f& contactPoint) const 
    {
        int minIndex;
        auto verts=getVertices();
        // Vec2f pair=getExtremProjPair(p.normal,verts,minIndex);
        float minProj=std::numeric_limits<float>::max();
        // float maxProj=std::numeric_limits<float>::lowest();
        int num=0;
        for(auto& v:verts)
        {
            float proj=v.dot(p.normal);
            //point below
            if(proj+p.d<0)
            {
                contactPoint+=v;
                num+=1;
                if(minProj>proj) minProj=proj;
            }
        }
        if(num==0) return true;
        deepness=minProj+p.d;
        contactPoint/=(float)num;

        return false;
    }


    bool intersectSphere(const Sphere& sph, float& deepness) const 
    {
        Mat3f invM{
            Vec3f{axis[0].x,axis[1].x,axis[2].x},
            Vec3f{axis[0].y,axis[1].y,axis[2].y},
            Vec3f{axis[0].z,axis[1].z,axis[2].z}
        };
        Vec3f localSphereCenter=invM*(sph.center-center);
        Vec3f closePoint;
        for(int i=0;i!=3;++i)
        {
            closePoint[i]=std::max(-extent[i],std::min(extent[i],localSphereCenter[i]));
        }

        if((closePoint-localSphereCenter).norm2()>sph.r*sph.r+epsilon)
        {   
            return false;
        }
        
        //sphere center out of the box
        if(closePoint!=localSphereCenter)
        {
            
            deepness=(localSphereCenter-closePoint).norm()-sph.r;
            // std::cout<<"deepness: "<<deepness<<std::endl;
            // std::cout<<"obb "<<center<<' '<<extent<<std::endl;
            // std::cout<<"closePoint "<<closePoint<<std::endl;
            // std::cout<<"sphere center: "<<sph.center<<std::endl;
        }
        else 
        {
            deepness=0.001f;
        }

        // bool inflag=true;
        // for(int i=0;i!=3;++i)
        // {
        //     if(localSphereCenter[i]<-extent[i] || localSphereCenter[i]>extent[i])
        //     {
        //         inflag=false;
        //         break;
        //     }
        // }
        return true;
    }

};

}

