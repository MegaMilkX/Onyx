#include "sound_emitter.h"
#include "transform.h"

#include "sound_root.h"

void SoundRoot::Update()
{
    if(!listener)
        return;
    Au::Math::Mat4f mat = listener->GetComponent<Transform>()->GetTransform();
    GameState::GetAudioMixer()->SetListenerTransform((float*)&mat);
}

void SoundRoot::OnCreate()
{
    
}

void SoundRoot::AddEmitter(SoundEmitter* emit)
{
    
}

void SoundRoot::SetListener(SoundListener* listener)
{
    this->listener = listener;
}