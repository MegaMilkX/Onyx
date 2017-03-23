#ifndef COMPONENT_TRANSFORM_H
#define COMPONENT_TRANSFORM_H

#include <aurora/math.h>

#include "../scene_object.h"

class Transform : public SceneObject::Component
{
public:
    Transform()
    : _parent(0) {}
    Transform(Transform* parent)
    : _parent(parent) 
    {
        if(_parent)
            _parent->_addChild(this);
    }
    ~Transform()
    {
        if(_parent)
            _parent->_removeChild(this);
    }
    
    void Translate(float x, float y, float z);
    void Translate(const Au::Math::Vec3f& vec);
    void Rotate(float angle, float axisX, float axisY, float axisZ);
    void Rotate(float angle, const Au::Math::Vec3f& axis);
    
    void Position(float x, float y, float z);
    void Position(const Au::Math::Vec3f& position);
    void Rotation(float x, float y, float z);
    void Rotation(const Au::Math::Quat& rotation);
    void Scale(float scale);
    void Scale(float x, float y, float z);
    void Scale(const Au::Math::Vec3f& scale);
    
    Au::Math::Vec3f Position();
    Au::Math::Quat Rotation();
    Au::Math::Vec3f Scale();
    
    Au::Math::Mat4f GetTransform();

    virtual void OnCreate()
    {
        
    }
private:
    void _addChild(Transform* transform);
    void _removeChild(Transform* transform);

    Au::Math::Vec3f _position;
    Au::Math::Quat _rotation;
    Au::Math::Vec3f _scale;
    Au::Math::Mat4f _transform;
    
    Transform* _parent;
    std::vector<Transform*> _children;
};

#endif
