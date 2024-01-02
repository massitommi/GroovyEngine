#include "audio.h"
#include "fmod/include/fmod.h"

static FMOD_SYSTEM* sFMODSystem = nullptr;
static constexpr uint32 FMOD_MAX_CHANNELS = 32;
static std::vector<AudioClipHandle> sClips;

#define FMOD_ASSERT(FMOD_CALL, Msg) CORE_ASSERT((FMOD_CALL) == FMOD_OK, Msg)

void Audio::Init()
{
	FMOD_ASSERT(FMOD_System_Create(&sFMODSystem, FMOD_VERSION), "Unable to create FMOD system");
	FMOD_ASSERT(FMOD_System_Init(sFMODSystem, FMOD_MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr), "Unable to init FMOD system");
}

void Audio::Shutdown()
{
	FMOD_System_Close(sFMODSystem);
}

void Audio::Update()
{
	FMOD_System_Update(sFMODSystem);
}

AudioClipHandle Audio::CreateClip(BufferView& file)
{
	FMOD_SOUND* res = nullptr;
	FMOD_CREATESOUNDEXINFO desc = {};
	desc.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	desc.length = (unsigned int)file.remaining();
	FMOD_ASSERT(FMOD_System_CreateSound
	(
		sFMODSystem, (const char*)file.read_to_end(),
		FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_LOOP_OFF | FMOD_2D | FMOD_3D_WORLDRELATIVE | FMOD_3D_INVERSEROLLOFF,
		&desc, &res
	), "Unable to create FMOD sound");

	sClips.push_back(res);
	return (AudioClipHandle)res;
}

AudioClipHandle Audio::CreateClipFromFile(const char* file)
{
	FMOD_SOUND* res = nullptr;
	FMOD_ASSERT(FMOD_System_CreateSound
	(
		sFMODSystem, file,
		FMOD_LOOP_OFF | FMOD_2D | FMOD_3D_WORLDRELATIVE | FMOD_3D_INVERSEROLLOFF,
		nullptr, &res
	), "Unable to create FMOD sound");

	sClips.push_back(res);
	return (AudioClipHandle)res;
}

AudioClipInfo Audio::GetClipInfo(AudioClipHandle clip)
{
	check(clip);

	AudioClipInfo info;
	int channels, bpp;
	FMOD_SOUND_FORMAT format;
	FMOD_Sound_GetLength((FMOD_SOUND*)clip, &info.durationMilliseconds, FMOD_TIMEUNIT_MS);
	FMOD_Sound_GetFormat((FMOD_SOUND*)clip, nullptr, &format, &channels, &bpp);
	info.channels = (uint16)channels;
	info.bitsPerChannel = (byte)bpp;

	switch (format)
	{
	case FMOD_SOUND_FORMAT_PCM8:
		info.format = AUDIO_CLIP_FORMAT_PCM8;
		break;
	case FMOD_SOUND_FORMAT_PCM16:
		info.format = AUDIO_CLIP_FORMAT_PCM16;
		break;
	case FMOD_SOUND_FORMAT_PCM24:
		info.format = AUDIO_CLIP_FORMAT_PCM24;
		break;
	case FMOD_SOUND_FORMAT_PCM32:
		info.format = AUDIO_CLIP_FORMAT_PCM32;
		break;
	case FMOD_SOUND_FORMAT_PCMFLOAT:
		info.format = AUDIO_CLIP_FORMAT_PCMFLOAT;
		break;
	case FMOD_SOUND_FORMAT_BITSTREAM:
		info.format = AUDIO_CLIP_FORMAT_BITSTREAM;
		break;
	}

	return info;
}

void Audio::DestroyClip(AudioClipHandle clip)
{
	FMOD_ASSERT(FMOD_Sound_Release((FMOD_SOUND*)clip), "Unable to release FMOD audio");

	sClips.erase(std::find(sClips.begin(), sClips.end(), clip));
}

void Audio::PlayClip(AudioClipHandle clip)
{
	check(clip);
	FMOD_CHANNEL* playChannel;
	FMOD_System_PlaySound(sFMODSystem, (FMOD_SOUND*)clip, nullptr, false, &playChannel);
}

void Audio::StopEverything()
{
	for (AudioClipHandle clip : sClips)
	{
		FMOD_SOUNDGROUP* soundGroup = nullptr;
		int numPlaying = 0;
		FMOD_Sound_GetSoundGroup((FMOD_SOUND*)clip, &soundGroup);
		FMOD_SoundGroup_GetNumPlaying(soundGroup, &numPlaying);
		if (numPlaying)
			FMOD_SoundGroup_Stop(soundGroup);
	}
}

uint32 Audio::GetClipsCount()
{
	return sClips.size();
}
