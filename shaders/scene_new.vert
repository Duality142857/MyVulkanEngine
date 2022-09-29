#version 460
// #extension GL_EXT_nonuniform_qualifier :enable 
// #extension SPV_EXT_descriptor_indexing :enable
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
} ubo_scene;

layout(push_constant) uniform PushConsts {
    mat4 model;
} pushConsts;
// layout (binding = 1) uniform modelMats
// {
// 	mat4 mats[];
// } ;

//transform from [-1,1] to [0,1] for uv mapping
const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

// const mat4 biasMat = mat4( 
// 1.0, 0.0, 0.0, 0.0,
// 0.0, 1.0, 0.0, 0.0,
// 0.0, 0.0, 0.5, 0.0,
// 0.0, 0.0, 0.5, 1.0 );


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTexCoord;
// layout(location =4 ) in int modelId;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3 ) out vec4 fragShadowCoord;
layout(location = 4) out vec3 fragViewVec_normalized;
layout(location = 5) out vec3 fragLightVec;
layout(location = 6) flat out int outTextureId;

mat4 translate(vec3 tv)
{
    return mat4(1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                tv.x,tv.y,tv.z,1);
}


void main()
{
    outTextureId=int(inTexCoord.z);
    mat4 transformMat=ubo_scene.proj*ubo_scene.view*pushConsts.model;
    gl_Position=transformMat*vec4(inPosition,1.f);

    vec4 worldPos=pushConsts.model*vec4(inPosition,1.0f);

    float w_reci=1.0/worldPos.w;
    fragPosition=worldPos.xyz*w_reci;

    fragNormal=(pushConsts.model*vec4(inNormal,0.f)).xyz;
    
    fragTexCoord=inTexCoord.xy;

    fragShadowCoord=biasMat*ubo_scene.lightMVP*pushConsts.model*vec4(inPosition,1.0);

    fragViewVec_normalized=normalize(ubo_scene.eyePos-worldPos.xyz);
    fragLightVec=ubo_scene.lightPos-worldPos.xyz;
}