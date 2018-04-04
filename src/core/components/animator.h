#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <util/gfxm.h>

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
    void Tick(float dt)
    {
        animFrom += dt * 60.0f;
    }
    void Apply(gfxm::transform& t)
    {
        AnimPose& pose = animFrom.GetPose();
        t.position(pose.position);
        t.rotation(pose.rotation);
        t.scale(pose.scale);
    }
    void Set(AnimTrack& track)
    {
        animFrom = track.GetCursor();
    }
private:
    AnimTrack::Cursor animFrom;
    AnimTrack::Cursor animTo;
    float blendWeight;
    float blendStep;
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
        //FrameRate(data->FrameRate());
        for(auto& kv : data->GetTracks())
        {
            SetAnim(kv.first, kv.second);
        }
    }    
    void SetAnim(const std::string& name, AnimTrack& anim)
    {
        anims[name] = anim;
        Play(name);
    }
    
    void FrameRate(float fps) { }
    void Play(const std::string& name)
    {
        layers[0].Set(anims[name]);
    }
    void BlendOverTime(const std::string& to, float t)
    {
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
        transform = GetBindTransform();
        for(auto& l : layers)
        {
            l.Tick(dt);
            l.Apply(transform);
        }
        // TODO: Apply transform
    }
    
    void Update(float time)
    {        
        for(auto c : children)
        {
            c->Tick(time);
        }
    }

    void SetLayerCount(size_t c) { layers.resize(c); }
    AnimLayer& GetLayer(size_t i) { return layers[i]; }
    
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
    
    std::vector<AnimLayer> layers;

    std::map<std::string, AnimTrack> anims;    
    //-- Root anim only
    std::set<Animator*> children;
};

#endif
