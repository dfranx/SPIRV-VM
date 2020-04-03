#version 450 core

layout(location=0) uniform vec2 uValue;
layout(location=1) uniform int sel;

layout(location=0) out vec4 outColor;

float sum(float x, float y)
{
	return x + y;
}

void main()
{
	float a = sum(uValue.x, uValue.y);
	int b = 3;
	int c = 3;
	switch (sel) {
		case 0:
			c = 4;
		break;
		case 1: 
			c = 5;
		break;
		case 2:
			discard;
			break;
		default:
			c = 2;
			break;
	}

	if (b*c <= 12)
		a *= 2.0f;

	outColor = vec4(a, sum(0.2, 0.2), 0.8, 1.0);
}