#pragma once
#include"vec.h"
#include"geoConstants.h"
namespace MyGeo{

// static constexpr float epsilon=0.001f;
// static constexpr float squareEpsilon=epsilon*epsilon;

static inline int getMidInd(int i0,int i1, int N)
{
    if(i0<i1) return (i0+i1)/2;
    return (i0+i1+N)/2%N;
}
static inline int getExtremeInd(const Vec2f* v, const Vec2f& d, int N)
{
    int i0=0,i1=0;
    while(true)
    {
        int mid=getMidInd(i0,i1,N);
        int next=(mid+1)%N;
        Vec2f e= v[next]-v[mid];
        if(e.dot(d)>0)
        {
            if(mid!=i0) i0=mid;
            else return i1;
        }
        else 
        {
            int prev=(mid+N-1)%N;
            e=v[mid]-v[prev];
            if(e.dot(d)<0) i1=mid;
            else return mid;
        }
    }
}



struct Line2D
{
    Vec2f source;
    Vec2f direction;
    Line2D(const Vec2f& _source, const Vec2f& _dest):source{_source},direction{_dest-_source}
    {
        std::cout<<source<<"->"<<direction<<std::endl;
    }
    int intersect(const Line2D& l1, Vec2f& p) const 
    {
        auto delta=l1.source-source;
        auto kd0=delta.cross(direction);
        auto kd1=delta.cross(l1.direction);
        auto k=direction.cross(l1.direction);

        float squareL0=direction.norm2();
        float squareL1=l1.direction.norm2();
        float squareK=k*k;
        //向量1叉乘向量2
        if(squareK>epsilon_sqr*squareL0*squareL1)
        {
            float s=kd1/k;
            p=source+direction*s;
            return 1;//intersect
        }
        //起点距离平方
        float squareDelta=delta.norm2();
        k=delta.cross(direction);
        //起点差向量叉乘第一条直线的代表向量的平方
        squareK=k*k;
        //若趋近0则说明重合
        if(squareK>epsilon_sqr*squareK*squareDelta)
        {
            return 0;//different parallel lines
        }
        return 2;//same line
    }
};

struct LineSegment2D
{

    Vec2f source;
    Vec2f direction;
    LineSegment2D(const Vec2f& _source, const Vec2f& dest):source{_source},direction{dest-_source}{}

    /**
     * @brief intersection of 2 segments
     * 
     * @param l1 
     * @param ip 0, 1 intersection points or an interval represented by 2 end points
     * @return int 
     */
    int intersect(const LineSegment2D& l1, Vec2f ip[2]) const 
    {
        auto delta=l1.source-source;
        auto kd0=delta.cross(direction);
        auto kd1=delta.cross(l1.direction);
        auto k=direction.cross(l1.direction);

        float squareL0=direction.norm2();
        float squareL1=l1.direction.norm2();
        float squareK=k*k;
        //向量1叉乘向量2
        if(squareK>epsilon_sqr*squareL0*squareL1)
        {
            float s=kd1/k;
            if(s<0 || s>1) return 0;
            float t=kd0/k;
            if(t<0 || t>1) return 0;

            ip[0]=source+direction*s;
            return 1;//intersect
        }
        //起点距离平方
        float squareDelta=delta.norm2();
        k=delta.cross(direction);
        //起点差向量叉乘第一条直线的代表向量的平方
        squareK=k*k;
        //若趋近0则说明重合
        if(squareK>epsilon_sqr*squareK*squareDelta)
        {
            return 0;//different parallel lines
        }
        //所在直线重合，但也分情况
        float smin=direction.dot(delta)/squareL0;
        float smax=smin+direction.dot(l1.direction)/squareL0;
        float w[2];
        if(smin>smax) std::swap(smin,smax);
        
        int imax=intersectHelper(0,1,smin,smax,w);
        for(int i=0;i!=imax;++i) ip[i]=source+w[i]*direction;

        return imax;
    }

    static int intersectHelper(float u0, float u1, float v0, float v1, float w[2])
    {
        //
        if(v0>u1 || v1<u0) return 0;
        if(v0<u1)//
        {
            if(v1>u0)
            {
                if(v0>u0) w[0]=v0;
                else w[0]=u0;

                if(v1<u1) w[1]=v1;
                else w[1]=u1;
                
                return 2;
            }
            else //v1==u0
            {
                w[0]=u0;
                return 1;
            }
        }
        else //v0==u1
        {
            w[0]=u1;
            return 1;
        }
    }
};

struct Triangle2D
{
    Vec2f v[3];

    friend std::ostream& operator<<(std::ostream& ostr,const Triangle2D& t)
    {
        ostr<<t.v[0]<<' '<<t.v[1]<<' '<<t.v[2]<<std::endl;
        return ostr;
    }
    
    Triangle2D(){}
    Triangle2D(const Vec2f& v0,const Vec2f& v1,const Vec2f& v2):v{v0,v1,v2}
    {}
    //分离轴版本二
    bool intersectTriangle2D_P(const Triangle2D& t1) const 
    {
        const Triangle2D& t0=*this;
        for(int i=0;i!=3;i++)
        {
            //get the edge normal as a potential seperating axis
            int next=(i+1)%3;
            Vec2f e=t0.v[next]-t0.v[i];
            Vec2f d={e.y,-e.x};
            //get the projection interval of the other triangle on this axis, if all >0 then return false
            
            if(judgeSide(t0.v[i],d,t1))
            {
                return false;
            }
        }

        for(int i=0;i!=3;i++)
        {
            //get the edge normal as a potential seperating axis
            int next=(i+1)%3;
            Vec2f e=t1.v[next]-t1.v[i];
            Vec2f d={e.y,-e.x};
            //get the projection interval of the other triangle on this axis, if all >0 then return false
            if(judgeSide(t1.v[i],d,t0))
            {
                return false;
            }
        }
        return true;

    }

    //
    bool intersectTriangle2D(const Triangle2D& t1) const 
    {
        if(!intersectTriangle2D_P(t1)) return false;
        
        //compute intersection info
        

    }

    static bool judgeSide(const Vec2f& s, const Vec2f& d, const Triangle2D& t)
    {
        for(int i=0;i!=3;++i)
        {
            Vec2f w=t.v[i]-s;
            if(w.dot(d)<=epsilon)
            {
                return false;
            }
        }
        return true;

    }
};










}

