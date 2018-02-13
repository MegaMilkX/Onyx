#include "transform.h"

#include "mesh.h"

typedef Au::Math::Vec4f vec4;
typedef Au::Math::Vec3f vec3;
typedef Au::Math::Vec2f vec2;
typedef Au::Math::Mat4f mat4;
typedef Au::Math::Mat3f mat3;
typedef Au::Math::Quat quat;

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
    Rotate(Au::Math::AngleAxis(angle, axis));
}

void Transform::Rotate(const Au::Math::Quat& q)
{
    _rotation = 
        Au::Math::Normalize(
            q * 
            _rotation
        );
}

void Transform::LookAt(const Au::Math::Vec3f& target, const Au::Math::Vec3f& forward, const Au::Math::Vec3f& up, float f)
{
	f = max(-1.0f, min(f, 1.0f));
	
    Transform* trans = GetObject()->GetComponent<Transform>();
    Au::Math::Mat4f mat = trans->GetTransform();
    Au::Math::Vec3f pos = mat[3];
    
    Au::Math::Vec3f newFwdUnit = Au::Math::Normalize(target - pos);
    Au::Math::Vec3f rotAxis = Au::Math::Normalize(Au::Math::Cross(forward, newFwdUnit));
    
    Au::Math::Quat q;
    float dot = Au::Math::Dot(forward, newFwdUnit);
	
    const float eps = 0.01f;
    if(fabs(dot + 1.0f) <= eps)
    {
        q = Au::Math::AngleAxis(Au::Math::PI * f, trans->Up());
    }/*
    else if(fabs(dot - 1.0f) <= eps)
    {
        q = Au::Math::Quat(0.0f, 0.0f, 0.0f, 1.0f);
    }*/
    else
	{
        float rotAngle = acosf(max(-1.0f, min(dot, 1.0f))) * f;
        q = Au::Math::AngleAxis(rotAngle, rotAxis);
    }
    
    trans->Rotate(q);
}

void Transform::LookAtChain(const Au::Math::Vec3f& target, const Au::Math::Vec3f& forward, const Au::Math::Vec3f& up, float f, int chain)
{
    float fStep = f / (float)chain;
    if(chain<0)
        return;
    chain--;
    if(_parent)
    {
       _parent->LookAtChain(target, forward, up, fStep, chain);
    }
    
    LookAt(target, forward, up, fStep);
}

void Transform::Track(const Au::Math::Vec3f& target, float f)
{
    Track(target, Forward(), f);
}

void Transform::Track(const Au::Math::Vec3f& target, const Au::Math::Vec3f& forward, float f)
{
    vec3 vforward = Au::Math::Normalize(forward);
    vec3 vtarget = Au::Math::Normalize(target - WorldPosition());
    vec3 rotationAxis = Au::Math::Normalize(Au::Math::Cross(vforward, vtarget));
    rotationAxis = Au::Math::Inverse(GetTransform()) * vec4(rotationAxis.x, rotationAxis.y, rotationAxis.z, 0.0f);
    float dot = Au::Math::Dot(vforward, vtarget);
    float angle = (float)acos(max(-1.0f, min(dot, 1.0f)));
    
    quat q = Au::Math::AngleAxis(angle, rotationAxis);
    Rotate(q);
}

void IKCollectChain(Transform* t, std::vector<Transform*>& r, int chainLength)
{
    if(!t)
        return;
    if(chainLength > 0)
    {
        r.push_back(t);
        chainLength--;
        IKCollectChain(t->ParentTransform(), r, chainLength);
    }
}

void Transform::IKChain(const Au::Math::Vec3f& target, int chain)
{
    std::vector<Transform*> bones;
    IKCollectChain(this, bones, chain);
    if(bones.empty())
        return;
    Transform* origin = bones.back();
    Transform* end = bones.front();
    
    vec3 vforward = Au::Math::Normalize(end->WorldPosition() - origin->WorldPosition());
    vec3 vtarget = Au::Math::Normalize(target - origin->WorldPosition());
    vec3 rotationAxis = Au::Math::Normalize(Au::Math::Cross(vforward, vtarget));
    rotationAxis = Au::Math::Inverse(origin->GetTransform()) * vec4(rotationAxis.x, rotationAxis.y, rotationAxis.z, 0.0f);
    float dot = Au::Math::Dot(vforward, vtarget);
    float angle = (float)acos(max(-1.0f, min(dot, 1.0f)));
    quat q = Au::Math::AngleAxis(angle, rotationAxis);
    origin->Rotate(q);
}

struct ChainBone
{
    vec3 pos;
    vec3 dir;
    float length;
    Transform* t;
};

void IKCollectBones(Transform* t, std::vector<ChainBone>& r, int chainLength)
{
    if(!t)
        return;
    if(chainLength > 0)
    {
        ChainBone bone;
        bone.pos = t->WorldPosition();
        if(r.empty())
            bone.length = 1.0f;
        else
        {
            bone.length = (r.back().pos - bone.pos).length();
            bone.dir = Au::Math::Normalize(bone.pos - r.back().pos);
        }
        bone.t = t;
        r.push_back(bone);
        chainLength--;
        IKCollectBones(t->ParentTransform(), r, chainLength);
    }
}

void FABRIK_impl(std::vector<ChainBone>& bones, const Au::Math::Vec3f& target)
{
    vec3 tgt = target;
    for(unsigned i = 1; i < bones.size() - 1; ++i)
    {
        ChainBone& b = bones[i];
        ChainBone& next = bones[i + 1];
        
        b.pos = tgt;
        tgt = tgt + Au::Math::Normalize(next.pos - b.pos) * next.length;
    }
    
    tgt = bones.back().pos;
    for(int i = (int)bones.size() - 2; i >= 1; --i)
    {
        ChainBone& b = bones[i];
        ChainBone& next = bones[i - 1];
        
        b.pos = tgt;
        tgt = tgt + Au::Math::Normalize(b.pos - next.pos) * b.length;
    }
}

void Transform::FABRIK(const Au::Math::Vec3f& target, int chainLength)
{
    std::vector<ChainBone> bones;
    IKCollectBones(this, bones, chainLength);
    if(bones.empty())
        return;
    
    ChainBone b;
    b.pos = target;
    b.length = 1.0f;
    bones.insert(bones.begin(), b);
    b.pos = bones.back().pos;
    b.length = bones.back().pos.length();
    bones.push_back(b);
    
    for(unsigned i = 0; i < 5; ++i)
    {
        FABRIK_impl(bones, target);
    }
    
    for(int i = (int)bones.size() - 2; i >= 1; --i)
    {
        ChainBone& b = bones[i];
        ChainBone& next = bones[i - 1];
        b.t->Track(next.pos, b.dir);
    }
}

void Transform::Position(float x, float y, float z)
{ Position(Au::Math::Vec3f(x, y, z)); }
void Transform::Position(const Au::Math::Vec3f& position)
{ _position = position; }
void Transform::Rotation(float x, float y, float z)
{ _rotation = EulerToQuat(Au::Math::Vec3f(x, y, z)); }
void Transform::Rotation(float x, float y, float z, float w)
{ Rotation(Au::Math::Quat(x, y, z, w)); }
void Transform::Rotation(const Au::Math::Quat& rotation)
{ _rotation = rotation; }

void Transform::Scale(float scale)
{ Scale(Au::Math::Vec3f(scale, scale, scale)); }
void Transform::Scale(float x, float y, float z)
{ Scale(Au::Math::Vec3f(x, y, z)); }
void Transform::Scale(const Au::Math::Vec3f& scale)
{ _scale = scale; }

Au::Math::Vec3f Transform::WorldPosition()
{
    return GetTransform()[3];
}
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
Au::Math::Vec3f Transform::Left()
{ return -Right(); }
Au::Math::Vec3f Transform::Down()
{ return -Up(); }
Au::Math::Vec3f Transform::Forward()
{ return -Back(); }

void Transform::SetTransform(Au::Math::Mat4f& t)
{
    _position = Au::Math::Vec3f(t[3].x, t[3].y, t[3].z);
    Au::Math::Mat3f rotMat = Au::Math::ToOrientationMat3(t);
    _rotation = Au::Math::ToQuat(rotMat);
    Au::Math::Vec3f right = t[0];
    Au::Math::Vec3f up = t[1];
    Au::Math::Vec3f back = t[2];
    _scale = Au::Math::Vec3f(right.length(), up.length(), back.length());
}

Au::Math::Mat4f Transform::GetLocalTransform()
{
    return 
        Au::Math::Translate(Au::Math::Mat4f(1.0f), _position) * 
        Au::Math::ToMat4(_rotation) * 
        Au::Math::Scale(Au::Math::Mat4f(1.0f), _scale);
}

Au::Math::Mat4f Transform::GetTransform()
{
    Au::Math::Mat4f localTransform = GetLocalTransform();
            
    if(_parent)
        _transform = _parent->GetTransform() * localTransform;
    else
        _transform = localTransform;
    
    return _transform;
}

void Transform::OnCreate()
{
    //GetObject()->GetComponent<DebugTransformIcon>();
}
