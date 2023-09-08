#pragma once

#include "core/core.h"
#include "math/vector.h"
#include "renderer/color.h"

struct FrameBufferSpec
{
	uint32 width, height;
	std::vector<EColorFormat> colorAttachments;
	bool hasDepthAttachment, swapchainTarget;
};

typedef Vec4 ClearColor;

class CORE_API FrameBuffer
{
public:
	virtual ~FrameBuffer() = default;

	virtual const FrameBufferSpec& GetSpecs() const = 0;
	virtual void Bind() = 0;
	virtual void Resize(uint32 width, uint32 height) = 0;
	virtual void ClearColorAttachment(uint32 colorIndex, ClearColor clearColor) = 0;
	virtual void ClearColorAttachments(ClearColor clearColor) = 0;
	virtual void ClearDepthAttachment() = 0;
	virtual void* GetRendererID(uint32 colorIndex) const = 0;

	static FrameBuffer* Create(const FrameBufferSpec& specs);
};

