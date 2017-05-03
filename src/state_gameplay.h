#ifndef STATE_GAMEPLAY_H
#define STATE_GAMEPLAY_H

#include <stdlib.h>

#include "game_state.h"
#include "scene_object.h"

#include "state_test.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/mesh.h"
#include "components/material.h"
#include "components/light_omni.h"

#include "components/luascript.h"

#include "components/animation.h"
#include "components/skeleton.h"

#include "util.h"

class Gameplay : public GameState
{
public:
    Gameplay()
    : camMoveFlags(0) {}
    
    virtual void OnInit() 
    {        
        gfxScene = scene.GetComponent<GFXScene>();
        gfxScene->Init(GFXDevice());
        gfxScene->AmbientColor(0.1f, 0.1f, 0.1f);
        gfxScene->RimColor(0.4f, 0.4f, 0.8f);
        
        camera = scene.CreateObject()->GetComponent<Camera>();
        camera->Perspective(1.6f, 16.0f/9.0f, 0.01f, 1000.0f);
        camera->GetObject()->GetComponent<Transform>()->Translate(0.0f, 1.5f, 7.0f);
        LightOmni* light = camera->GetObject()->GetComponent<LightOmni>();
        light->Color(0.6f, 1.0f, 0.8f);
        light->Intensity(1.0f);
        
        script = scene.GetComponent<LuaScript>();
        script->SetScript("scene");
        
        
        SceneObject* animTest = scene.CreateObject();
        
        Mesh* m = animTest->GetComponent<Mesh>();
        m->SetMesh("teapot");
        m->SetMaterial("material");
        
        AnimData* animData = Resource<AnimData>::Get("frame15");
        Animation* anim = animTest->GetComponent<Animation>();
        anim->SetAnim("test", animData->GetChild("Sphere001").GetAnim("Take 001"));
        anim->FrameRate(animData->FrameRate());
        anim->Play("test");
        //anim->SetAnimData("skin");
    }
    virtual void OnCleanup() 
    {
    }

    virtual void OnUpdate() 
    {
        scene.GetComponent<Animation>()->Update(DeltaTime());
        
        script->Relay("Update");
        Transform* camTrans = camera->GetObject()->GetComponent<Transform>();
        Au::Math::Vec3f t(0.0f, 0.0f, 0.0f);
        if(camMoveFlags & 1)
            t += -camTrans->Back();
        if(camMoveFlags & 2)
            t += -camTrans->Right();
        if(camMoveFlags & 4)
            t += camTrans->Back();
        if(camMoveFlags & 8)
            t += camTrans->Right();
        
        t = Au::Math::Normalize(t);
        t = t * 7.1f * DeltaTime();
        camTrans->Translate(t);
    }
    virtual void OnRender(Au::GFX::Device* device)
    {
        camera->Render(device);
    }
    
    virtual void KeyDown(Au::Input::KEYCODE key)
    {        
        if(key == Au::Input::KEY_2)
        {
            GameState::Pop();
            GameState::Push<StateTest>();
        }
        else if(key == Au::Input::KEY_W) camMoveFlags |= 1;
        else if(key == Au::Input::KEY_A) camMoveFlags |= 2;
        else if(key == Au::Input::KEY_S) camMoveFlags |= 4;
        else if(key == Au::Input::KEY_D) camMoveFlags |= 8;
    }
    
    virtual void KeyUp(Au::Input::KEYCODE key)
    {
        if(key == Au::Input::KEY_W) camMoveFlags &= ~1;
        else if(key == Au::Input::KEY_A) camMoveFlags &= ~2;
        else if(key == Au::Input::KEY_S) camMoveFlags &= ~4;
        else if(key == Au::Input::KEY_D) camMoveFlags &= ~8;
    }
    
    virtual void MouseMove(int x, int y)
    {
        Transform* camTrans = camera->GetObject()->GetComponent<Transform>();
        
        camTrans->Rotate(-x * 0.005f, Au::Math::Vec3f(0, 1, 0));
        camTrans->Rotate(-y * 0.005f, camTrans->GetTransform() * Au::Math::Vec3f(1, 0, 0));
    }
private:
    SceneObject scene;
    GFXScene* gfxScene;
    Camera* camera;
    LuaScript* script;
    
    float fov = 1.6f;
    float zfar = 100.0f;
    
    char camMoveFlags;
};

#endif
