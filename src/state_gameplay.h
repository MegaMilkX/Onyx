#ifndef STATE_GAMEPLAY_H
#define STATE_GAMEPLAY_H

#include "game_state.h"
#include "scene_object.h"

#include "state_test.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/mesh.h"

#include "util.h"

class Gameplay : public GameState
{
public:
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
        
        projection = Au::Math::Perspective(fov, 16.0f/9.0f, 0.1f, zfar);
        view.Translate(Au::Math::Vec3f(0.0f, 1.5f, 7.0f));
    }
    virtual void OnCleanup() {}
    virtual void OnUpdate() {}
    virtual void OnRender(Au::GFX::Device* device)
    {
        //camera->Render(device);
        gfxScene->Render(
            device,
            projection,
            view.GetTransform()
        );
    }
    
    virtual void KeyDown(Au::Input::KEYCODE key)
    {
        if(key == Au::Input::KEY_2)
        {
            GameState::Pop();
            GameState::Push<StateTest>();
        }
    }
private:
    SceneObject scene;
    GFXScene* gfxScene;
    Camera* camera;
    
    Au::Math::Transform model;
    Au::Math::Transform view;
    Au::Math::Mat4f projection;
    
    float fov = 1.6f;
    float zfar = 100.0f;
};

#endif
