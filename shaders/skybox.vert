#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 texCoord;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
    mat4 modelview_inv;
    float mipLevel;
} ubo_skybox;

void main()
{
    texCoord=normalize(inPosition);
    // texCoord.xy*=-1;
    gl_Position=ubo_skybox.proj*ubo_skybox.view*vec4(inPosition,1.0);
}