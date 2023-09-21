#include "audio_component.h"
#include "audio/audio_clip.h"

GROOVY_CLASS_IMPL(AudioComponent)
	GROOVY_REFLECT(mAudioClip)
	GROOVY_REFLECT(mAutoplay)
GROOVY_CLASS_END()

AudioComponent::AudioComponent()
	: mAudioClip(nullptr), mAutoplay(true)
{
}

void AudioComponent::BeginPlay()
{
	Super::BeginPlay();

	if (mAutoplay)
		PlayClip();
}

void AudioComponent::PlayClip()
{
	if (mAudioClip)
		mAudioClip->Play();
}