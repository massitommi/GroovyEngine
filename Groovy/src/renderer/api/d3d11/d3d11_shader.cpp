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

static EShaderVariableType GetVarType(D3D11_SHADER_VARIABLE_DESC varDesc, D3D11_SHADER_TYPE_DESC varTypeDesc)
{
	switch (varTypeDesc.Type)
	{
		case D3D_SVT_FLOAT: // float type
		{
			switch (varDesc.Size)
			{
				case 4 * 1:
					return SHADER_VARIABLE_TYPE_FLOAT1;
				case 4 * 2:
					return SHADER_VARIABLE_TYPE_FLOAT2;
				case 4 * 3:
					return SHADER_VARIABLE_TYPE_FLOAT3;
				case 4 * 4:
					return SHADER_VARIABLE_TYPE_FLOAT4;
				case 64:
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

D3D11Shader::D3D11Shader(ShaderSrc vertexSrc, ShaderSrc pixelSrc, const std::vector<ShaderVariable>& input)
{
	// compile and create the shaders
	ID3DBlob* vertexBytecode = d3dUtils::CompileShader(SHADER_TARGETS[SHADER_TARGET_VERTEX], vertexSrc.data, vertexSrc.length);
	ID3DBlob* pixelBytecode = d3dUtils::CompileShader(SHADER_TARGETS[SHADER_TARGET_PIXEL], pixelSrc.data, pixelSrc.length);

	mVertex = d3dUtils::CreateVertexShader(vertexBytecode);
	mPixel = d3dUtils::CreatePixelShader(pixelBytecode);

	// startup reflecion
	ID3D11ShaderReflection* reflector = nullptr;
	d3dcheckslow(D3DReflect(vertexBytecode->GetBufferPointer(), vertexBytecode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector));
	D3D11_SHADER_DESC shaderDesc;
	d3dcheckslow(reflector->GetDesc(&shaderDesc));

	// const buffers
	D3D11_SHADER_BUFFER_DESC bufferDesc;
	D3D11_SHADER_VARIABLE_DESC varDesc;
	D3D11_SHADER_TYPE_DESC varTypeDesc;

	mConstBuffersDesc.resize(shaderDesc.ConstantBuffers);
	mConstBuffers.resize(shaderDesc.ConstantBuffers);

	for (uint32 i = 0; i < shaderDesc.ConstantBuffers; i++)
	{
		auto bufferReflector = reflector->GetConstantBufferByIndex(i);
		bufferReflector->GetDesc(&bufferDesc);

		mConstBuffersDesc[i].name = bufferDesc.Name;
		mConstBuffersDesc[i].size = bufferDesc.Size;
		mConstBuffersDesc[i].variables.resize(bufferDesc.Variables);

		for (uint32 j = 0; j < bufferDesc.Variables; j++)
		{
			auto varReflector = bufferReflector->GetVariableByIndex(j);
			varReflector->GetDesc(&varDesc);
			auto varType = varReflector->GetType();
			varType->GetDesc(&varTypeDesc);

			mConstBuffersDesc[i].variables[j] =
			{
				varDesc.Name,
				varDesc.Size,
				varDesc.StartOffset,
				GetVarType(varDesc, varTypeDesc)
			};
		}

		mConstBuffers[i] = d3dUtils::CreateBuffer
		(
			D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, D3D11_USAGE_DYNAMIC,
			mConstBuffersDesc[i].size, nullptr
		);
	}

	// TODO Reflect samplers and textures!

	// input layout
	D3D11_SIGNATURE_PARAMETER_DESC inputDesc;
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;
	layout.resize(shaderDesc.InputParameters);

	for (uint32 i = 0; i < shaderDesc.InputParameters; i++)
	{
		d3dcheckslow(reflector->GetInputParameterDesc(i, &inputDesc));

		layout[i].SemanticName = inputDesc.SemanticName;
		layout[i].SemanticIndex = inputDesc.SemanticIndex;
		layout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layout[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		layout[i].Format = GetNativeVarType(inputDesc);
		layout[i].InputSlot = 0;
		layout[i].InstanceDataStepRate = 0;
	}

	mInputLayout = d3dUtils::CreateInputLayout(vertexBytecode, layout.data(), layout.size());

	reflector->Release();
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
			if (mConstBuffersDesc[i].bufferTarget & CONSTBUFFER_TARGET_VERTEX)
			{
				d3dUtils::gContext->VSSetConstantBuffers(i, 1, &mConstBuffers[i]);
			}
			if (mConstBuffersDesc[i].bufferTarget & CONSTBUFFER_TARGET_PIXEL)
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

const std::vector<ShaderConstBuffer>& D3D11Shader::GetSets() const
{
	return {};
}

void D3D11Shader::SetAttribute(uint32 setIndex, uint32 varIndex, void* data, size_t dataSize)
{
}

void D3D11Shader::SetAttribute(const char* varName, void* data, size_t dataSize)
{
}

#endif