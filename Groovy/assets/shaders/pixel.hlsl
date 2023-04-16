struct VertexOutput
{
	float4 position : SV_POSITION;
	float4 color: COLOR;
	float2 textCoords : TEXTCOORDS;
};

Texture2D texture0;
SamplerState sampler0;

struct PixelOutput
{
	float4 color0 : SV_TARGET0;
};

PixelOutput main(VertexOutput vertexOutput)
{
	PixelOutput output;
	output.color0 = texture0.Sample(sampler0, vertexOutput.textCoords) * vertexOutput.color;
	return output;
}