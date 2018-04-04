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

class AnimNodeController
{
public:
    AnimNodeController() {}
    AnimNodeController(Transform* t, const AnimNode& node)
    : transform(t), animNode(node) {}
    Transform* transform;
    AnimNode animNode;
};

class AnimController
{
public:
    AnimController() {}
    AnimController(Transform* root)
    : root(root) {}
    void AddNode(AnimNode& node)
    {
        SceneObject* o = root->Object()->FindObject(node.name);
        if(!o) return;
        nodes.push_back(AnimNodeController(o->Get<Transform>(), node));
    }
    
    void Tick(float dt)
    {
        for(auto& con : nodes)
        {
            std::cout << con.animNode.name << std::endl;
        }
    }

    Transform* root;
    std::vector<AnimNodeController> nodes;
};

class AnimBlendController
{
public:

};

class AnimNodePersist
{
public:
private:
    AnimNode node;
    float cursor;
    float length;
};

class AnimLayer
{
public:
    void Tick(float dt)
    {

    }
    void Apply(gfxm::transform& t)
    {

    }
private:
    AnimNodePersist nodeFrom;
    AnimNodePersist nodeTo;
    float blendWeight;
    float blendStep;
};

class Animator : public SceneObject::Component
{
public:
    Animator()
    {}

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
        AnimController con(Get<Transform>());
        for(auto& kv : anim.GetNodes())
        {
            con.AddNode(kv.second);
        }
        anims[name] = con;
        Play(name);
    }
    
    void FrameRate(float fps) { }
    void Play(const std::string& name)
    { 
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

    std::map<std::string, AnimController> anims;    
    //-- Root anim only
    std::set<Animator*> children;
};

#endif
