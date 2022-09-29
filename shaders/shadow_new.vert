#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTexCoord;

layout (binding = 0) uniform UBO 
{
	mat4 depthMVP;
	// mat4 sceneModel;
} ubo;

layout(push_constant) uniform PushConsts {
    mat4 model;
} pushConsts;
// layout (bindi
// layout (binding = 1) uniform
// {
// 	mat4 mats[];
// } modelMats;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

 
void main()
{
	gl_Position =  ubo.depthMVP*pushConsts.model * vec4(inPosition, 1.0);
}