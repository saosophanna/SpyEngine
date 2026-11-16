#version 450

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform SpritePushConstants
{
    mat4 projection;
} pushConstants;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

void main()
{
    gl_Position = pushConstants.projection * vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
