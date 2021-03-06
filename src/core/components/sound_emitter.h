#ifndef SOUND_EMITTER_H
#define SOUND_EMITTER_H

#include "../scene_object.h"
#include "sound_root.h"
#include <sound_clip.h>

#undef GetObject

class SoundEmitter : public SceneObject::Component
{
public:
    SoundEmitter()
    : clip(0)
    {}

    void SetClip(const std::string& name)
    {
        clip = asset<SoundClip>::get(name);
        if(!clip)
            return;
        emitter = GameState::GetAudioMixer()->CreateEmitter(clip->GetBuffer());
        emitter->Play(1);
    }

    void OnCreate()
    {
        soundRoot = GetObject()->Root()->GetComponent<SoundRoot>();
        soundRoot->AddEmitter(this);
    }
protected:
    SoundRoot* soundRoot;
    asset<SoundClip> clip;
    AudioEmitter* emitter;
};

class SoundListener : public SceneObject::Component
{
public:
    void OnCreate()
    {
        soundRoot = GetObject()->Root()->GetComponent<SoundRoot>();
        soundRoot->SetListener(this);
    }
protected:
    SoundRoot* soundRoot;
};

#endif
