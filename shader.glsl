#version 450 core

layout(location=0) uniform vec2 uValue;
layout(location=1) uniform int sel;

layout(location=0) out vec4 outColor;

struct InfoObject
{
	float Multiplier;
};

struct Object
{
	float Factor;
	float Delta;
	InfoObject Info;
};

float process(Object obj, InfoObject info)
{
	return obj.Delta * info.Multiplier;
}

void main()
{
	Object obj;
	obj.Delta= 0.2f;
	obj.Factor = 0.4f;
	obj.Info.Multiplier = 0.5f;
	outColor = vec4(1.0f, obj.Factor, process(obj, obj.Info), 2.0f);
}