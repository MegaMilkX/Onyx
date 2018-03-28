#ifndef STATE_TEST_H
#define STATE_TEST_H

#include "game_state.h"
#include "scene_object.h"

#include <aurora/transform.h>
#include <iostream>
#include <fstream>

#include "util.h"

class Gameplay;

class StateTest : public GameState
{
public:
    virtual void OnInit()
    {
        mesh = LoadMesh(GFXDevice(), "data\\miku.fbx");
        renderState = CreateRenderState(GFXDevice());
        
        projection = Au::Math::Perspective(fov, 16.0f/9.0f, 0.1f, zfar);
        view.Translate(Au::Math::Vec3f(0.0f, 1.5f, 7.0f));
        
        uniModelMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixModel");
        uniViewMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixView");
        uniProjMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixProjection");
        
        uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos", 3);
        uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB", 3);
        uniLightOmniPos.Set(Au::Math::Vec3f(0.0f, 1.5f, 2.5f), 0);
        uniLightOmniRGB.Set(Au::Math::Vec3f(0.8f, 0.6f, 0.2f), 0);
        uniLightOmniPos.Set(Au::Math::Vec3f(4.0f, 0.0f, 0.0f), 1);
        uniLightOmniRGB.Set(Au::Math::Vec3f(0.6f, 0.8f, 0.2f), 1);
        uniLightOmniPos.Set(Au::Math::Vec3f(-4.0f, 0.0f, 0.0f), 2);
        uniLightOmniRGB.Set(Au::Math::Vec3f(0.8f, 0.6f, 0.8f), 2);
        
        lx = 0.0f; ly = 0.0f;
        
        SceneObject* object = scene.CreateObject();
    }
    virtual void OnSwitch()
    {
        
    }
    virtual void OnCleanup()
    {
        
    }
    virtual void OnUpdate()
    {
        
    }
    virtual void OnRender(Au::GFX::Device* device)
    {
        device->Bind(renderState);
        device->Set(uniModelMat4f, model.GetTransform());
        device->Set(uniViewMat4f, Au::Math::Inverse(view.GetTransform()));
        device->Set(uniProjMat4f, projection);
        device->Bind(mesh);
        device->Render();
    }
    
    virtual void MouseMove(int x, int y)
    {
        uniLightOmniPos.Set(Au::Math::Vec3f(lx += x * 0.01f, ly -= y * 0.01f, 2.5f), 0);
        model.Rotate(x * 0.01f, Au::Math::Vec3f(0, 1, 0));
        model.Rotate(y * 0.01f, model.GetTransform() * Au::Math::Vec3f(1, 0, 0));
    }
    
    virtual void KeyDown(Au::Input::KEYCODE key)
    {
        if(key == Au::Input::KEY_1)
        {
            GameState::Pop();
            GameState::Push<Gameplay>();
        }
    }
private:
    SceneObject scene;

    Au::GFX::RenderState* renderState;
    Au::GFX::Mesh* mesh;
    Au::Math::Transform model;
    Au::Math::Transform view;
    Au::Math::Mat4f projection;
    
    Au::GFX::Uniform uniModelMat4f;
    Au::GFX::Uniform uniViewMat4f;
    Au::GFX::Uniform uniProjMat4f;
    Au::GFX::Uniform uniLightOmniPos;
    Au::GFX::Uniform uniLightOmniRGB;
    
    float fov = 1.6f;
    float zfar = 100.0f;
    
    float lx, ly;
};

#endif
