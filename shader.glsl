#version 450 core

layout(location=0) uniform vec4 uValue;
layout(location=1) uniform int sel;

layout(location=0) out vec4 outColor;

void main()
{
	double a = 0.5;
	double b = 1.5;
	double c = a*b;
	outColor = vec4(c);
}