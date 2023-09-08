#pragma once

#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(AudioComponent)
class AudioComponent : public ActorComponent
{
	GROOVY_CLASS_BODY(AudioComponent, ActorComponent)

public:
	AudioComponent();

	virtual void BeginPlay() override;

	void PlayClip();
	AudioClip* GetClip() const { return mAudioClip; }

private:
	AudioClip* mAudioClip;
	bool mAutoplay;
};