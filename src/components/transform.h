#ifndef COMPONENT_TRANSFORM_H
#define COMPONENT_TRANSFORM_H

#include <aurora/math.h>

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
    _scale(1.0f, 1.0f, 1.0f)
    {
        if(_parent)
            AttachTo(parent);
    }
    ~Transform()
    {
        if(_parent)
            _parent->Detach(this);
    }
    
    void Translate(float x, float y, float z);
    void Translate(const Au::Math::Vec3f& vec);
    void Rotate(float angle, float axisX, float axisY, float axisZ);
    void Rotate(float angle, const Au::Math::Vec3f& axis);
    
    void Position(float x, float y, float z);
    void Position(const Au::Math::Vec3f& position);
    void Rotation(float x, float y, float z);
    void Rotation(const Au::Math::Quat& rotation);
    void Rotation(float x, float y, float z, float w);
    void Scale(float scale);
    void Scale(float x, float y, float z);
    void Scale(const Au::Math::Vec3f& scale);
    
    Au::Math::Vec3f Position();
    Au::Math::Quat Rotation();
    Au::Math::Vec3f Scale();
    
    Au::Math::Vec3f Right();
    Au::Math::Vec3f Up();
    Au::Math::Vec3f Back();
    
    void SetTransform(Au::Math::Mat4f& t);
    Au::Math::Mat4f GetLocalTransform();
    Au::Math::Mat4f GetTransform();
    
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

    virtual void OnCreate();
private:

    Au::Math::Vec3f _position;
    Au::Math::Quat _rotation;
    Au::Math::Vec3f _scale;
    Au::Math::Mat4f _transform;
    
    Transform* _parent;
    std::vector<Transform*> _children;
};

#endif
