#if PLATFORM_WIN32

#include "d3d11_shader.h"
#include "d3d11_utils.h"

#define SHADER_ENTRYPOINT "main"
#define VERTEX_SHADER_TARGET_VER "vs_5_0"
#define PIXEL_SHADER_TARGET_VER "ps_5_0"

static EShaderVariableType GetVarType(D3D11_SHADER_VARIABLE_DESC varDesc, D3D11_SHADER_TYPE_DESC varTypeDesc)
{
	switch (varTypeDesc.Type)
	{
		case D3D_SVT_FLOAT: // float type
		{
			switch (varDesc.Size)
			{
				case 4 * 1: // float
					return SHADER_VARIABLE_TYPE_FLOAT1;
				case 4 * 2: // vec2
					return SHADER_VARIABLE_TYPE_FLOAT2;
				case 4 * 3: // vec3
					return SHADER_VARIABLE_TYPE_FLOAT3;
				case 4 * 4: // vec4
					return SHADER_VARIABLE_TYPE_FLOAT4;
				case 64: // 4x4 matrix
					return SHADER_VARIABLE_TYPE_FLOAT4X4;
			}
		}
		break;
	}
	checkslow(0);
	return SHADER_VARIABLE_TYPE_FLOAT1;
}

static DXGI_FORMAT GetNativeVarType(EShaderVariableType varType)
{
	switch (varType)
	{
		case SHADER_VARIABLE_TYPE_FLOAT1:	return DXGI_FORMAT_R32_FLOAT;
		case SHADER_VARIABLE_TYPE_FLOAT2:	return DXGI_FORMAT_R32G32_FLOAT;
		case SHADER_VARIABLE_TYPE_FLOAT3:	return DXGI_FORMAT_R32G32B32_FLOAT;
		case SHADER_VARIABLE_TYPE_FLOAT4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	checkslow(0);
	return DXGI_FORMAT_R32_FLOAT;
}

static DXGI_FORMAT GetNativeVarType(const D3D11_SIGNATURE_PARAMETER_DESC& inputDesc)
{
	switch (inputDesc.ComponentType)
	{
		case D3D_REGISTER_COMPONENT_FLOAT32:
		{
			switch (inputDesc.Mask)
			{
			case 0b1:
				return DXGI_FORMAT_R32_FLOAT;
			case 0b11:
				return DXGI_FORMAT_R32G32_FLOAT;
			case 0b111:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			case 0b1111:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
		}
		break;
	}
	checkslow(0);
	return DXGI_FORMAT_R32_FLOAT;
}

static std::vector<ConstBufferDesc> GetBuffersDesc(ID3D11ShaderReflection* reflector, const D3D11_SHADER_DESC& desc)
{
	std::vector<ConstBufferDesc> res;
	res.resize(desc.ConstantBuffers);

	D3D11_SHADER_BUFFER_DESC bufferDesc;
	D3D11_SHADER_VARIABLE_DESC varDesc;
	D3D11_SHADER_TYPE_DESC varTypeDesc;

	for (uint32 i = 0; i < desc.ConstantBuffers; i++)
	{
		auto bufferReflector = reflector->GetConstantBufferByIndex(i);
		d3dcheckslow(bufferReflector->GetDesc(&bufferDesc));

		res[i].name = bufferDesc.Name;
		res[i].size = bufferDesc.Size;
		res[i].variables.resize(bufferDesc.Variables);

		for (uint32 j = 0; j < bufferDesc.Variables; j++)
		{
			auto varReflector = bufferReflector->GetVariableByIndex(j);
			d3dcheckslow(varReflector->GetDesc(&varDesc));
			auto varType = varReflector->GetType();
			d3dcheckslow(varType->GetDesc(&varTypeDesc));

			res[i].variables[j] =
			{
				varDesc.Name,
				varDesc.Size,
				varDesc.StartOffset,
				GetVarType(varDesc, varTypeDesc)
			};
		}
	}

	return res;
}

static std::vector<ShaderResTexture> GetTextures(ID3D11ShaderReflection* reflector, const D3D11_SHADER_DESC& desc)
{
	std::vector<ShaderResTexture> res;
	D3D11_SHADER_INPUT_BIND_DESC resDesc;
	for (uint32 i = 0; i < desc.BoundResources; i++)
	{
		reflector->GetResourceBindingDesc(i, &resDesc);
		if (resDesc.Type != D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE)
			continue;
		ShaderResTexture resText;
		resText.name = resDesc.Name;
		resText.bindSlot = resDesc.BindPoint;
		res.push_back(resText);
	}
	return res;
}

static ID3DBlob* CompileShader(const char* target, const void* src, size_t srcSize)
{
	ID3DBlob* bytecode;
	ID3DBlob* error;
	d3dcheckslowf(D3DCompile(src, srcSize, nullptr, nullptr, nullptr, SHADER_ENTRYPOINT, target, 0, 0, &bytecode, &error), (const char*)error);

	if (error)
		error->Release();

	return bytecode;
}

D3D11Shader::D3D11Shader(const void* vertexSrc, size_t vertexSize, const void* pixelSrc, size_t pixelSize)
	: mUUID(0)
{
	// compile and create the shaders
	ID3DBlob* vertexBytecode = CompileShader(VERTEX_SHADER_TARGET_VER, vertexSrc, vertexSize);
	ID3DBlob* pixelBytecode = CompileShader(PIXEL_SHADER_TARGET_VER, pixelSrc, pixelSize);

	d3dcheckslow(d3d11Utils::gDevice->CreateVertexShader(vertexBytecode->GetBufferPointer(), vertexBytecode->GetBufferSize(), nullptr, &mVertex));
	d3dcheckslow(d3d11Utils::gDevice->CreatePixelShader(pixelBytecode->GetBufferPointer(), pixelBytecode->GetBufferSize(), nullptr, &mPixel));

	// startup reflecion
	ID3D11ShaderReflection* vertexReflector = nullptr;
	d3dcheckslow(D3DReflect(vertexBytecode->GetBufferPointer(), vertexBytecode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&vertexReflector));
	D3D11_SHADER_DESC vertexDesc;
	d3dcheckslow(vertexReflector->GetDesc(&vertexDesc));
	
	ID3D11ShaderReflection* pixelReflector = nullptr;
	d3dcheckslow(D3DReflect(pixelBytecode->GetBufferPointer(), pixelBytecode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pixelReflector));
	D3D11_SHADER_DESC pixelDesc;
	d3dcheckslow(pixelReflector->GetDesc(&pixelDesc));

	// const buffers
	mVertexConstBuffersDesc = GetBuffersDesc(vertexReflector, vertexDesc);
	mPixelConstBuffersDesc = GetBuffersDesc(pixelReflector, pixelDesc);

	// pixel const buffers data
	mPixelConstBuffers.resize(mPixelConstBuffersDesc.size());

	D3D11_BUFFER_DESC constBufferDesc = {};
	constBufferDesc.ByteWidth = 0;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	for (uint32 i = 0; i < mPixelConstBuffers.size(); i++)
	{
		constBufferDesc.ByteWidth = mPixelConstBuffersDesc[i].size;
		d3dcheckslow(d3d11Utils::gDevice->CreateBuffer(&constBufferDesc, nullptr, &mPixelConstBuffers[i]));
	}

	// vertex input layout
	D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
	layout.resize(vertexDesc.InputParameters);

	for (uint32 i = 0; i < vertexDesc.InputParameters; i++)
	{
		d3dcheckslow(vertexReflector->GetInputParameterDesc(i, &inputDesc));

		layout[i].SemanticName = inputDesc.SemanticName;
		layout[i].SemanticIndex = inputDesc.SemanticIndex;
		layout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layout[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		layout[i].Format = GetNativeVarType(inputDesc);
		layout[i].InputSlot = 0;
		layout[i].InstanceDataStepRate = 0;
	}

	d3dcheckslow(d3d11Utils::gDevice->CreateInputLayout(layout.data(), layout.size(), vertexBytecode->GetBufferPointer(), vertexBytecode->GetBufferSize(), &mInputLayout));

	// textures
	mResTextures = GetTextures(pixelReflector, pixelDesc);

	// cleanup
	vertexReflector->Release();
	pixelReflector->Release();
	vertexBytecode->Release();
	pixelBytecode->Release();
}

D3D11Shader::~D3D11Shader()
{
	mVertex->Release();
	mPixel->Release();
	mInputLayout->Release();

	for (auto constBuffer : mPixelConstBuffers)
		constBuffer->Release();
}

void D3D11Shader::Bind()
{
	d3d11Utils::gContext->VSSetShader(mVertex, nullptr, 0);
	d3d11Utils::gContext->PSSetShader(mPixel, nullptr, 0);
	d3d11Utils::gContext->IASetInputLayout(mInputLayout);
	d3d11Utils::gContext->PSSetConstantBuffers(0, mPixelConstBuffers.size(), mPixelConstBuffers.data());
}

uint32 D3D11Shader::GetVertexConstBufferIndex(const std::string& bufferName)
{
	for (uint32 i = 0; i < mVertexConstBuffersDesc.size(); i++)
	{
		if (mVertexConstBuffersDesc[i].name == bufferName)
		{
			return i;
		}
	}
	return ~((uint32)0);
}

uint32 D3D11Shader::GetPixelConstBufferIndex(const std::string& bufferName)
{
	for (uint32 i = 0; i < mPixelConstBuffersDesc.size(); i++)
	{
		if (mPixelConstBuffersDesc[i].name == bufferName)
		{
			return i;
		}
	}
	return ~((uint32)0);
}

void D3D11Shader::OverwritePixelConstBuffer(uint32 index, void* data)
{
	check(index < mPixelConstBuffers.size());

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	d3dverify(d3d11Utils::gContext->Map(mPixelConstBuffers[index], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy(mappedRes.pData, data, mPixelConstBuffersDesc[index].size);
	d3d11Utils::gContext->Unmap(mPixelConstBuffers[index], 0);
}
#endif