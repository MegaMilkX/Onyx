#ifndef ANIM_STATE_H
#define ANIM_STATE_H

#include <string>
#include <aurora/lua.h>
#include <util/gfxm.h>
#include <scene_object.h>
#include <animator.h>

inline void PrintTest(const std::string& v)
{
    std::cout << v << std::endl;
}

class AnimState : public SceneObject::Component
{
public:
    AnimState()
    {
        lua.Init();

        lua.Bind(&Transform::Back, "Back");
        lua.Bind(&Transform::Forward, "Front");
        lua.Bind(&Transform::Up, "Up");
        lua.Bind<Transform, void, float, float, float>(&Transform::Translate, "Translate");
        lua.Bind<Transform, gfxm::vec3>(&Transform::Position, "GetPosition");
        lua.Bind<Transform, void, float, float, float>(&Transform::Position, "SetPosition");
        lua.Bind(&Transform::LookDir, "LookDir");

        lua.Bind(&Animator::BlendOverTime, "Blend");
        lua.Bind(&Animator::ApplyBlend, "ApplyBlend");
        lua.Bind(&Animator::ApplyAdd, "ApplyAdd");
        lua.Bind(&Animator::GetAnimCursor, "GetCursor");
        lua.Bind(&Animator::Stopped, "Stopped");

        lua.Bind(&AnimTrack::Cursor::Advance, "Advance");
        lua.Bind(&AnimTrack::Cursor::SetFrame, "SetFrame");

        lua.Bind(&gfxm::vec3::x, "x");
        lua.Bind(&gfxm::vec3::y, "y");
        lua.Bind(&gfxm::vec3::z, "z");

        lua.Bind(&PrintTest, "Print");
        lua.Bind(&AnimState::Blend, "Blend");
        lua.Bind(&AnimState::Switch, "Switch");
        lua.SetGlobal("State", this);        
        
    }

    void OnCreate()
    {
        lua.SetGlobal("Animator", Get<Animator>());
        lua.SetGlobal("Transform", Get<Transform>());
        layerMotion1 = asset<Animation>::get("character")->operator[]("LayerMotion01")->GetCursor();
    }

    void AppendScript(const std::string& source)
    {
        lua.LoadSource(source);
    }
    void Blend(const std::string& anim, float blendSpeed = 0.0f)
    {
        Get<Animator>()->BlendOverTime(anim, blendSpeed);
    }
    void Switch(const std::string& state)
    {
        currentState = state;
        lua.CallMember(currentState, "Start");
    }
    template<typename T>
    void Set(const std::string& name, const T& data)
    {
        lua.SetGlobal(name, &data);
    }

    void Update()
    {
        Get<Animator>()->Tick(GameState::DeltaTime());
        lua.CallMember(currentState, "Update");

        layerMotion1 += GameState::DeltaTime();
        Get<Animator>()->ApplyAdd(layerMotion1, 1.0f);
        
        Get<Animator>()->Finalize();
    }
private:
    AnimTrack::Cursor layerMotion1;

    std::string currentState;
    Au::Lua lua;
};

#endif
