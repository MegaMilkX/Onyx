#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include "actor.h"
#include <transform.h>
#include <camera.h>
#include <game_state.h>

class CharacterController : public SceneObject::Component
{
public:
    CharacterController()
    : chara(0), dirFlags(0) {}
    
    void SetTarget(Actor* chara)
    {
        this->chara = chara;
    }

    void Update()
    {
        if(!chara)
            return;
        Transform* camTrans = renderer->CurrentCamera()->GetComponent<Transform>();
        Au::Math::Vec3f t(0.0f, 0.0f, 0.0f);
        if(dirFlags & 1)
            t += Au::Math::Normalize(Au::Math::Vec3f(-camTrans->Back().x, 0.0f, -camTrans->Back().z));
        if(dirFlags & 2)
            t += Au::Math::Normalize(Au::Math::Vec3f(-camTrans->Right().x, 0.0f, -camTrans->Right().z));
        if(dirFlags & 4)
            t += Au::Math::Normalize(Au::Math::Vec3f(camTrans->Back().x, 0.0f, camTrans->Back().z));
        if(dirFlags & 8)
            t += Au::Math::Normalize(Au::Math::Vec3f(camTrans->Right().x, 0.0f, camTrans->Right().z));
        
        t = Au::Math::Normalize(t);
        t = t * 3;
        
        chara->Velocity(t);

        while(eKeyDown* e = disp_onKeyDown.poll())    
            KeyDown(e->key);
        while(eKeyUp* e = disp_onKeyUp.poll())
            KeyUp(e->key);
    }
    event_dispatcher<eKeyDown> disp_onKeyDown;
    event_dispatcher<eKeyUp> disp_onKeyUp;

    void KeyDown(Au::Input::KEYCODE key)
    {
        if(key == Au::Input::KEY_W) dirFlags |= 1;
        else if(key == Au::Input::KEY_A) dirFlags |= 2;
        else if(key == Au::Input::KEY_S) dirFlags |= 4;
        else if(key == Au::Input::KEY_D) dirFlags |= 8;
    }
    
    void KeyUp(Au::Input::KEYCODE key)
    {
        if(key == Au::Input::KEY_W) dirFlags &= ~1;
        else if(key == Au::Input::KEY_A) dirFlags &= ~2;
        else if(key == Au::Input::KEY_S) dirFlags &= ~4;
        else if(key == Au::Input::KEY_D) dirFlags &= ~8;
    }

    void OnCreate()
    {
        renderer = GetObject()->Root()->GetComponent<Renderer>();
    }
private:
    Renderer* renderer;
    Actor* chara;
    char dirFlags;
};

#endif
