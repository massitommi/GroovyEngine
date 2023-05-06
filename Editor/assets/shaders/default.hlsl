GROOVY_SHADER_VERTEX

struct VertexOutput
{
	float4 position : SV_POSITION;
	float4 color: COLOR;
	float2 textCoords : TEXTCOORDS;
};

cbuffer mvpBuffer : register(b0)
{
	float4x4 mvp;
};

cbuffer anotherBuffer : register(b1)
{
	float4 tintColor;
	float3 LOD;
}

VertexOutput main(float4 position : POSITION, float4 color : COLOR, float2 textCoords : TEXTCOORDS)
{
	VertexOutput output;

	output.position = mul(position, mvp);
	output.color = color * tintColor * float4(LOD.x, LOD.y, LOD.z, 1.0f);
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

Texture2D tex1;
Texture2D texture0;

SamplerState sampler0;

Texture2D AmbientOcclusion;
Texture2D TextureACaso;

struct PixelOutput
{
	float4 color0 : SV_TARGET0;
};

PixelOutput main(VertexOutput vertexOutput)
{
	PixelOutput output;
	output.color0 = texture0.Sample(sampler0, vertexOutput.textCoords) * vertexOutput.color;
	output.color0 *= texture0.Sample(sampler0, vertexOutput.textCoords) * vertexOutput.color;
	output.color0 *= tex1.Sample(sampler0, vertexOutput.textCoords) * vertexOutput.color;
	output.color0 *= AmbientOcclusion.Sample(sampler0, vertexOutput.textCoords) * vertexOutput.color;
	output.color0 *= TextureACaso.Sample(sampler0, vertexOutput.textCoords) * vertexOutput.color;

	return output;
}