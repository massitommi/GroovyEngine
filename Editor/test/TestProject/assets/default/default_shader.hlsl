GROOVY_SHADER_VERTEX

struct VertexOutput
{
	float4 position : SV_POSITION;
	float4 color: COLOR;
	float2 textCoords : TEXTCOORDS;
};

cbuffer ViewProjBuffer : register(b0)
{
	float4x4 vp;
};

cbuffer ModelBuffer : register(b1)
{
	float4x4 m;
};

VertexOutput main(float4 position : POSITION, float4 color : COLOR, float2 textCoords : TEXTCOORDS)
{
	VertexOutput output;

	output.position = mul(position, mul(m, vp));
	output.color = color;
	output.textCoords = textCoords;

	return output;
}

GROOVY_SHADER_PIXEL

struct VertexOutput
{
	float4 position : SV_POSITION;
	float4 color: COLOR;
	float2 textCoords : TEXTCOORDS;
};

struct PixelOutput
{
	float4 color0 : SV_TARGET0;
};

Texture2D albedo;
SamplerState sampler0;

PixelOutput main(VertexOutput vertexOutput)
{
	PixelOutput output;
	output.color0 = albedo.Sample(sampler0, vertexOutput.textCoords) * vertexOutput.color;

	return output;
}