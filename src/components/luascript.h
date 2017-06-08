#ifndef COMPONENT_LUA_SCRIPT_H
#define COMPONENT_LUA_SCRIPT_H

#include "../scene_object.h"
#include "../resource.h"

#include "transform.h"
#include "renderer.h"
#include "camera.h"
#include "light_omni.h"
#include "mesh.h"

#include "animation.h"
#include "skeleton.h"

#include <aurora/lua.h>

class ScriptData
{
public:
    ScriptData(const std::string& source)
    : source(source)
    {}
    std::string& Get() { return source; }
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

inline void LuaPrint(const std::string& msg)
{
    std::cout << msg << std::endl;
}

class LuaScript : public SceneObject::Component
{
public:
    LuaScript()
    : _next(0) {}
    ~LuaScript()
    {
        SceneObject* root = GetObject()->Root();
        LuaScript* rootScript = root->GetComponent<LuaScript>();
        rootScript->_unlink(this);
        _state.Cleanup();
    }
    
    void SetScript(const std::string& name)
    {
        ScriptData* sd = Resource<ScriptData>::Get(name);
        if(!sd)
            return;
        
        _state.SetGlobal(GetObject(), "SceneObject");
        _state.LoadSource(sd->Get());
        _state.Call("Init");
    }
    
    virtual void OnCreate()
    {
        SceneObject* root = GetObject()->Root();
        LuaScript* rootScript = root->GetComponent<LuaScript>();
        rootScript->_link(this);
        
        _state.Init();
        
        _state.Bind(&LuaPrint, "Print");
        
        _state.Bind(&SceneObject::Root, "Root");
        _state.Bind(&SceneObject::CreateObject, "CreateObject");
        _state.Bind(&SceneObject::FindObject, "FindObject");
        _state.Bind(&SceneObject::GetComponent<LuaScript>, "Script");
        _state.Bind(&SceneObject::GetComponent<Transform>, "Transform");
        _state.Bind(&SceneObject::GetComponent<Camera>, "Camera");
        _state.Bind(&SceneObject::GetComponent<LightOmni>, "LightOmni");
        _state.Bind(&SceneObject::GetComponent<LightDirect>, "LightDirect");
        _state.Bind(&SceneObject::GetComponent<Renderer>, "Renderer");
        _state.Bind(&SceneObject::GetComponent<Mesh>, "Mesh");
        _state.Bind(&SceneObject::GetComponent<Animation>, "Animation");
        _state.Bind(&SceneObject::GetComponent<Skeleton>, "Skeleton");
        
        _state.Bind(&LuaScript::SetScript, "SetScript");
        _state.Bind<LuaScript, SceneObject*>(&LuaScript::GetObject, "GetObject");
        
        _state.Bind<Transform, void, float, float, float>(&Transform::Translate, "Translate");
        _state.Bind<Transform, void, float, float, float, float>(&Transform::Rotate, "Rotate");
        _state.Bind<Transform, void, float, float, float>(&Transform::Position, "Position");
        _state.Bind<Transform, void, float, float, float>(&Transform::Rotation, "Rotation");
        _state.Bind<Transform, void, float>(&Transform::Scale, "Scale");
        _state.Bind<Transform, SceneObject*>(&Transform::GetObject, "GetObject");
        
        _state.Bind(&Camera::Perspective, "Perspective");
        _state.Bind<Camera, SceneObject*>(&Camera::GetObject, "GetObject");
        
        _state.Bind<LightOmni, void, float, float, float>(&LightOmni::Color, "Color");
        _state.Bind<LightOmni, SceneObject*>(&LightOmni::GetObject, "GetObject");
        
        _state.Bind<LightDirect, void, float, float, float>(&LightDirect::Color, "Color");
        _state.Bind<LightDirect, void, float, float, float>(&LightDirect::Direction, "Direction");
        _state.Bind<LightDirect, SceneObject*>(&LightDirect::GetObject, "GetObject");
        
        _state.Bind(&Renderer::AmbientColor, "AmbientColor");
        _state.Bind(&Renderer::RimColor, "RimColor");
        _state.Bind<Renderer, SceneObject*>(&Renderer::GetObject, "GetObject");
        
        _state.Bind<Mesh, void, const std::string&>(&Mesh::SetMesh, "SetMesh");
        _state.Bind<Mesh, void, const std::string&>(&Mesh::SetSubMesh, "SetSubMeshName");
        _state.Bind<Mesh, void, unsigned int>(&Mesh::SetSubMesh, "SetSubMeshIndex");
        _state.Bind<Mesh, void, const std::string&>(&Mesh::SetMaterial, "SetMaterial");
        _state.Bind<Mesh, SceneObject*>(&Mesh::GetObject, "GetObject");
        
        _state.Bind<Animation, void, const std::string&>(&Animation::SetAnimData, "SetAnimData");
        _state.Bind<Animation, SceneObject*>(&Animation::GetObject, "GetObject");
        
        _state.Bind<Skeleton, void, const std::string&>(&Skeleton::SetData, "SetData");
        _state.Bind<Skeleton, SceneObject*>(&Skeleton::GetObject, "GetObject");
    }
    
    template<typename... Args>
    void Relay(const std::string& func, Args... args)
    {
        _state.Call(func, args...);
        if(_next) _next->Relay(func);
    }
    
private:
    void _link(LuaScript* script)
    {
        if(script == this)
            return;
        
        if(_next == 0)
            _next = script;
        else
            _next->_link(script);
    }
    
    void _unlink(LuaScript* script)
    {
        if(_next == 0)
            return;
        if(_next == script)
            _next = script->_next;
        else
            _next->_unlink(script);
    }

    Au::Lua _state;
    LuaScript* _next;
};

#endif
