#version 460
layout(location = 0) in vec3 texCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
    mat4 modelview_inv;
    float mipLevel;
} ubo_skybox;
layout(binding = 4) uniform samplerCube cubeSampler;



void main()
{
    outColor=texture(cubeSampler, texCoord, ubo_skybox.mipLevel);
    // outColor=vec4(1,1,1,1);
}