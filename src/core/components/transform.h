#ifndef COMPONENT_TRANSFORM_H
#define COMPONENT_TRANSFORM_H

#include <util/gfxm.h>

#include "../scene_object.h"

class Transform : public SceneObject::Component
{
public:
    Transform()
    : Transform(0) {}
    Transform(Transform* parent)
    : _parent(parent),
    _position(0.0f, 0.0f, 0.0f),
    _rotation(0.0f, 0.0f, 0.0f, 1.0f),
    _scale(1.0f, 1.0f, 1.0f),
    dirty(true)
    {
        if(_parent)
            AttachTo(parent);
    }
    ~Transform()
    {
        if(_parent)
            _parent->Detach(this);
    }

    void Dirty();
    
    void Translate(float x, float y, float z);
    void Translate(const gfxm::vec3& vec);
    void Rotate(float angle, float axisX, float axisY, float axisZ);
    void Rotate(float angle, const gfxm::vec3& axis);
    void Rotate(const gfxm::quat& q);
    
    void LookAt(const gfxm::vec3& target, const gfxm::vec3& forward, const gfxm::vec3& up = gfxm::vec3(0.0f, 1.0f, 0.0f), float f = 1.0f);
    void LookDir(const gfxm::vec3& dir, const gfxm::vec3& forward, const gfxm::vec3& up = gfxm::vec3(0.0f, 1.0f, 0.0f), float f = 1.0f);
    void LookAtChain(const gfxm::vec3& target, const gfxm::vec3& forward, const gfxm::vec3& up, float f = 1.0f, int chain = 2);
    
    void Track(const gfxm::vec3& target, float f = 1.0f);
    void Track(const gfxm::vec3& target, const gfxm::vec3& forward, float f = 1.0f);
    
    void IKChain(const gfxm::vec3& target, int chain);
    
    void FABRIK(const gfxm::vec3& target, int chainLength);
    
    void Position(float x, float y, float z);
    void Position(const gfxm::vec3& position);
    void Rotation(float x, float y, float z);
    void Rotation(const gfxm::quat& rotation);
    void Rotation(float x, float y, float z, float w);
    void Scale(float scale);
    void Scale(float x, float y, float z);
    void Scale(const gfxm::vec3& scale);
    
    gfxm::vec3 WorldPosition();
    gfxm::vec3 Position();
    gfxm::quat WorldRotation();
    gfxm::quat Rotation();
    gfxm::vec3 Scale();
    
    gfxm::vec3 Right();
    gfxm::vec3 Up();
    gfxm::vec3 Back();
    gfxm::vec3 Left();
    gfxm::vec3 Down();
    gfxm::vec3 Forward();
    
    gfxm::quat GetParentRotation();

    void SetTransform(gfxm::mat4& t);
    gfxm::mat4 GetLocalTransform();
    gfxm::mat4 GetParentTransform();
    gfxm::mat4 GetTransform();

    void ToWorldPosition(gfxm::vec3& pos)
    {
        gfxm::vec4 posw(pos.x, pos.y, pos.z, 1.0f);
        posw = (GetTransform()) * posw;
        pos = gfxm::vec3(posw.x, posw.y, posw.z);
        //pos = gfxm::inverse(GetTransform()) * pos;
    }

    void ToWorldDirection(gfxm::vec3& dir)
    {
        gfxm::mat3 m3 = gfxm::to_mat3(GetParentTransform());
        dir = m3 * dir;
    }

    void ToWorldRotation(gfxm::quat& q)
    {
        if(_parent)
        {
            q = gfxm::normalize(gfxm::inverse(WorldRotation()) * q);
        }
    }

    void ToWorldScale(gfxm::vec3& vec)
    {
        vec = gfxm::inverse(GetTransform()) * vec;
    }
    
    void AttachTo(Transform* parent)
    { parent->Attach(this); }
    void Attach(Transform* trans)
    {
        Detach(trans);
        _children.push_back(trans);
        trans->_parent = this;
    }
    void Detach(Transform* trans)
    {
        for(unsigned i = 0; i < _children.size(); ++i)
            if(_children[i] == trans)
            {
                _children[i]->_parent = 0;
                _children.erase(_children.begin() + i);
            }
    }
    Transform* ParentTransform() { return _parent; }

    virtual void OnCreate();
    virtual std::string Serialize() 
    {
        using json = nlohmann::json;
        json j = json::object();
        j["Translation"] = { _position.x, _position.y, _position.z };
        j["Rotation"] = { _rotation.x, _rotation.y, _rotation.z, _rotation.w };
        j["Scale"] = { _scale.x, _scale.y, _scale.z };
        return j.dump();
    }
    virtual void Deserialize(const std::string& data) 
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        
        json jt = j["Translation"];
        if(jt.is_array() && jt.size() == 3)
        {
            Position(
                jt[0].get<float>(),
                jt[1].get<float>(),
                jt[2].get<float>()
            );
        }
        
        json jr = j["Rotation"];
        if(jr.is_array())
        {
            if(jr.size() == 3)
                Rotation(
                    jr[0].get<float>(),
                    jr[1].get<float>(),
                    jr[2].get<float>()
                );
            if(jr.size() == 4)
                Rotation(
                    jr[0].get<float>(),
                    jr[1].get<float>(),
                    jr[2].get<float>(),
                    jr[3].get<float>()
                );
        }           
        
        json js = j["Scale"];
        if(js.is_array() && js.size() == 3)
        {
            Scale(
                js[0].get<float>(),
                js[1].get<float>(),
                js[2].get<float>()
            );
        }
        else if(js.is_number())
        {
            Scale(js.get<float>());
        }
    }
private:
    bool dirty;
    gfxm::vec3 _position;
    gfxm::quat _rotation;
    gfxm::vec3 _scale;
    gfxm::mat4 _transform;
    
    Transform* _parent;
    std::vector<Transform*> _children;
};

#endif
