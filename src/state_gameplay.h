#ifndef STATE_GAMEPLAY_H
#define STATE_GAMEPLAY_H

#include "game_state.h"
#include "scene_object.h"

#include "state_test.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/mesh.h"
#include "components/material.h"
#include "components/light_omni.h"

#include "components/luascript.h"

#include "util.h"

class Gameplay : public GameState
{
public:
    Gameplay()
    : camMoveFlags(0) {}
    
    virtual void OnInit() 
    {
        Resource<MeshData>::AddSearchPath("data");
        Resource<MeshData>::AddReader<MeshReaderFBX>("fbx");
        
        Resource<ScriptData>::AddSearchPath("data");
        Resource<ScriptData>::AddReader<ScriptReaderLUA>("lua");
        
        gfxScene = scene.GetComponent<GFXScene>();
        gfxScene->Init(GFXDevice());
        
        //LuaScript* script = scene.GetComponent<LuaScript>();
        //script->Relay("Init");
        
        MeshData* meshData = Resource<MeshData>::Get("miku");
        Au::GFX::Mesh* gfxMesh = GFXDevice()->CreateMesh();
        gfxMesh->Format(
            Au::Position() << 
            Au::Normal() << 
            Au::ColorRGB() << 
            Au::BoneWeight4() << 
            Au::BoneIndex4()
        );
        meshData->FillMesh(gfxMesh);
        
        Au::GFX::RenderState* renderState = CreateRenderState(GFXDevice());
        
        Mesh* mesh = scene.CreateSceneObject()->GetComponent<Mesh>();
        mesh->SetMesh(gfxMesh);
        mesh->SetRenderState(renderState);
        Material* mat = mesh->GetObject()->GetComponent<Material>();
        
        mesh2 = scene.CreateSceneObject()->GetComponent<Mesh>();
        mesh2->SetMesh(gfxMesh);
        mesh2->SetRenderState(renderState);
        mesh2->GetObject()->GetComponent<Transform>()->Translate(-6.0f, 0.0f, 0.0f);
        mesh2->GetObject()->GetComponent<Material>();
        
        camera = scene.CreateSceneObject()->GetComponent<Camera>();
        camera->Perspective(1.6f, 16.0f/9.0f, 0.01f, 100.0f);
        camera->GetObject()->GetComponent<Transform>()->Translate(0.0f, 1.5f, 7.0f);
        LightOmni* light = camera->GetObject()->GetComponent<LightOmni>();
        light->Color(0.2f, 0.8f, 0.6f);
        light->Intensity(1.0f);
        
        LightOmni* light2 = scene.CreateSceneObject()->GetComponent<LightOmni>();
        light2->Color(0.8f, 0.4f, 1.0f);
        light2->GetObject()->GetComponent<Transform>()->Position(-0.5f, 1.7f, 0.5f);
    }
    virtual void OnCleanup() 
    {
        Resource<MeshData>::Free("miku");
    }
    virtual void OnUpdate() 
    {
        Transform* camTrans = camera->GetObject()->GetComponent<Transform>();
        if(camMoveFlags & 1)
            camTrans->Translate(-camTrans->Back() * 0.001f);
        if(camMoveFlags & 2)
            camTrans->Translate(-camTrans->Right() * 0.001f);
        if(camMoveFlags & 4)
            camTrans->Translate(camTrans->Back() * 0.001f);
        if(camMoveFlags & 8)
            camTrans->Translate(camTrans->Right() * 0.001f);
        
        mesh2->GetObject()->GetComponent<Transform>()->Rotate(0.001f, 0.0f, 1.0f, 0.0f);
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
    Mesh* mesh2;
    
    float fov = 1.6f;
    float zfar = 100.0f;
    
    char camMoveFlags;
};

#endif
