#pragma once
#include"../geometry/space.h"
#include"camera.h"
#include<mygeo/vec.h>
#include<vector>
#include"rng.h"
// static constexpr float OneMinusEpsilon = 0x1.fffffep-1;

struct Sampler
{
    const int64_t samplesPerPixel;

    size_t array1DOffset, array2DOffset;
    MyGeo::Vec2i currentPixel;
    int64_t currentPixelSampleIndex;
    std::vector<int> samples1DArraySizes, samples2DArraySizes;
    std::vector<std::vector<float>> sampleArray1D;
    std::vector<std::vector<MyGeo::Vec2f>> sampleArray2D;

    Sampler::Sampler(int64_t samplesPerPixel)
        : samplesPerPixel(samplesPerPixel) { }

    virtual void startPixel(const MyGeo::Vec2i& p)
    {
        currentPixel=p;
        currentPixelSampleIndex=0;
        array1DOffset=array2DOffset=0;
    }

    //generate next 1 dimension
    virtual float get1D() = 0;
    //generate next 2 dimensions
    virtual MyGeo::Vec2f get2D() = 0;
    
    CameraSample getCameraSample(const MyGeo::Vec2i& pRaster)
    {
        CameraSample cs;
        cs.pFilm=MyGeo::Vec2f{(float)pRaster.x,(float)pRaster.y}+get2D();
        cs.time=get1D();
        cs.pLens=get2D();
        return cs;
    }

    virtual int roundCount(int n) const {return n;}
    
    virtual bool startNextSample()
    {
        array1DOffset=array2DOffset=0;
        return ++currentPixelSampleIndex < samplesPerPixel;
    }

    virtual bool setSampleNumber(int64_t sampleNum)
    {
        array1DOffset = array2DOffset = 0;
        currentPixelSampleIndex=sampleNum;
        return currentPixelSampleIndex < samplesPerPixel;
    }

    virtual std::unique_ptr<Sampler> clone(int seed) = 0;
    void request1DArray(int n) 
    {
        samples1DArraySizes.push_back(n);
        sampleArray1D.push_back(std::vector<float>(n * samplesPerPixel));
    }
    void request2DArray(int n)
    {
        samples2DArraySizes.push_back(n);
        sampleArray2D.push_back(std::vector<MyGeo::Vec2f>(n*samplesPerPixel));
    }

    const float* get1DArray(int n)
    {
        if(array1DOffset==sampleArray1D.size()) return nullptr;
        return &sampleArray1D[array1DOffset++][currentPixelSampleIndex*n];
    }
    const MyGeo::Vec2f* get2DArray(int n)
    {
        if(array2DOffset==sampleArray2D.size()) return nullptr;
        return &sampleArray2D[array2DOffset++][currentPixelSampleIndex*n];
    }
};

//The number of dimensions of the sample vectors that will be used by the rendering algorithm isn’t known ahead of time. (Indeed, it’s only determined implicitly by the number of Get1D() and Get2D() calls and the requested arrays.)

struct PixelSampler: public Sampler
{
    std::vector<std::vector<float>> samples1D;
    std::vector<std::vector<MyGeo::Vec2f>> samples2D;
    int current1DDimension = 0, current2DDimension = 0;
    RNG rng;
    
    PixelSampler(int64_t samplesPerPixel, int nSampledDimensions)
    : Sampler(samplesPerPixel) 
    {
        for (int i = 0; i < nSampledDimensions; ++i) 
        {
            samples1D.push_back(std::vector<float>(samplesPerPixel));
            samples2D.push_back(std::vector<MyGeo::Vec2f>(samplesPerPixel));
        }
    }

    bool startNextSample()
    {
        current1DDimension=current2DDimension=0;
        return Sampler::startNextSample();
    }
    bool setSampleNumber(int64_t sampleNum)
    {
        current1DDimension=current2DDimension=0;
        return Sampler::setSampleNumber(sampleNum);
    }
    float get1D()
    {
        if(current1DDimension < samples1D.size())
            return samples1D[current1DDimension++][currentPixelSampleIndex];
        else
            return rng.uniformFloat();
    }
    MyGeo::Vec2f get2D()
    {
        if(current2DDimension < samples2D.size())
            return samples2D[current2DDimension++][currentPixelSampleIndex];
        else 
            return MyGeo::Vec2f{rng.uniformFloat(),rng.uniformFloat()};
    }
};

struct GlobalSampler: public Sampler
{
    int dimension;
    int64_t intervalSampleIndex;
    static const int arrayStartDim=5;
    int arrayEndDim;

    bool startNextSample()
    {
        dimension=0;
        intervalSampleIndex=getIndexForSample(currentPixelSampleIndex+1);
        return Sampler::startNextSample();
    }
    void startPixel(const MyGeo::Vec2i& p) 
    {
        Sampler::startPixel(p);
        dimension=0;
        intervalSampleIndex=getIndexForSample(0);
        arrayEndDim=arrayStartDim+sampleArray1D.size()+2*sampleArray2D.size();

        for(size_t i=0;i<samples1DArraySizes.size();++i)
        {
            int nSamples=samples1DArraySizes[i]*samplesPerPixel;
            for(int j=0;j<nSamples;++j)
            {
                int64_t index=getIndexForSample(j);
                sampleArray1D[i][j]=sampleDimension(index,arrayStartDim+1);
            }
        }
       int dim = arrayStartDim + samples1DArraySizes.size();
       for (size_t i = 0; i < samples2DArraySizes.size(); ++i) 
       {
           int nSamples = samples2DArraySizes[i] * samplesPerPixel;
           for (int j = 0; j < nSamples; ++j) {
               int64_t idx = getIndexForSample(j);
               sampleArray2D[i][j].x = sampleDimension(idx, dim);
               sampleArray2D[i][j].y = sampleDimension(idx, dim+1);
           }
           dim += 2;
       }

    }
    bool setSampleNumber(int64_t sampleNum)
    {
        dimension=0;
        intervalSampleIndex=getIndexForSample(sampleNum);
        return Sampler::setSampleNumber(sampleNum);
    }
    float get1D()
    {
        if(dimension>=arrayStartDim && dimension<arrayEndDim) dimension=arrayEndDim;
        return sampleDimension(intervalSampleIndex,dimension++);
    }
    MyGeo::Vec2f get2D()
    {
        if (dimension + 1 >= arrayStartDim && dimension < arrayEndDim)
            dimension = arrayEndDim;
        MyGeo::Vec2f p{sampleDimension(intervalSampleIndex, dimension),
                sampleDimension(intervalSampleIndex, dimension + 1)};
        dimension += 2;
        return p;
    }
    GlobalSampler(int64_t samplesPerPixel): Sampler(samplesPerPixel){}

    virtual int64_t getIndexForSample(int64_t sampleNum) const =0;

    virtual float sampleDimension(int64_t index, int dimension) const =0;
};
