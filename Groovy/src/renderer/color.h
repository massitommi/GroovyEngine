#pragma once

enum EColorFormat
{
	COLOR_FORMAT_R8G8B8A8_UNORM,		// RGBA
	COLOR_FORMAT_R8G8B8_UNORM,			// RGB

	COLOR_FORMAT_R32G32B32A32_FLOAT,	// vec4
	COLOR_FORMAT_R32G32B32_FLOAT,		// vec3
	COLOR_FORMAT_R32G32_FLOAT,			// vec2
	COLOR_FORMAT_R32_FLOAT,				// float
		
	COLOR_FORMAT_R32_UINT,				// uint32

	COLOR_FORMAT_D24S8_UNORM_UINT		// DepthBuffer (24 unorm bits for depth, 8 uint bits for stencil)
};