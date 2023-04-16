#pragma once

#include "core/core.h"
#include "math/vector.h"

enum EFrameBufferTextureFormat
{
	FRAME_BUFFER_TEXTURE_FORMAT_RGBA,
	FRAME_BUFFER_TEXTURE_FORMAT_REDINT
};

struct FrameBufferSpec
{
	uint32 width, height;
	std::vector<EFrameBufferTextureFormat> colorAttachments;
	bool hasDepthAttachment, swapchainTarget;
};

struct ClearColor
{
	float r, g, b, a;
};

class FrameBuffer
{
public:
	virtual ~FrameBuffer() = default;

	virtual const FrameBufferSpec& GetSpecs() const = 0;
	virtual void Bind() = 0;
	virtual void Resize(uint32 width, uint32 height) = 0;
	virtual void ClearColorAttachment(uint32 colorIndex, ClearColor clearColor) = 0;
	virtual void ClearDepthAttachment() = 0;
	virtual uint64 GetRendererID(uint32 colorIndex) const = 0;

	static FrameBuffer* Create(const FrameBufferSpec& specs);
};

