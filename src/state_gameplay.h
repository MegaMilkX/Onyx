#ifndef STATE_GAMEPLAY_H
#define STATE_GAMEPLAY_H

#include "game_state.h"
#include "object.h"

#include "state_test.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/mesh.h"

class Gameplay : public GameState
{
public:
    virtual void OnInit() 
    {
        camera = scene.CreateObject()->GetComponent<Camera>();
        Mesh* mesh = scene.CreateObject()->GetComponent<Mesh>();
    }
    virtual void OnCleanup() {}
    virtual void OnUpdate() {}
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
    }
private:
    Object scene;
    Camera* camera;
};

#endif
