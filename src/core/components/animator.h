#ifndef ANIMATOR_H
#define ANIMATOR_H



#include <fstream>

#include <aurora/media/fbx.h>
#include <aurora/curve.h>

#include "../scene_object.h"
#include "transform.h"
#include <game_state.h>

#include <animation.h>

class AnimLayer
{
public:
    virtual void Tick(float dt) = 0;
    virtual AnimPose* GetPose(AnimPose* current) = 0;
    virtual typeindex Type() = 0;
};

class AnimLayerPlain : public AnimLayer
{
public:
    void Tick(float dt)
    {
        cursor += dt;
    }
    AnimPose* GetPose(AnimPose* current) { return &(*current = *cursor.GetPose()); }
    void SetTrack(AnimTrack* t)
    {
        if(!t) return;
        cursor = t->GetCursor();
    }

    typeindex Type() { return TypeInfo<AnimLayerPlain>::Index(); }
private:
    AnimTrack::Cursor cursor;
};

class AnimLayerAdd : public AnimLayer
{
public:
    void Tick(float dt)
    {
        cursor += dt;
    }
    AnimPose* GetPose(AnimPose* current)
    {
        AnimPose* pose = cursor.GetPose();
        for(auto& kv : pose->poses)
        {
            gfxm::mat4 bp = bindPose->poses[kv.first];
            bp = gfxm::inverse(bp);
            gfxm::mat4 deltaPose = bp * kv.second;
            current->poses[kv.first] = deltaPose * current->poses[kv.first];
        }
        return current;
    }
    void SetBindPose(AnimPose* bind)
    {
        bindPose = bind;
    }
    void SetTrack(AnimTrack* t)
    {
        if(!t) return;
        cursor = t->GetCursor();
    }

    typeindex Type() { return TypeInfo<AnimLayerAdd>::Index(); }
private:
    AnimTrack::Cursor cursor;
    AnimPose* bindPose;
};

class Animator : public SceneObject::Component
{
public:
    Animator()
    {
        SetLayerCount(1);
    }

    void Set(const std::string& resource)
    {
        Set(asset<Animation>::get(resource));
    }
    void Set(asset<Animation> data)
    {
        if(data.empty())
            return;

        animation = data;

        //FrameRate(data->FrameRate());
        for(auto& kv : data->GetTracks())
        {
            SetAnim(kv.first, kv.second);
        }

        animNodes.clear();
        for(auto& kv : data->GetBindPose().poses)
        {
            SceneObject* o = Object()->FindObject(kv.first);
            if(o)
                animNodes[kv.first] = o->Get<Transform>();
        }
    }    
    std::map<std::string, Transform*> animNodes;
    void SetAnim(const std::string& name, AnimTrack* anim)
    {
        anims[name] = anim;
        Play(name);
    }

    template<typename T>
    T* GetLayer(int i)
    {
        AnimLayer* l = layers[i];
        if(!l)
        {
            layers[i] = new T();
            return (T*)layers[i];
        }
        else if (l->Type() == TypeInfo<T>::Index())
        {
            return (T*)l;
        }
        else
        {
            return 0;
        }
    }
    
    void FrameRate(float fps) { }
    AnimTrack* currentTrack;
    void Play(const std::string& name)
    {
        currentTrack = anims[name];
        SetLayerCount(2);
        GetLayer<AnimLayerPlain>(0)->SetTrack(anims[name]);
        GetLayer<AnimLayerAdd>(1)->SetBindPose(&animation->GetBindPose());
        GetLayer<AnimLayerAdd>(1)->SetTrack(anims["LayerMotion01"]);
    }
    void BlendOverTime(const std::string& to, float t)
    {
        Play(to);
    }

    void SetRootMotionTarget(SceneObject* root)
    {
        if(!root)
            return;
        rootMotionTarget = root->Get<Transform>();
    }
    Transform* rootMotionTarget;
    
    void Tick(float dt)
    {
        if(animation.empty())
            return;

        AnimPose bindPose = animation->GetBindPose();
        AnimPose* curPose = &bindPose;
        AnimPose* pose;
        for(auto l : layers)
        {
            l->Tick(dt * animation->FrameRate());
            pose = l->GetPose(curPose);
        }
        for(auto& kv : pose->poses)
        {
            Transform* t = animNodes[kv.first];
            if(t)
            {
                t->SetTransform(*(Au::Math::Mat4f*)&kv.second);
            }
        }
    }
    
    void Update(float time)
    {        
        for(auto c : children)
        {
            c->Tick(time);
        }
    }

    void SetLayerCount(size_t c) { layers.resize(c); }
    AnimLayer* GetLayer(size_t i) { return layers[i]; }
    
    ~Animator()
    {
        Object()->Root()->GetComponent<Animator>()->_removeChild(this);
    }
    
    virtual void OnCreate()
    {
        if(Object()->IsRoot())
        {
            return;
        }
        Object()->Root()->GetComponent<Animator>()->_addChild(this);
    }
private:
    void _addChild(Animator* anim)
    {
        children.insert(anim);
    }
    
    void _removeChild(Animator* anim)
    {
        children.erase(anim);
    }
    
    gfxm::transform transform;
    
    std::vector<AnimLayer*> layers;

    asset<Animation> animation;
    std::map<std::string, AnimTrack*> anims;    
    //-- Root anim only
    std::set<Animator*> children;
};

#endif
