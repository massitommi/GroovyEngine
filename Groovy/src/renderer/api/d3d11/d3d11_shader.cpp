#if PLATFORM_WIN32

#include "d3d11_shader.h"
#include "d3d11_utils.h"

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

std::vector<ConstBufferDesc> GetBuffersDesc(ID3D11ShaderReflection* reflector, const D3D11_SHADER_DESC& desc)
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

std::vector<ShaderResTexture> GetTextures(ID3D11ShaderReflection* reflector, const D3D11_SHADER_DESC& desc)
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

D3D11Shader::D3D11Shader(const void* vertexSrc, size_t vertexSize, const void* pixelSrc, size_t pixelSize)
	: mUUID(0)
{
	// compile and create the shaders
	ID3DBlob* vertexBytecode = d3dUtils::CompileShader(VERTEX_SHADER_TARGET_VER, vertexSrc, vertexSize);
	ID3DBlob* pixelBytecode = d3dUtils::CompileShader(PIXEL_SHADER_TARGET_VER, pixelSrc, pixelSize);

	mVertex = d3dUtils::CreateVertexShader(vertexBytecode);
	mPixel = d3dUtils::CreatePixelShader(pixelBytecode);

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

	mVertexConstBuffers.resize(mVertexConstBuffersDesc.size());
	mPixelConstBuffers.resize(mPixelConstBuffersDesc.size());

	for (uint32 i = 0; i < mVertexConstBuffers.size(); i++)
	{
		mVertexConstBuffers[i] = d3dUtils::CreateBuffer
		(
			D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, D3D11_USAGE_DYNAMIC,
			mVertexConstBuffersDesc[i].size, nullptr
		);
	}

	for (uint32 i = 0; i < mPixelConstBuffers.size(); i++)
	{
		mPixelConstBuffers[i] = d3dUtils::CreateBuffer
		(
			D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, D3D11_USAGE_DYNAMIC,
			mPixelConstBuffersDesc[i].size, nullptr
		);
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

	mInputLayout = d3dUtils::CreateInputLayout(vertexBytecode, layout.data(), layout.size());


	// textures
	mResTextures = GetTextures(pixelReflector, pixelDesc);


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

	for (auto constBuffer : mVertexConstBuffers)
		constBuffer->Release();
	for (auto constBuffer : mPixelConstBuffers)
		constBuffer->Release();
}

void D3D11Shader::Bind()
{
	d3dUtils::gContext->VSSetShader(mVertex, nullptr, 0);
	d3dUtils::gContext->PSSetShader(mPixel, nullptr, 0);
	d3dUtils::gContext->IASetInputLayout(mInputLayout);

	d3dUtils::gContext->VSSetConstantBuffers(0, mVertexConstBuffers.size(), mVertexConstBuffers.data());
	d3dUtils::gContext->PSSetConstantBuffers(0, mPixelConstBuffers.size(), mPixelConstBuffers.data());
}

size_t D3D11Shader::GetVertexConstBufferIndex(const std::string& bufferName)
{
	for (uint32 i = 0; i < mVertexConstBuffersDesc.size(); i++)
	{
		if (mVertexConstBuffersDesc[i].name == bufferName)
		{
			return i;
		}
	}
	return ~((size_t)0);
}

size_t D3D11Shader::GetPixelConstBufferIndex(const std::string& bufferName)
{
	for (uint32 i = 0; i < mPixelConstBuffersDesc.size(); i++)
	{
		if (mPixelConstBuffersDesc[i].name == bufferName)
		{
			return i;
		}
	}
	return ~((size_t)0);
}

void D3D11Shader::OverwriteVertexConstBuffer(uint32 index, void* data)
{
	check(index < mVertexConstBuffers.size());
	
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	d3dverify(d3dUtils::gContext->Map(mVertexConstBuffers[index], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy(mappedRes.pData, data, mVertexConstBuffersDesc[index].size);
	d3dUtils::gContext->Unmap(mVertexConstBuffers[index], 0);
}

void D3D11Shader::OverwritePixelConstBuffer(uint32 index, void* data)
{
	check(index < mPixelConstBuffers.size());

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	d3dverify(d3dUtils::gContext->Map(mPixelConstBuffers[index], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy(mappedRes.pData, data, mPixelConstBuffersDesc[index].size);
	d3dUtils::gContext->Unmap(mPixelConstBuffers[index], 0);
}
#endif