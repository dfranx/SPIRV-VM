#version 450

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform TestBlock
{
    vec2 info;
    float time;
} uBlock;

void main() {
   outColor = vec4(uBlock.info, abs(sin(uBlock.time)), 1.0f);
}