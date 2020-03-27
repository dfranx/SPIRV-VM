#version 450 core

layout(location=0) uniform vec2 uValue;

layout(location=0) out vec4 outColor;

void main()
{
	float a = uValue.x + uValue.y;
	outColor = vec4(a, 0.4, 0.8, 1.0);
}