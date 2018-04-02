#ifndef ANIM_STATE_H
#define ANIM_STATE_H

#include <string>
#include <aurora/lua.h>
#include <aurora/math.h>
#include <scene_object.h>
#include <animation.h>

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
        lua.SetGlobal("Transform", Get<Transform>());
    }

    void AppendScript(const std::string& source)
    {
        lua.LoadSource(source);
    }
    void Blend(const std::string& anim, float blendSpeed = 0.0f)
    {
        Get<Animation>()->BlendOverTime(anim, blendSpeed);
    }
    void Switch(const std::string& state)
    {
        currentState = state;
        lua.CallMember(currentState, "Start");
        std::cout << state << std::endl;
    }
    template<typename T>
    void Set(const std::string& name, const T& data)
    {
        lua.SetGlobal(name, &data);
    }

    void Update()
    {
        lua.CallMember(currentState, "Update");
    }
private:
    std::string currentState;
    Au::Lua lua;
};

#endif