#include "transform.h"

#include "mesh.h"

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
{
    Au::Math::Quat qx = AngleAxis(x, Au::Math::Vec3f(1.0f, 0.0f, 0.0f));
    Au::Math::Quat qy = AngleAxis(y, Au::Math::Vec3f(0.0f, 1.0f, 0.0f));
    Au::Math::Quat qz = AngleAxis(z, Au::Math::Vec3f(0.0f, 0.0f, 1.0f));
    _rotation = qx * qy * qz;
}
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

Au::Math::Vec3f Transform::Right()
{ return GetTransform()[0]; }
Au::Math::Vec3f Transform::Up()
{ return GetTransform()[1]; }
Au::Math::Vec3f Transform::Back()
{ return GetTransform()[2]; }

void Transform::SetTransform(Au::Math::Mat4f& t)
{
    _position = Au::Math::Vec3f(t[3].x, t[3].y, t[3].z);
}

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

void Transform::OnCreate()
{
    GetObject()->GetComponent<DebugTransformIcon>();
}
