#version 450 core

layout(location=0) uniform vec4 uValue;
layout(location=1) uniform int sel;

layout(location=0) out vec4 outColor;

uniform sampler2D Noise2d;

void main()
{
	outColor = texture(Noise2d, vec2(0, round(uValue).x));
}