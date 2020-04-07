struct PSInput
{
	float4 Color : COLOR;
};

cbuffer buf : register(b0)
{
	float iTime;
};

float4 main(PSInput pin) : SV_TARGET
{
	return pin.Color;
}