#include "transform.h"

void Transform::Translate(float x, float y, float z)
{ Translate(Au::Math::Vec3f(x, y, z)); }
void Transform::Translate(const Au::Math::Vec3f& vec)
{
    _position = _position + vec;
}
void Transform::Rotate(float angle, float axisX, float axisY, float axisZ)
{ Rotate(angle, Au::Math::Vec3f(axisX, axisY, axisZ)); }
void Transform::Rotate(float angle, const Au::Math::Vec3f& axis)
{
    _rotation = 
        Au::Math::Normalize(
            Au::Math::AngleAxis(angle, axis) * 
            _rotation
        );
}

void Transform::Position(float x, float y, float z)
{ Position(Au::Math::Vec3f(x, y, z)); }
void Transform::Position(const Au::Math::Vec3f& position)
{ _position = position; }
void Transform::Rotation(float x, float y, float z)
{  }
void Transform::Rotation(const Au::Math::Quat& rotation)
{ _rotation = rotation; }

void Transform::Scale(float scale)
{ Scale(Au::Math::Vec3f(scale, scale, scale)); }
void Transform::Scale(float x, float y, float z)
{ Scale(Au::Math::Vec3f(x, y, z)); }
void Transform::Scale(const Au::Math::Vec3f& scale)
{ _scale = scale; }

Au::Math::Vec3f Transform::Position()
{ return _position; }
Au::Math::Quat Transform::Rotation()
{ return _rotation; }
Au::Math::Vec3f Transform::Scale()
{ return _scale; }

Au::Math::Mat4f Transform::GetTransform()
{
    Au::Math::Mat4f localTransform = 
            Au::Math::Translate(Au::Math::Mat4f(1.0f), _position) * 
            Au::Math::ToMat4(_rotation) * 
            Au::Math::Scale(Au::Math::Mat4f(1.0f), _scale);
            
    if(_parent)
        _transform = _parent->GetTransform() * localTransform;
    else
        _transform = localTransform;
    
    return _transform;
}

void Transform::_addChild(Transform* transform)
{
    _removeChild(transform);
    _children.push_back(transform);
}

void Transform::_removeChild(Transform* transform)
{
    for(unsigned i = 0; i < _children.size(); ++i)
    {
        if(_children[i] == transform)
        {
            _children.erase(_children.begin() + i);
            break;
        }
    }
}