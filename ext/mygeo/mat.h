#pragma once
#include"vec.h"
#include<concepts>

namespace MyGeo{
template<class T,int M, int N>
struct Mat;

using Mat3f=Mat<float,3,3>;
using Mat4f=Mat<float,4,4>;


static Mat4f invMat(const Mat4f& m);

template<class T,int M, int N>
std::ostream& operator<<(std::ostream& ostr,const Mat<T,M,N>& m)
{
    for(int i=0;i!=M;i++)
    {
        for(int j=0;j!=N;j++)
        {
            ostr<<m(i,j)<<' ';
        }
        std::cout<<std::endl;
    }

    // for(int i=0;i!=N;i++)
    // {
    //     ostr<<m.col[i]<<std::endl;
    // }
    return ostr;
}


// template<class T,int N>
template<class T, int N>
struct DetFunctor;


//determinant declaration
// template<class T,int N>
// T det(const Mat<T, N, N>& m);
// static constexpr float detFunc2(const Mat<float, 2, 2>& m);
// static constexpr float detFunc3(const Mat<float, 3, 3>& m);
// static constexpr float detFunc4(const Mat<float, 4, 4>& m);

template<class T,int N>
Mat<T,N,N> Eye()
{
    Mat<T,N,N> m;
    for(int i=0;i!=N;i++) 
    for(int j=0;j!=N;j++)
    {
        if(j==i) m.col[i][i]=1;
        else m.col[i][j]=0;
    }
    return m;
}

template<class T,int M, int N>
struct Mat
{
    std::array<Vec<T,M>,N> col;

    T determinant() const
    {
        static_assert(M==N);
        //make sure T is float
        static_assert(std::is_same<float, typename std::remove_cv<T>::type>::value);
        // return det<T,N>(*this);
        DetFunctor<T,N> det;
        return det(*this);
    }

    const T& operator()(int i,int j) const 
    {
        return col[j][i];
    }

    T& operator()(int i,int j)
    {
        return col[j][i];
    }

    Mat& operator+=(const Mat& m)
    {
        for(int i=0;i!=N;i++)
        {
            col[i]+=m.col[i];
        } 
        return *this;
    }
    Mat operator+(const Mat& m)
    {
        Mat mr=*this;
        mr+=m;
        return mr;
    }
    Mat operator-()
    {
        Mat mr;
        for(int i=0;i!=N;i++) mr.col[i]=-col[i];
        return mr;
    }
    Mat operator-(const Mat& m)
    {
        Mat mr=m;
        return *this+(-mr);
        
        // return -m;
    }

    Mat& operator-=(const Mat& m)
    {
        *this+=-*this;
        return *this;
    }
    Mat& operator*=(T k)
    {
        for(auto& vc:col) vc*=k;
        return *this;
    }
    Mat operator*(T k) const
    {
        Mat mr=*this;
        mr*=k;
        return mr;        
    }

    friend Mat operator*(T k, const Mat& m)
    {
        return m*k;
    }

    Mat<T,N,M> transpose() const 
    {
        Mat<T,N,M> m;
        for(int i=0;i!=M;i++)
        for(int j=0;j!=N;j++)
        {
            m.col[j][i]=col[i][j];
        }
        return m;
    }

    Mat<T,N,N> inverse() const 
    {
        return invMat(*this);
    }

    Vec<T,M> operator*(const Vec<T,N>& v) const
    {
        // std::cout<<"v: "<<v<<std::endl;
        Vec<T,M> vr;//{0,0,0};
        vr.data.fill(0);
        for(int i=0;i!=N;i++) vr+=col[i]*v[i];
        // std::cout<<&(vr.data[1])<<std::endl;
        return vr;
    }
    template<int L>
    Mat<T,M,L> operator*(const Mat<T,N,L>& m) const 
    {
        Mat<T,M,L> mr;
        for(int i=0;i!=L;i++) mr.col[i]=*this*m.col[i];
        // std::cout<<&(mr.col[1][1])<<std::endl;
        return mr;
    }

    friend std::ostream& operator<<<T,M,N>(std::ostream& ostr,const Mat&);

    Mat adjMat() const 
    {
        T det=determinant();
        assert(det!=0);
        Mat<T,N,N> am{};

    }

};



#define DET3(m,r1,r2,r3,c1,c2,c3) ( m(r1, c1) * (m(r2, c2) * m(r3, c3) - m(r2, c3) * m(r3, c2))\
        + m(r1, c2) * (m(r3, c1) * m(r2, c3) - m(r2, c1) * m(r3, c3)) \
        + m(r1, c3) * (m(r2, c1) * m(r3, c2) - m(r2, c2) * m(r3, c1)) )




template<class T>
struct DetFunctor<T,2>
{
    T operator()(const Mat<T,2,2>& m)
    {
        return m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1);
    }
};
template<class T>
struct DetFunctor<T,3>
{
    T operator()(const Mat<T,3,3>& m)
    {
        return DET3(m,0,1,2,0,1,2);
    }
};
template<class T>
struct DetFunctor<T,4>
{
    T operator()(const Mat<T,4,4>& m)
    {
        return m(0, 0) * DET3(m, 1, 2, 3, 1, 2, 3) \
            - m(0, 1) * DET3(m, 1, 2, 3, 0, 2, 3) \
            + m(0, 2) * DET3(m, 1, 2, 3, 0, 1, 3) \
            - m(0, 3) * DET3(m, 1, 2, 3, 0, 1, 2);
    }
};


// static constexpr float detFunc2(const Mat<float, 2, 2>& m)
// {
//     return m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1);
// }
// static constexpr float detFunc3(const Mat<float, 3, 3>& m)
// {
//     return DET3(m,0,1,2,0,1,2);
// }
// static constexpr float detFunc4(const Mat<float, 4, 4>& m)
// {
//     return m(0, 0) * DET3(m, 1, 2, 3, 1, 2, 3) \
//         - m(0, 1) * DET3(m, 1, 2, 3, 0, 2, 3) \
//         + m(0, 2) * DET3(m, 1, 2, 3, 0, 1, 3) \
//         - m(0, 3) * DET3(m, 1, 2, 3, 0, 1, 2);
// }


// template<>
// float det<float, 2>(const Mat<float, 2, 2>& m)
// {
//     return m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1);
// }
// template<>
// float det<float, 3>(const Mat<float, 3, 3>& m)
// {
//     return DET3(m,0,1,2,0,1,2);
// }

// template<>
// float det<float, 4>(const Mat<float, 4, 4>& m)
// {
//     return m(0, 0) * DET3(m, 1, 2, 3, 1, 2, 3) \
//         - m(0, 1) * DET3(m, 1, 2, 3, 0, 2, 3) \
//         + m(0, 2) * DET3(m, 1, 2, 3, 0, 1, 3) \
//         - m(0, 3) * DET3(m, 1, 2, 3, 0, 1, 2);
// }

//child 3 dimensional det of 4 dimensional det, with nominated rows and columns
template<class T, int r1,int r2,int r3,int c1,int c2,int c3>
T DET3_4(const Mat<T,4,4>& m)
{
    return m(r1, c1) * (m(r2, c2) * m(r3, c3) - m(r2, c3) * m(r3, c2))\
        + m(r1, c2) * (m(r3, c1) * m(r2, c3) - m(r2, c1) * m(r3, c3)) \
        + m(r1, c3) * (m(r2, c1) * m(r3, c2) - m(r2, c2) * m(r3, c1));
}

template<class T, int r1,int r2, int c1, int c2>
T DET2_3(const Mat<T,3,3>& m)
{
    return m(r1,c1)*m(r2,c2)-m(r1,c2)*m(r2,c1);
}

#define REM4_0 1,2,3
#define REM4_1 0,2,3
#define REM4_2 0,1,3
#define REM4_3 0,1,2
#define AR4(m,i,j) DET3_4<float,REM4_##i,REM4_##j>(m)

#define REM3_0 1,2
#define REM3_1 0,2
#define REM3_2 0,1
#define AR3(m,i,j) DET2_3<float,REM3_##i,REM3_##j>(m)


static Mat3f invMat(const Mat3f& m)
{
    auto recidet=1.f/m.determinant();
    Mat3f resM;
    resM(0,0)=AR3(m,0,0)*recidet;
    resM(0,1)=-AR3(m,1,0)*recidet;
    resM(0,2)=AR3(m,2,0)*recidet;
    resM(1,0)=-AR3(m,0,1)*recidet;
    resM(1,1)=AR3(m,1,1)*recidet;
    resM(1,2)=-AR3(m,2,1)*recidet;
    resM(2,0)=AR3(m,0,2)*recidet;
    resM(2,1)=-AR3(m,1,2)*recidet;
    resM(2,2)=AR3(m,2,2)*recidet;

    return resM;
}

//inverse matrix
static Mat4f invMat(const Mat4f& m)
{

    auto recidet=1.f/m.determinant();
    Mat4f resM;

    resM(0,0)= AR4(m,0,0)*recidet; 
    resM(0,1)= -AR4(m,1,0)*recidet; 
    resM(0,2)= AR4(m,2,0)*recidet; 
    resM(0,3)= -AR4(m,3,0)*recidet; 

    resM(1,0)= -AR4(m,0,1)*recidet; 
    resM(1,1)= AR4(m,1,1)*recidet; 
    resM(1,2)= -AR4(m,2,1)*recidet; 
    resM(1,3)= AR4(m,3,1)*recidet; 

    resM(2,0)= AR4(m,0,2)*recidet;
    resM(2,1)= -AR4(m,1,2)*recidet;
    resM(2,2)= AR4(m,2,2)*recidet;
    resM(2,3)= -AR4(m,3,2)*recidet;

    resM(3,0)= -AR4(m,0,3)*recidet;
    resM(3,1)= AR4(m,1,3)*recidet;
    resM(3,2)= -AR4(m,2,3)*recidet;
    resM(3,3)= AR4(m,3,3)*recidet;
    
    return resM;
}





}


