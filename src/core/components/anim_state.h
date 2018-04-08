#ifndef ANIM_STATE_H
#define ANIM_STATE_H

#include <string>
#include <aurora/lua.h>
#include <aurora/math.h>
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
        lua.Bind<Transform, Au::Math::Vec3f>(&Transform::Position, "GetPosition");
        lua.Bind<Transform, void, float, float, float>(&Transform::Position, "SetPosition");
        lua.Bind(&Transform::LookDir, "LookDir");

        lua.Bind(&Animator::BlendOverTime, "Blend");
        lua.Bind(&Animator::ApplyBlend, "ApplyBlend");
        lua.Bind(&Animator::ApplyAdd, "ApplyAdd");
        lua.Bind(&Animator::GetAnimCursor, "GetCursor");
        lua.Bind(&Animator::Stopped, "Stopped");

        lua.Bind(&AnimTrack::Cursor::Advance, "Advance");

        lua.Bind(&Au::Math::Vec3f::x, "x");
        lua.Bind(&Au::Math::Vec3f::y, "y");
        lua.Bind(&Au::Math::Vec3f::z, "z");

        lua.Bind(&PrintTest, "Print");
        lua.Bind(&AnimState::Blend, "Blend");
        lua.Bind(&AnimState::Switch, "Switch");
        lua.SetGlobal("State", this);        
        
    }

    void OnCreate()
    {
        lua.SetGlobal("Animator", Get<Animator>());
        lua.SetGlobal("Transform", Get<Transform>());
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
        Get<Animator>()->Finalize();
    }
private:
    std::string currentState;
    Au::Lua lua;
};

#endif
