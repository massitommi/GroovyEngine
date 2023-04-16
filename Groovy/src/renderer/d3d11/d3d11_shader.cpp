#if PLATFORM_WIN32

#include "d3d11_shader.h"
#include "d3d11_utils.h"

const char* SHADER_TARGETS[] =
{
	"vs_5_0",
	"ps_5_0"
};

#define SHADER_TARGET_VERTEX 0
#define SHADER_TARGET_PIXEL 1

static DXGI_FORMAT GetNativeAttribFormat(EShaderAttributeType type)
{
	switch (type)
	{
		case SHADER_ATTRIBUTE_TYPE_FLOAT1:	return DXGI_FORMAT_R32_FLOAT;
		case SHADER_ATTRIBUTE_TYPE_FLOAT2:	return DXGI_FORMAT_R32G32_FLOAT;
		case SHADER_ATTRIBUTE_TYPE_FLOAT3:	return DXGI_FORMAT_R32G32B32_FLOAT;
		case SHADER_ATTRIBUTE_TYPE_FLOAT4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	check(0);
	return DXGI_FORMAT_R32_FLOAT;
}

D3D11Shader::D3D11Shader(ShaderSrc vertexSrc, ShaderSrc pixelSrc, const std::vector<ShaderAttribute>& attribs, const std::vector<ShaderConstBuffer>& constBuffers)
	: mConstBufferDescs(constBuffers)
{
	// compile and create the shaders
	ID3DBlob* vertexBytecode = d3dUtils::CompileShader(SHADER_TARGETS[SHADER_TARGET_VERTEX], vertexSrc.data, vertexSrc.length);
	ID3DBlob* pixelBytecode = d3dUtils::CompileShader(SHADER_TARGETS[SHADER_TARGET_PIXEL], pixelSrc.data, pixelSrc.length);

	mVertex = d3dUtils::CreateVertexShader(vertexBytecode);
	mPixel = d3dUtils::CreatePixelShader(pixelBytecode);

	// create the layout
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
	layout.resize(attribs.size());

	for (uint32 i = 0; i < attribs.size(); i++)
	{
		layout[i].SemanticIndex = 0;
		layout[i].SemanticName = *attribs[i].name;
		layout[i].AlignedByteOffset = attribs[i].alignedOffset;
		layout[i].Format = GetNativeAttribFormat(attribs[i].type);
		layout[i].InputSlot = 0;
		layout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layout[i].InstanceDataStepRate = 0;
	}

	mInputLayout = d3dUtils::CreateInputLayout(vertexBytecode, &layout[0], layout.size());

	// create const buffers
	mConstBuffers.resize(constBuffers.size());

	for (uint32 i = 0; i < constBuffers.size(); i++)
	{
		checkslowf(constBuffers[i].size % 16 == 0, "D3D11 const buffers must be 16 bytes aligned!");

		mConstBuffers[i] = d3dUtils::CreateBuffer
		(
			D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, D3D11_USAGE_DYNAMIC,
			constBuffers[i].size, nullptr
		);
	}

	vertexBytecode->Release();
	pixelBytecode->Release();
}

D3D11Shader::~D3D11Shader()
{
	mInputLayout->Release();
	mVertex->Release();
	mPixel->Release();

	for (auto constBuffer : mConstBuffers)
	{
		constBuffer->Release();
	}
;}

void D3D11Shader::Bind()
{
	d3dUtils::gContext->VSSetShader(mVertex, nullptr, 0);
	d3dUtils::gContext->IASetInputLayout(mInputLayout);
	d3dUtils::gContext->PSSetShader(mPixel, nullptr, 0);

	if (!mConstBuffers.size())
	{
		d3dUtils::gContext->VSGetConstantBuffers(0, 0, nullptr);
		d3dUtils::gContext->PSGetConstantBuffers(0, 0, nullptr);
	}
	else
	{
		for (uint32 i = 0; i < mConstBuffers.size(); i++)
		{
			if (mConstBufferDescs[i].bufferTarget & CONSTBUFFER_TARGET_VERTEX)
			{
				d3dUtils::gContext->VSSetConstantBuffers(i, 1, &mConstBuffers[i]);
			}
			if (mConstBufferDescs[i].bufferTarget & CONSTBUFFER_TARGET_PIXEL)
			{
				d3dUtils::gContext->PSGetConstantBuffers(i, 1, &mConstBuffers[i]);
			}
		}
	}
}

void D3D11Shader::SetConstBuffer(uint32 index, void* data, size_t size)
{
	check(index <= mConstBuffers.size());
	d3dUtils::OverwriteBuffer(mConstBuffers[index], data, size);
}

#endif