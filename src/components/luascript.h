#ifndef COMPONENT_LUA_SCRIPT_H
#define COMPONENT_LUA_SCRIPT_H

#include "../scene_object.h"
#include "../resource.h"

#include <aurora/lua.h>

class ScriptData
{
public:
    ScriptData(const std::string& source)
    : source(source)
    {}
    std::string Get() { return source; }
private:
    std::string source;
};

class ScriptReaderLUA : public Resource<ScriptData>::Reader
{
public:
    ScriptData* operator()(const std::string& filename)
    {
        std::ifstream file(filename);
        if(!file.is_open())
            return 0;
        std::string source((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
                 
        return new ScriptData(source);
        
        file.close();
    }
};

class LuaScript : public SceneObject::Component
{
public:
    LuaScript()
    : _next(0) {}
    ~LuaScript()
    {
        _state.Cleanup();
    }
    
    virtual void OnCreate()
    {
        _state.Init();
        _state.Bind(&SceneObject::Root, "Root");
        _state.Bind(&SceneObject::CreateSceneObject, "CreateObject");
        _state.Bind(&SceneObject::GetComponent<LuaScript>, "Script");
        _state.Bind(&SceneObject::GetComponent<Transform>, "Transform");
        _state.Bind(&SceneObject::GetComponent<Camera>, "Camera");
    }
    
    void Relay(const std::string& func)
    {
        if(_next) _next->Relay(func);
    }
    
private:
    Au::Lua _state;
    LuaScript* _next;
};

#endif
