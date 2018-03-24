#ifndef STATE_GAMEPLAY_H
#define STATE_GAMEPLAY_H

#include <stdlib.h>

#include "game_state.h"
#include "scene_object.h"

#include "state_test.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/model.h"
#include "components/light_omni.h"
#include "components/luascript.h"
#include "components/animation.h"
#include "components/skeleton.h"
#include "components/dynamics/rigid_body.h"
#include "components/collision/collider.h"
#include "components/sound_emitter.h"
#include "components/text_mesh.h"
#include "components/overlay/overlay_root.h"
#include "components/gui/gui_root.h"
#include "components/gui/gui_box.h"
#include "components/gui/gui_window.h"
#include "components/gui/gui_layout.h"

#include "lib/font_rasterizer.h"

#include "actor.h"

#include "util.h"

class CharacterController : public SceneObject::Component
{
public:
    CharacterController()
    : chara(0), dirFlags(0) {}
    
    void SetTarget(Actor* chara)
    {
        this->chara = chara;
    }

    void Update()
    {
        if(!chara)
            return;
        Transform* camTrans = renderer->CurrentCamera()->GetComponent<Transform>();
        Au::Math::Vec3f t(0.0f, 0.0f, 0.0f);
        if(dirFlags & 1)
            t += Au::Math::Normalize(Au::Math::Vec3f(-camTrans->Back().x, 0.0f, -camTrans->Back().z));
        if(dirFlags & 2)
            t += Au::Math::Normalize(Au::Math::Vec3f(-camTrans->Right().x, 0.0f, -camTrans->Right().z));
        if(dirFlags & 4)
            t += Au::Math::Normalize(Au::Math::Vec3f(camTrans->Back().x, 0.0f, camTrans->Back().z));
        if(dirFlags & 8)
            t += Au::Math::Normalize(Au::Math::Vec3f(camTrans->Right().x, 0.0f, camTrans->Right().z));
        
        t = Au::Math::Normalize(t);
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
    Actor* chara;
    char dirFlags;
};

class CharacterCamera : public SceneObject::Component
{
public:
    void SetTarget(SceneObject* so)
    { target = so->GetComponent<Transform>(); }
    void SetTarget(SceneObject::Component* com)
    { target = com->GetComponent<Transform>(); }

    void MouseMove(int x, int y)
    {
        transform->Rotate(-x * 0.005f, 0.0f, 1.0f, 0.0f);
        transform->Rotate(-y * 0.005f, transform->Right());
    }
    
    event_dispatcher<eMouseMove> disp_onMouseMove;
    void Update(float dt)
    {
        while(eMouseMove* e = disp_onMouseMove.poll())
            MouseMove(e->dx, e->dy);
        
        Au::Math::Vec3f tgt = target->Position();
        tgt.y += 1.5f;
        tgt = (tgt - transform->Position()) * (dt * 7.0f);
        transform->Translate(tgt);
        
        Collision::RayHit hit;
        if(GetObject()->Root()->GetComponent<Collision>()->RayTest(Au::Math::Ray(transform->Position(), transform->Back() * 1.5f), hit))
        {
            cam->GetComponent<Transform>()->Position(0.0f, 0.0f, (hit.position - transform->Position()).length() - 0.1f);
        }
        else
        {
            cam->GetComponent<Transform>()->Position(0.0f, 0.0f, 1.5f);
        }
    }
    
    void OnCreate()
    {
        transform = GetComponent<Transform>();
        target = GetObject()->Root()->GetComponent<Transform>();
        
        cam = GetObject()->CreateObject()->GetComponent<Camera>();
        cam->Perspective(1.4f, 16.0f/9.0f, 0.01f, 1000.0f);
        cam->GetComponent<Transform>()->Translate(0.0, 0.0, 1.5);
        cam->GetComponent<Transform>()->AttachTo(transform);
        
        cam->GetComponent<SoundListener>();
    }
private:
    Transform* transform;
    Transform* target;
    Camera* cam;
};

class Gameplay : public GameState
{
public:
    Gameplay()
    {}
    
    virtual void OnInit() 
    {        
        renderer = scene.GetComponent<Renderer>();
        renderer->Init(GFXDevice());
        renderer->AmbientColor(0.1f, 0.1f, 0.1f);
        renderer->RimColor(0.4f, 0.4f, 0.8f);
        
        character = scene.CreateObject()->GetComponent<Actor>();
        SceneObject* pelvis = character->GetObject()->FindObject("Pelvis");
        if(pelvis)
        {
            pelvis->GetComponent<LightOmni>()->Intensity(1.0f);
            pelvis->GetComponent<LightOmni>()->Color(0.8f, 0.8f, 0.8f);
        }
        
        character->GetComponent<Transform>()->Translate(0.0f, 50.0f, 3.0f);
        camera = scene.CreateObject()->GetComponent<CharacterCamera>();
        camera->SetTarget(character);
        charController = scene.GetComponent<CharacterController>();
        charController->SetTarget(character);
        
        script = scene.GetComponent<LuaScript>();
        script->SetScript("scene");
        
        SoundEmitter* snd = scene.CreateObject()->GetComponent<SoundEmitter>();
        snd->SetClip("test");
        
        std::ofstream file("scene.scn", std::ios::out);
        file << std::setw(4) << scene.Serialize();
        file.close();
        
        animation = scene.GetComponent<Animation>();
        collision = scene.GetComponent<Collision>();
        soundRoot = scene.GetComponent<SoundRoot>();
        
        scene.CreateObject()->GetComponent<TextMesh>()->SetText("Hello, World!");

        quad = scene.CreateObject()->GetComponent<Quad>();
        quad->SetImage("AngeHalloween_A");
        quad->SetSize(960, 800);
        text = scene.CreateObject()->GetComponent<Text2d>();
        text->GetComponent<Transform>()->Translate(0, 200, 0);
        text->SetText(std::vector<int>{0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21});
        text->SetSize(20);

        fpsText = scene.CreateObject()->GetComponent<Text2d>();
        fpsText->SetSize(16);
        fpsText->Get<Transform>()->Position(0.0f, 0.0f, 0.0f);
        //text->font->set("calibri");

        Text2d* t = scene.CreateObject()->GetComponent<Text2d>();
        t->GetComponent<Transform>()->Translate(0, 500, 0);
        t->SetSize(32);
        t->SetText(R"(Three Rings for the Elven-kings under the sky,
Seven for the Dwarf-lords in their halls of stone,
Nine for Mortal Men doomed to die,
One for the Dark Lord on his dark throne
In the Land of Mordor where the Shadows lie.
One Ring to rule them all, One Ring to find them,
One Ring to bring them all and in the darkness bind them
In the Land of Mordor where the Shadows lie.)");
        t->SetFont("Cup and Talon");

        Text2d* title = scene.CreateObject()->GetComponent<Text2d>();
        title->GetComponent<Transform>()->Translate(960, 500, 0);
        title->SetSize(86);
        title->SetFont("FantaisieArtistique");
        title->SetText("Fantasy Title");

        GuiBox* box = scene.CreateObject()->Get<GuiBox>();
        box->Get<GuiLayout>()->Dock(GuiLayout::UP);
        box->Get<GuiLayout>()->SetSize(25, 25);
        GuiBox* box2 = scene.CreateObject()->Get<GuiBox>();
        box2->Get<GuiLayout>()->Dock(GuiLayout::DOWN);
        box2->Get<GuiLayout>()->SetSize(25, 25);
        //box->Get<Transform>()->Translate(120.0, 300.0, 0.0);
        //box->Get<GuiLayout>()->SetSize(300, 300);
    }
    virtual void OnCleanup() 
    {
    }

    float fps = 0.0f;
    event_dispatcher<eChar> dispatcher_onChar;
    event_dispatcher<eKeyDown> disp_KeyDown;
    event_dispatcher<eKeyUp> disp_KeyUp;
    event_dispatcher<eMouseMove> disp_onMouseMove;
    virtual void OnUpdate() 
    {
        while(eChar* e = dispatcher_onChar.poll())    
        {
            if(e->code == 8)
            {
                if(!str.empty())
                    str.pop_back();
            }
            else
            {
                str.push_back(e->code);
            }
            text->SetText(str);
        }
        while(auto e = disp_KeyDown.poll())
        {
            script->Relay("KeyDown", (int)e->key);

            if(e->key == Au::Input::KEY_Q)
                character->GetComponent<Transform>()->Position(0.0f, 0.25f, 0.0f);
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

        fps = 1.0f / DeltaTime();
        std::string s = std::to_string(fps);
        fpsText->SetText(std::string("FPS: ") + s);

        scene.Get<GuiRoot>()->Update();

        animation->Update(DeltaTime());
        collision->Update(DeltaTime());
        
        //scene.GetComponent<Dynamics>()->Step(DeltaTime());
        
        script->Relay("Update");
        
        camera->Update(DeltaTime());
        charController->Update();
        
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
    Animation* animation;
    Collision* collision;
    SoundRoot* soundRoot;

    Quad* quad;
    
    Actor* character;
    CharacterCamera* camera;
    CharacterController* charController;
    
    float fov = 1.6f;
    float zfar = 100.0f;
};

#endif
