#version 450 core

layout(location=0) uniform vec4 uValue;
layout(location=1) uniform int sel;

layout(location=0) out vec4 outColor;

void incr(out float a)
{
	a = 2.0f;
}

float mtl(float a, float b)
{
	return a*b;
}

void main()
{
	float mul = 0.5f;
	incr(mul);

	vec4 data = vec4(1.0f, 2.0f, 3.0f, 4.0f);

	if (sel == 0)
		discard;
	else
		data.xy = vec2(0.5f, 0.75f);

	outColor = vec4(mtl(mul, data.x), data.xyz);
}