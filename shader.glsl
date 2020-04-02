#version 450 core

layout(location=0) uniform vec2 uValue;

layout(location=0) out vec4 outColor;

float sum(float a, float b)
{
	return a + b;
}

void main()
{
	float a = sum(uValue.x, uValue.y);
	outColor = vec4(a, sum(0.2, 0.2), 0.8, 1.0);
}