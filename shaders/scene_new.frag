#version 460
#extension GL_EXT_nonuniform_qualifier :enable
layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 lightMVP;
    vec3 lightPos;
    vec3 lightColor;
    vec3 eyePos;
    vec3 ks;
    vec3 kd;
} ubo;

#define LAYERNUM 512
layout(binding = 3) buffer mystorageBuffer
{
    uint objectIDs[LAYERNUM];
};

layout(push_constant) uniform PushConsts {
    mat4 model;
    vec2 mousePos;
    uint objId;
    uint selectedId;
} pushConsts;


layout (constant_id = 0) const int textArraySize = 10;

layout(binding = 1) uniform sampler2D texSampler[textArraySize];
layout (binding = 2) uniform sampler2D shadowMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location=2) in vec2 fragTexCoord;
layout(location = 3) in vec4 inShadowCoord;
layout(location = 4) in vec3 fragViewVec_normalized;
layout(location = 5) in vec3 fragLightVec;
layout(location = 6) flat in int textureId;
layout(location =7 ) in vec3 inColor;

layout(location = 0) out vec4 outColor;



#define NUM_SAMPLES 20
#define NUM_RINGS 10

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586
vec2 disk[NUM_SAMPLES];

float rand(float x)
{
    return fract(sin(x)*10000.0);
}
float rand(vec2 uv)
{
    const float a = 12.9898, b = 78.233, c = 43758.5453;
    float dt=dot(uv.xy,vec2(a,b));
    float sn=mod(dt,PI);
    return fract(sin(sn)*c);
}

void poissonDiskSamples(const in vec2 randomSeed)
{
    float angleStep=PI2*float(NUM_RINGS)/float(NUM_SAMPLES);
    float numSamples_reci=1.0/float(NUM_SAMPLES);
    float angle=rand(randomSeed)*PI2;
    float radius=numSamples_reci;
    float radiusStep=radius;

    for(int i=0;i<NUM_SAMPLES;++i)
    {
        disk[i]=vec2(cos(angle),sin(angle))*pow(radius,0.75);
        radius+=radiusStep;
        angle+=angleStep;
    }
}

vec3 gethalfvec(vec3 a, vec3 b)
{
    return normalize(a+b);
}

vec3 getTexture(sampler2D sampl, vec2 tex)
{
    if(tex.x<-0.5) return ubo.kd;
    return texture(sampl,fragTexCoord).xyz;
}

const float lightSize=50.0;

float shadowFactor(vec4 shadowCoord, vec2 displacement)
{
    float shadow=1.0;
    if(shadowCoord.z>-1.0 && shadowCoord.z<1.0)
    {
        float blockerDist=texture(shadowMap, shadowCoord.st+displacement).r;
        if(shadowCoord.w>0.0 && blockerDist<shadowCoord.z)
        {
            shadow=0.1;
        }
    }
    return shadow;
}

//ndc深度值转换到相机(光源)空间z坐标
float getDepth(float d)
{
    // near*far/(far-d(far-near))
    return 5.0/(-50.0+d*49.9);
}


//计算半影尺寸
float getPenumbraSize(vec4 shadowCoord, const in float lightSize, const in float bDist)
{
    float receiverDist=-getDepth(shadowCoord.z);
    float blockerDist=-getDepth(bDist);
    return lightSize*(receiverDist-blockerDist)/blockerDist;
}

float pcf(vec4 shadowCoord, float penSize) {
    ivec2 texSize=textureSize(shadowMap,0);
    float shadow=1.0;
    for(int i=0;i<NUM_SAMPLES;++i)
    {
        shadow+=shadowFactor(shadowCoord,disk[i]*penSize*1.0/float(texSize.x));
    }
    return shadow/float(NUM_SAMPLES);
    
}

float pcss(vec4 shadowCoord)
{
    poissonDiskSamples(shadowCoord.xy);
    ivec2 texSize=textureSize(shadowMap,0);
    float blockerDist=0.0;
    for(int i=0;i<NUM_SAMPLES;++i)
    {
        blockerDist+=texture(shadowMap,shadowCoord.st+disk[i]*4.0/float(texSize.x)).r;
    }
    blockerDist/=NUM_SAMPLES;
    float penumbraSize=getPenumbraSize(shadowCoord,lightSize,blockerDist);
    return pcf(shadowCoord,penumbraSize);
}

// #define LAYERNUM 32
// layout(binding = 3) buffer mystorageBuffer
// {
//     uint objectIDs[LAYERNUM];
// };

// layout(push_constant) uniform PushConsts {
//     mat4 model;
//     vec2 mousePos;
//     uint objId;
// } pushConsts;
float getDarkness(vec2 uv, float k)
{
    if(uv.x<0||uv.y<0) return 1.0;
    int t=int(floor(uv.x*k))+int(floor(uv.y*k));
    if(t%2==0) return 0.0;
    return 1.0;
}

    

void main() 
{
    //mouse picking 
    uint layerInd=uint(gl_FragCoord.z*LAYERNUM);
    if(length(pushConsts.mousePos-gl_FragCoord.xy)<1)
    {
        objectIDs[layerInd]=pushConsts.objId;
    }
    vec4 shadowCoord=inShadowCoord/inShadowCoord.w;
    float visibility=pcss(shadowCoord);
    
    float d2reci=1.0/dot(fragLightVec,fragLightVec);

    vec3 lightVec_normalized=normalize(fragLightVec);

    vec3 halfVec=gethalfvec(lightVec_normalized,fragViewVec_normalized);
    
    
    float cos0=clamp(dot(halfVec,inNormal),0,1);
    float cos1=clamp(dot(inNormal,lightVec_normalized),0,1);

    vec3 texColor=vec3(1,1,1);
    if(textureId>=0) texColor=getTexture(texSampler[textureId],fragTexCoord);

    vec3 kd_texture=ubo.kd*texColor;

    vec3 diffuse=kd_texture*ubo.lightColor*cos1*d2reci;
    vec3 specular=ubo.ks*ubo.lightColor*pow(cos0,30)*d2reci;
    vec3 ambient=vec3(0.02,0.02,0.02)*kd_texture;
    
    vec3 color;

    if(inColor.x<0.5) color=vec3(0,0,0);
    else color=vec3(1,1,1);

    float darkness=getDarkness(fragTexCoord,2);
    

    // if(pushConsts.selectedId==pushConsts.objId)
    // {
    //     outColor=vec4(0,1,0,1);   
    // }
    // else
    // {
    //     outColor=vec4(vec3(darkness,darkness,darkness)*inColor*(specular+diffuse+ambient)*visibility,1.0);
    // }
    vec3 selectFilter=vec3(darkness,darkness,darkness);
    if(pushConsts.selectedId==pushConsts.objId)
    {
        selectFilter=vec3(0,5,0);
    }


    outColor=vec4(selectFilter*inColor*(specular+diffuse+ambient)*visibility,1.0);


}