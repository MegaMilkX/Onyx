#ifndef COMPONENT_LUA_SCRIPT_H
#define COMPONENT_LUA_SCRIPT_H

#include "../scene_object.h"
#include <resource.h>

#include "transform.h"
#include "renderer.h"
#include "camera.h"
#include "light_omni.h"
#include "model.h"

#include "animator.h"
#include "skeleton.h"

#include "dynamics/rigid_body.h"
#include "collision/collider.h"

#include "../asset.h"

#include <aurora/lua.h>

class ScriptData
{
public:
    ScriptData()
    : source("")
    {}
    ScriptData(const std::string& source)
    : source(source)
    {}
    std::string& Get() { return source; }
private:
    std::string source;
};

template<>
inline bool LoadAsset<ScriptData, LUA>(ScriptData* script, const std::string& filename)
{
    std::ifstream file(filename);
    if(!file.is_open())
        return false;
    std::string source((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

    *script = ScriptData(source);
    
    file.close();
    return true;
}

inline void LuaPrint(const std::string& msg)
{
    std::cout << msg << std::endl;
}

inline gfxm::vec3 Vec3Create(float x, float y, float z) { return gfxm::vec3(x, y, z); }
inline gfxm::vec3 Vec3Add(const gfxm::vec3& a, const gfxm::vec3& b) { return a + b; }
inline gfxm::vec3 Vec3MultScalar(const gfxm::vec3& v, float s) { return v * s; }

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
        ScriptData* sd = asset<ScriptData>::get(name);
        if(!sd)
            return;
        scriptName = name;
        _state.SetGlobal("SceneObject", GetObject());
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
        
        _state.Bind(&Vec3Create, "Vec3");
        _state.Bind(&Vec3Add, "Vec3Add");
        _state.Bind(&Vec3MultScalar, "Vec3MultScalar");
        _state.Bind<gfxm::vec3, const gfxm::vec3&>(&gfxm::normalize, "Vec3Normalize");
        _state.Bind(&gfxm::vec3::x, "x");
        _state.Bind(&gfxm::vec3::y, "y");
        _state.Bind(&gfxm::vec3::z, "z");

        _state.Bind(&asset<Mesh>::set, "Set");
        _state.Bind(&asset<Material>::set, "Set");
        _state.Bind(&asset<Animation>::set, "Set");
        _state.Bind(&asset<SkeletonData>::set, "Set");
        
        _state.Bind(&SceneObject::Root, "Root");
        _state.Bind(&SceneObject::CreateObject, "CreateObject");
        _state.Bind(&SceneObject::FindObject, "FindObject");
        _state.Bind<SceneObject, void, const std::string&>(&SceneObject::Name, "SetName");
        //_state.Bind<SceneObject, std::string>(&SceneObject::Name, "GetName"); TODO CONST LUA FUNCTIONS
        _state.Bind(&SceneObject::GetComponent<LuaScript>, "Script");
        _state.Bind(&SceneObject::GetComponent<Transform>, "Transform");
        _state.Bind(&SceneObject::GetComponent<Camera>, "Camera");
        _state.Bind(&SceneObject::GetComponent<LightOmni>, "LightOmni");
        _state.Bind(&SceneObject::GetComponent<LightDirect>, "LightDirect");
        _state.Bind(&SceneObject::GetComponent<Renderer>, "Renderer");
        _state.Bind(&SceneObject::GetComponent<Model>, "Model");
        _state.Bind(&SceneObject::GetComponent<Animator>, "Animation");
        _state.Bind(&SceneObject::GetComponent<Skeleton>, "Skeleton");
		_state.Bind(&SceneObject::GetComponent<PlaneCollider>, "PlaneCollider");
		_state.Bind(&SceneObject::GetComponent<SphereCollider>, "SphereCollider");
        _state.Bind(&SceneObject::GetComponent<MeshCollider>, "MeshCollider");
        _state.Bind(&SceneObject::GetComponent<RigidBody>, "RigidBody");
        
        _state.Bind(&LuaScript::SetScript, "SetScript");
        _state.Bind<LuaScript, SceneObject*>(&LuaScript::GetObject, "GetObject");
        
        _state.Bind<Transform, void, float, float, float>(&Transform::Translate, "Translate");
        _state.Bind<Transform, void, float, float, float, float>(&Transform::Rotate, "Rotate");
        _state.Bind<Transform, void, float, const gfxm::vec3&>(&Transform::Rotate, "RotateVec");
        _state.Bind<Transform, void, float, float, float>(&Transform::Position, "Position");
        _state.Bind<Transform, void, const gfxm::vec3&>(&Transform::Position, "PositionVec");
        _state.Bind<Transform, gfxm::vec3>(&Transform::Position, "GetPosition");
        _state.Bind<Transform, void, float, float, float>(&Transform::Rotation, "Rotation");
        _state.Bind<Transform, void, float>(&Transform::Scale, "Scale");
        _state.Bind<Transform, SceneObject*>(&Transform::GetObject, "GetObject");
        _state.Bind(&Transform::Right, "Right");
        _state.Bind(&Transform::Back, "Back");
        _state.Bind(&Transform::Up, "Up");
        _state.Bind(&Transform::Left, "Left");
        _state.Bind(&Transform::Forward, "Forward");
        _state.Bind(&Transform::Down, "Down");
        _state.Bind(&Transform::AttachTo, "AttachTo");
        _state.Bind(&Transform::Attach, "Attach");
        
        _state.Bind(&Camera::Perspective, "Perspective");
        _state.Bind<Camera, SceneObject*>(&Camera::GetObject, "GetObject");
        
        _state.Bind<LightOmni, void, float, float, float>(&LightOmni::Color, "Color");
        _state.Bind<LightOmni, SceneObject*>(&LightOmni::GetObject, "GetObject");
        
        _state.Bind<LightDirect, void, float, float, float>(&LightDirect::Color, "Color");
        _state.Bind<LightDirect, void, float, float, float>(&LightDirect::Direction, "Direction");
        _state.Bind<LightDirect, SceneObject*>(&LightDirect::GetObject, "GetObject");
        
        _state.Bind<Renderer, void, Camera*>(&Renderer::CurrentCamera, "SetCurrentCamera");
        _state.Bind<Renderer, Camera*>(&Renderer::CurrentCamera, "GetCurrentCamera");
        _state.Bind(&Renderer::AmbientColor, "AmbientColor");
        _state.Bind(&Renderer::RimColor, "RimColor");
        _state.Bind<Renderer, SceneObject*>(&Renderer::GetObject, "GetObject");
        
        _state.Bind(&Model::mesh, "mesh");
        _state.Bind(&Model::material, "material");
        _state.Bind<Model, SceneObject*>(&Model::GetObject, "GetObject");
        
        _state.Bind<Animator, void, const std::string&>(&Animator::Set, "SetAnim");
        _state.Bind(&Animator::Play, "Play");
        _state.Bind<Animator, SceneObject*>(&Animator::GetObject, "GetObject");
        
        _state.Bind<Skeleton, void, const std::string&>(&Skeleton::SetData, "SetData");
        _state.Bind<Skeleton, SceneObject*>(&Skeleton::GetObject, "GetObject");
		
		_state.Bind<MeshCollider, void, const std::string&>(&MeshCollider::SetMesh, "SetMesh");
        
        _state.Bind<RigidBody, void, const gfxm::vec3&>(&RigidBody::SetLinearVelocity, "SetLinearVelocity");
        _state.Bind(&RigidBody::SetLinearFactor, "SetLinearFactor");
        _state.Bind(&RigidBody::SetAngularFactor, "SetAngularFactor");
        _state.Bind(&RigidBody::LookAt, "LookAt");
    }
    virtual std::string Serialize() 
    {
        using json = nlohmann::json;
        json j = json::object();
        j["Script"] = scriptName;
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Script"].is_string())
        {
            SetScript(j["Script"].get<std::string>());
        }
    }
    
    template<typename... Args>
    void Relay(const std::string& func, Args... args)
    {
        _state.Call(func, args...);
        if(_next) _next->Relay(func, args...);
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

    std::string scriptName;
    Au::Lua _state;
    LuaScript* _next;
};

#endif
