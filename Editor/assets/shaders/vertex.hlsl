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

VertexOutput main(float4 position : POSITION, float4 color : COLOR, float2 textCoords : TEXTCOORDS)
{
	VertexOutput output;

	output.position = mul(position, mvp);
	output.color = color;
	output.textCoords = textCoords;

	return output;
}