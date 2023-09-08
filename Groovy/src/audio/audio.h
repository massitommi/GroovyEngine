#pragma once

#include "core/core.h"

typedef void* AudioClipHandle;

enum AudioClipFormat : byte
{
	AUDIO_CLIP_FORMAT_NONE,
	AUDIO_CLIP_FORMAT_PCM8,
	AUDIO_CLIP_FORMAT_PCM16,
	AUDIO_CLIP_FORMAT_PCM24,
	AUDIO_CLIP_FORMAT_PCM32,
	AUDIO_CLIP_FORMAT_PCMFLOAT,
	AUDIO_CLIP_FORMAT_BITSTREAM
};

struct AudioClipInfo
{
	uint32 durationMilliseconds;
	uint16 channels;
	AudioClipFormat format;
	byte bitsPerChannel;
};

class Audio
{
public:
	static void Init();
	static void Shutdown();
	static void Update();

	static AudioClipHandle CreateClip(BufferView& file);
	static AudioClipHandle CreateClipFromFile(const char* file);
	static AudioClipInfo GetClipInfo(AudioClipHandle clip);

	static void DestroyClip(AudioClipHandle clip);

	static void PlayClip(AudioClipHandle clip);

	static void StopEverything();

	static uint32 GetClipsCount();
};