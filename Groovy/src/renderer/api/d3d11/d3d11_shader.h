#pragma once

#include "../shader.h"

class D3D11Shader : public Shader
{
public:
	D3D11Shader(ShaderSrc vertexSrc, ShaderSrc pixelSrc, const std::vector<ShaderVariable>& input);
	~D3D11Shader();

	virtual void Bind() override;
	virtual void SetConstBuffer(uint32 index, void* data, size_t size) override;
	virtual const std::vector<ShaderConstBuffer>& GetSets() const override;
	virtual void SetAttribute(uint32 setIndex, uint32 varIndex, void* data, size_t dataSize) override;
	virtual void SetAttribute(const char* varName, void* data, size_t dataSize) override;

private:
	struct ID3D11VertexShader* mVertex;
	struct ID3D11PixelShader* mPixel;
	struct ID3D11InputLayout* mInputLayout;
	std::vector<struct ID3D11Buffer*> mConstBuffers;
	std::vector<ShaderConstBuffer> mConstBuffersDesc;
};