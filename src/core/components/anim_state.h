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
        lua.Bind(&PrintTest, "Print");
        lua.Bind(&AnimState::Blend, "Blend");
        lua.Bind(&AnimState::Switch, "Switch");
        lua.SetGlobal("State", this);

        
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
