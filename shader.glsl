#version 450 core

layout(location=0) uniform vec4 uValue;
layout(location=1) uniform int sel;

layout(location=0) out vec4 outColor;

void main()
{
	float arr[4];
	for (int i = 0; i < 4; i++)
		arr[i] = uValue[i];

	outColor = vec4(arr[0], arr[1], arr[2], arr[3]);
}