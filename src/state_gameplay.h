#ifndef STATE_GAMEPLAY_H
#define STATE_GAMEPLAY_H

#include "game_state.h"
#include "scene_object.h"

#include "state_test.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/mesh.h"
#include "components/light_omni.h"

#include "util.h"

class Gameplay : public GameState
{
public:
    Gameplay()
    : camMoveFlags(0) {}
    
    virtual void OnInit() 
    {
        //camera = scene.CreateObject()->GetComponent<Camera>();
        //Mesh* mesh = scene.CreateObject()->GetComponent<Mesh>();
        Transform* transform = scene.CreateSceneObject()->GetComponent<Transform>();
        gfxScene = scene.GetComponent<GFXScene>();
        GFXRenderUnit renderUnit;
        renderUnit.transform = transform;
        renderUnit.mesh = LoadMesh(GFXDevice(), "miku.fbx");
        renderUnit.renderState = CreateRenderState(GFXDevice());
        gfxScene->AddRenderUnit(renderUnit);
        
        camera = scene.CreateSceneObject()->GetComponent<Camera>();
        camera->Perspective(1.6f, 16.0f/9.0f, 0.01f, 100.0f);
        camera->GetParentObject()->GetComponent<Transform>()->Translate(0.0f, 1.5f, 7.0f);
        LightOmni* light = camera->GetParentObject()->GetComponent<LightOmni>();
        light->Color(0.2f, 0.8f, 0.6f);
        light->Intensity(1.0f);
        
        LightOmni* light2 = scene.CreateSceneObject()->GetComponent<LightOmni>();
        light2->Color(0.6f, 0.2f, 0.8f);
        light2->GetParentObject()->GetComponent<Transform>()->Position(-2.0f, 1.5f, 0.0f);
    }
    virtual void OnCleanup() {}
    virtual void OnUpdate() 
    {
        Transform* camTrans = camera->GetParentObject()->GetComponent<Transform>();
        if(camMoveFlags & 1)
            camTrans->Translate(-camTrans->Back() * 0.001f);
        if(camMoveFlags & 2)
            camTrans->Translate(-camTrans->Right() * 0.001f);
        if(camMoveFlags & 4)
            camTrans->Translate(camTrans->Back() * 0.001f);
        if(camMoveFlags & 8)
            camTrans->Translate(camTrans->Right() * 0.001f);
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
        Transform* camTrans = camera->GetParentObject()->GetComponent<Transform>();
        
        camTrans->Rotate(-x * 0.005f, Au::Math::Vec3f(0, 1, 0));
        camTrans->Rotate(-y * 0.005f, camTrans->GetTransform() * Au::Math::Vec3f(1, 0, 0));
    }
private:
    SceneObject scene;
    GFXScene* gfxScene;
    Camera* camera;
    
    float fov = 1.6f;
    float zfar = 100.0f;
    
    char camMoveFlags;
};

#endif
