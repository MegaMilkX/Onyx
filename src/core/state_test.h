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
        
        projection = gfxm::perspective(fov, 16.0f/9.0f, 0.1f, zfar);
        view.translate(gfxm::vec3(0.0f, 1.5f, 7.0f));
        
        uniModelMat4f = Au::GFX::GetUniform<gfxm::mat4>("MatrixModel");
        uniViewMat4f = Au::GFX::GetUniform<gfxm::mat4>("MatrixView");
        uniProjMat4f = Au::GFX::GetUniform<gfxm::mat4>("MatrixProjection");
        
        uniLightOmniPos = Au::GFX::GetUniform<gfxm::vec3>("LightOmniPos", 3);
        uniLightOmniRGB = Au::GFX::GetUniform<gfxm::vec3>("LightOmniRGB", 3);
        uniLightOmniPos.Set(gfxm::vec3(0.0f, 1.5f, 2.5f), 0);
        uniLightOmniRGB.Set(gfxm::vec3(0.8f, 0.6f, 0.2f), 0);
        uniLightOmniPos.Set(gfxm::vec3(4.0f, 0.0f, 0.0f), 1);
        uniLightOmniRGB.Set(gfxm::vec3(0.6f, 0.8f, 0.2f), 1);
        uniLightOmniPos.Set(gfxm::vec3(-4.0f, 0.0f, 0.0f), 2);
        uniLightOmniRGB.Set(gfxm::vec3(0.8f, 0.6f, 0.8f), 2);
        
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
        device->Set(uniModelMat4f, model.matrix());
        device->Set(uniViewMat4f, gfxm::inverse(view.matrix()));
        device->Set(uniProjMat4f, projection);
        device->Bind(mesh);
        device->Render();
    }
    
    virtual void MouseMove(int x, int y)
    {
        uniLightOmniPos.Set(gfxm::vec3(lx += x * 0.01f, ly -= y * 0.01f, 2.5f), 0);
        model.rotate(x * 0.01f, gfxm::vec3(0, 1, 0));
        model.rotate(y * 0.01f, model.matrix() * gfxm::vec3(1, 0, 0));
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
    gfxm::transform model;
    gfxm::transform view;
    gfxm::mat4 projection;
    
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
