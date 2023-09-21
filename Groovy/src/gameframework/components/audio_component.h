#pragma once

#include "gameframework/actor_component.h"

GROOVY_CLASS_DECL(AudioComponent)
class CORE_API AudioComponent : public ActorComponent
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