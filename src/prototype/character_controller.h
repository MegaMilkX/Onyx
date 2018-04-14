#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include "character.h"
#include <transform.h>
#include <camera.h>
#include <game_state.h>

class CharacterController : public SceneObject::Component
{
public:
    CharacterController()
    : chara(0), dirFlags(0) {}
    
    void SetTarget(Character* chara)
    {
        this->chara = chara;
    }

    void Update()
    {
        if(!chara)
            return;
        Transform* camTrans = renderer->CurrentCamera()->GetComponent<Transform>();
        gfxm::vec3 t(0.0f, 0.0f, 0.0f);
        if(dirFlags & 1)
            t += gfxm::normalize(gfxm::vec3(-camTrans->Back().x, 0.0f, -camTrans->Back().z));
        if(dirFlags & 2)
            t += gfxm::normalize(gfxm::vec3(-camTrans->Right().x, 0.0f, -camTrans->Right().z));
        if(dirFlags & 4)
            t += gfxm::normalize(gfxm::vec3(camTrans->Back().x, 0.0f, camTrans->Back().z));
        if(dirFlags & 8)
            t += gfxm::normalize(gfxm::vec3(camTrans->Right().x, 0.0f, camTrans->Right().z));
        
        t = gfxm::normalize(t);
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
    Character* chara;
    char dirFlags;
};

#endif
