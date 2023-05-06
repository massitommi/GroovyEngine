#pragma once

#include "../shader.h"

class D3D11Shader : public Shader
{
public:
	D3D11Shader(const void* vertexSrc, size_t vertexSize, const void* pixelSrc, size_t pixelSize);
	~D3D11Shader();

	virtual void Bind() override;

	virtual const std::vector<ConstBufferDesc>& GetVertexConstBuffersDesc() const override { return mVertexConstBuffersDesc; }
	virtual const std::vector<ConstBufferDesc>& GetPixelConstBuffersDesc() const override { return mPixelConstBuffersDesc; }
	virtual const std::vector<ShaderResTexture>& GetPixelTexturesRes() const override { return mResTextures; }

	virtual size_t GetVertexConstBufferIndex(const std::string& bufferName) override;
	virtual size_t GetPixelConstBufferIndex(const std::string& bufferName) override;

	virtual void OverwriteVertexConstBuffer(uint32 index, void* data) override;
	virtual void OverwritePixelConstBuffer(uint32 index, void* data) override;

private:
	struct ID3D11VertexShader* mVertex;
	struct ID3D11PixelShader* mPixel;
	struct ID3D11InputLayout* mInputLayout;
	std::vector<struct ID3D11Buffer*> mVertexConstBuffers;
	std::vector<struct ID3D11Buffer*> mPixelConstBuffers;
	std::vector<ConstBufferDesc> mVertexConstBuffersDesc;
	std::vector<ConstBufferDesc> mPixelConstBuffersDesc;
	std::vector<ShaderResTexture> mResTextures;
};