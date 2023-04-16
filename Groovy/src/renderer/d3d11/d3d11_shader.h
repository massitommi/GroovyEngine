#pragma once

#include "renderer/shader.h"

class D3D11Shader : public Shader
{
public:
	D3D11Shader(ShaderSrc vertexSrc, ShaderSrc pixelSrc, const std::vector<ShaderAttribute>& attribs, const std::vector<ShaderConstBuffer>& constBuffers);
	~D3D11Shader();

	virtual void Bind() override;
	virtual void SetConstBuffer(uint32 index, void* data, size_t size) override;

private:
	struct ID3D11VertexShader* mVertex;
	struct ID3D11PixelShader* mPixel;
	struct ID3D11InputLayout* mInputLayout;
	std::vector<struct ID3D11Buffer*> mConstBuffers;
	std::vector<ShaderConstBuffer> mConstBufferDescs;
};