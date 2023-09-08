#include "audio_clip.h"
#include "assets/asset_loader.h"

AudioClip::AudioClip()
	: mHandle(nullptr), mUUID(0), mLoaded(false), mInfo({})
{
}

AudioClip::~AudioClip()
{
	if (mHandle)
		Audio::DestroyClip(mHandle);
}

void AudioClip::Load()
{
	AssetLoader::LoadGenericAsset(this);
	mLoaded = true;
}

void AudioClip::Save()
{
}

void AudioClip::Serialize(DynamicBuffer& fileData) const
{
}

void AudioClip::Deserialize(BufferView fileData)
{
	if (!fileData.empty())
	{
		mHandle = Audio::CreateClip(fileData);
		mInfo = Audio::GetClipInfo(mHandle);
	}
}

void AudioClip::Play()
{
	check(mHandle);
	Audio::PlayClip(mHandle);
}
