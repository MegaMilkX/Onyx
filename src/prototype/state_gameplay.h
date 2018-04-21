#ifndef STATE_GAMEPLAY_H
#define STATE_GAMEPLAY_H

#include <stdlib.h>

#include <game_state.h>
#include <scene_object.h>

#include <transform.h>
#include <camera.h>
#include <model.h>
#include <light_omni.h>
#include <luascript.h>
#include <animator.h>
#include <skeleton.h>
#include <dynamics/rigid_body.h>
#include <collision/collider.h>
#include <sound_emitter.h>
#include <text_mesh.h>
#include <overlay/overlay_root.h>
#include <gui/gui_root.h>
#include <gui/gui_box.h>
#include <gui/gui_window.h>
#include <gui/gui_layout.h>

#include <motion_script.h>

#include <util/fbx_scene.h>

#include <external/imgui/imgui.h>

#include "fps_display.h"
#include "test_cube.h"
#include "character.h"
#include "character_controller.h"
#include "character_camera.h"

class Gameplay : public GameState
{
public:
    Gameplay()
    {}
    
    virtual void OnInit() 
    {
        ScopedTimer timer(__FUNCTION__);
        
        GameState::GetMouseHandler()->Locked(false);
        GameState::GetMouseHandler()->Visible(true);

        renderer = scene.GetComponent<Renderer>();
        renderer->AmbientColor(0.1f, 0.1f, 0.1f);
        renderer->RimColor(0.4f, 0.4f, 0.8f);
        
        character = scene.CreateObject()->GetComponent<Character>();

        SceneObject* pelvis = character->GetObject()->FindObject("Pelvis");
        if(pelvis)
        {
            //pelvis->Get<Animation>()->SetRelativeTranslation(true);

            pelvis->GetComponent<LightOmni>()->Intensity(1.0f);
            pelvis->GetComponent<LightOmni>()->Color(0.8f, 0.8f, 0.8f);
        }

        character->GetComponent<Transform>()->Translate(0.0f, 0.0f, 3.0f);
        camera = scene.CreateObject()->GetComponent<CharacterCamera>();
        camera->SetTarget(character);
        charController = scene.GetComponent<CharacterController>();
        charController->SetTarget(character);
        
        script = scene.GetComponent<LuaScript>();
        script->SetScript("scene");
        
        SoundEmitter* snd = scene.CreateObject()->GetComponent<SoundEmitter>();
        snd->SetClip("amb01");
        
        std::ofstream file("scene.scn", std::ios::out);
        file << std::setw(4) << scene.Serialize();
        file.close();
        
        animation = scene.GetComponent<Animator>();
        collision = scene.GetComponent<Collision>();
        soundRoot = scene.GetComponent<SoundRoot>();
        
        scene.CreateObject()->GetComponent<TextMesh>()->SetText("Hello, World!");

        quad = scene.CreateObject()->GetComponent<Quad>();
        quad->SetImage("V8fBNZhT");
        quad->SetSize(400, 250);

        title = scene.CreateObject()->GetComponent<Text2d>();
        title->GetComponent<Transform>()->Translate(960, 800, 0);
        title->SetSize(86);
        title->SetFont("FantaisieArtistique");
        title->SetText("Irrelevant Text");

        testCube = scene.CreateObject()->Get<TestCube>();
        testCube->Get<Transform>()->Translate(1.0f, 1.0f, 0.0f);
        testCube->Object()->Name("cube");

        LoadSceneFromFbx(&scene, "data\\scene.fbx");
    }

    virtual void OnCleanup() 
    {
    }

    Text2d* title;
    event_dispatcher<eChar> dispatcher_onChar;
    event_dispatcher<eKeyDown> disp_KeyDown;
    event_dispatcher<eKeyUp> disp_KeyUp;
    event_dispatcher<eMouseMove> disp_onMouseMove;
    event_dispatcher<eMouseDown> disp_onMouseDown;
    event_dispatcher<eMouseUp> disp_onMouseUp;
    TestCube* testCube;
    virtual void OnUpdate() 
    {        
        while(eChar* e = dispatcher_onChar.poll())    
        {            
            if(e->code == 8)
            {
                // Backspace
            }
            else
            {
                //str.push_back(e->code);
            }
        }
        while(auto e = disp_KeyDown.poll())
        {
            script->Relay("KeyDown", (int)e->key);

            if(e->key == Au::Input::KEY_Q){
                character->GetComponent<Transform>()->Position(0.0f, 0.25f, 0.0f);
                character->Get<Transform>()->Rotation(0.0f, 0.0f, 0.0f, 1.0f);
            }
            if(e->key == Au::Input::KEY_E)
                character->GetComponent<Transform>()->Position(0.0f, 2.25f, -15.0f);
            if(e->key == Au::Input::KEY_R)
                character->GetComponent<Transform>()->Position(7.0f, 5.25f, -10.0f);
        }
        while(auto e = disp_KeyUp.poll())
        {
            script->Relay("KeyUp", (int)e->key);
        }
        while(auto e = disp_onMouseMove.poll())
        {
            script->Relay("MouseMove", e->dx, e->dy);
        }
        SceneObject* o = scene.Get<Collision>()->RayTest(
            gfxm::ray(
                camera->Get<Transform>()->WorldPosition() + camera->Get<Transform>()->Forward() * 0.3f,
                camera->Get<Transform>()->Forward() * 0.7f
            )
        );
        if(o)
        {
            title->SetText(o->Name());
            gfxm::vec2 p = renderer->CurrentCamera()->WorldToScreen(o->Get<Transform>()->WorldPosition());
            title->Get<Transform>()->Position(
                (p.x * 0.5f + 0.5f) * Common.frameSize.x, 
                (Common.frameSize.y - (p.y * 0.5f + 0.5f) * Common.frameSize.y), 
                0.0f
            );
        }
        else
        {
            title->SetText("");
        }
        while(auto e = disp_onMouseDown.poll())
        {
            if(o)
            {
                Interactable* i = o->FindComponent<Interactable>();
                if(i) i->Activate();
            }
        }
        while(auto e = disp_onMouseUp.poll())
        {
            
        }
        testCube->Update();

        scene.Get<GuiRoot>()->Update();

        collision->Update(DeltaTime());
        
        //scene.GetComponent<Dynamics>()->Step(DeltaTime());
        
        script->Relay("Update");
        
        camera->Update(DeltaTime());
        charController->Update();
        
        character->Update(DeltaTime());

        soundRoot->Update();
        //scene.FindObject("MIKU")->GetComponent<Transform>()->Track(character->GetComponent<Transform>()->WorldPosition());
        
    }
    virtual void OnRender()
    {
        renderer->Render();
        //scene.GetComponent<Collision>()->DebugDraw();
        //camera->Render(device);
    }
    
private:    
    SceneObject scene;
    Renderer* renderer;
    LuaScript* script;
    Animator* animation;
    Collision* collision;
    SoundRoot* soundRoot;

    Quad* quad;
    
    Character* character;
    CharacterCamera* camera;
    CharacterController* charController;
    
    float fov = 1.6f;
    float zfar = 100.0f;
};

#endif
