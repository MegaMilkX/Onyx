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

    FpsDisplay* fpsDisplay;
    
    virtual void OnInit() 
    {
        GameState::GetMouseHandler()->Locked(true);
        GameState::GetMouseHandler()->Visible(false);

        renderer = scene.GetComponent<Renderer>();
        renderer->Init(GFXDevice());
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
        text = scene.CreateObject()->GetComponent<Text2d>();
        text->GetComponent<Transform>()->Translate(0, 200, 0);
        text->SetText(std::vector<int>{0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21});
        text->SetSize(20);

        fpsDisplay = scene.Get<FpsDisplay>();

        title = scene.CreateObject()->GetComponent<Text2d>();
        title->GetComponent<Transform>()->Translate(960, 800, 0);
        title->SetSize(86);
        title->SetFont("FantaisieArtistique");
        title->SetText("Irrelevant Text");

        GuiBox* box = scene.CreateObject()->Get<GuiBox>();
        box->Get<GuiLayout>()->Dock(GuiLayout::UP);
        box->Get<GuiLayout>()->SetSize(25, 25);
        GuiBox* box2 = scene.CreateObject()->Get<GuiBox>();
        box2->Get<GuiLayout>()->Dock(GuiLayout::DOWN);
        box2->Get<GuiLayout>()->SetSize(25, 25);
        GuiBox* box3 = scene.CreateObject()->Get<GuiBox>();
        box3->Get<GuiLayout>()->Dock(GuiLayout::LEFT);
        box3->Get<GuiLayout>()->SetSize(100, 100);

        testCube = scene.CreateObject()->Get<TestCube>();
        testCube->Get<Transform>()->Translate(1.0f, 1.0f, 0.0f);
        testCube->Object()->Name("cube");
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
    TestCube* testCube;
    virtual void OnUpdate() 
    {
        fpsDisplay->Update(DeltaTime());

        while(eChar* e = dispatcher_onChar.poll())    
        {
            if(e->code == 8)
            {
                if(!str.empty())
                    str.pop_back();
            }
            else
            {
                //str.push_back(e->code);
            }
            text->SetText(str);
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
                (p.x * 0.5f + 0.5f) * 1920, 
                (1080 - (p.y * 0.5f + 0.5f) * 1080), 
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
    virtual void OnRender(Au::GFX::Device* device)
    {
        renderer->Render();
        //scene.GetComponent<Collision>()->DebugDraw();
        //camera->Render(device);
    }
    
    Text2d* fpsText;
    Text2d* text;
    std::vector<int> str;
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
