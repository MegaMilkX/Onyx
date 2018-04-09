#ifndef ANIMATOR_H
#define ANIMATOR_H



#include <fstream>

#include <aurora/media/fbx.h>
#include <aurora/curve.h>

#include "../scene_object.h"
#include "transform.h"
#include <game_state.h>

#include <animation.h>

class AnimJoint : public SceneObject::Component
{
public:
    void SetAnim(const std::string& name, AnimNode& anim)
    {
        anims[name] = anim;
    }

    void OnCreate()
    {
        t = Get<Transform>();
    }
private:
    Transform* t;
    std::map<std::string, AnimNode> anims;
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

        animation = data;
        for(auto& track_kv : data->GetTracks())
        {
            for(auto& node_kv : track_kv.second->GetNodes())
            {
                AnimJoint* j = GetJoint(node_kv.first);
                if(!j) continue;
                j->SetAnim(track_kv.first, node_kv.second);
            }
        }
        
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
    Transform* rootMotionSource = 0;
    void SetAnim(const std::string& name, AnimTrack* anim)
    {
        anims[name] = anim;
        rootMotionSource = 0;
        if(!anim->GetRootMotionNode().name.empty())
        {
            SceneObject* o = Object()->FindObject(anim->GetRootMotionNode().name);
            if(o) rootMotionSource = o->Get<Transform>();
        }
        Play(name);
    }

    std::map<std::string, AnimJoint*> joints;
    AnimJoint* GetJoint(const std::string& name)
    {
        SceneObject* o = Object()->FindObject(name);
        if(!o) return 0;
        return o->Get<AnimJoint>();
    }
    
    void FrameRate(float fps) { }
    AnimTrack::Cursor currentCursor;
    AnimTrack::Cursor targetCursor;
    float blend;
    float blendStep;
    void Play(const std::string& name)
    {
        currentCursor = anims[name]->GetCursor();
    }
    void BlendOverTime(const std::string& to, float t)
    {
        if(blend != 0.0f) currentCursor = targetCursor;
        targetCursor = anims[to]->GetCursor();
        blend = 0.0f;
        blendStep = 1.0f/t;
    }
    int Stopped(float secondsBeforeEnd = 0.0f) 
    {
        return currentCursor.EndReached(secondsBeforeEnd);
    }

    void ApplyAdd(AnimTrack::Cursor cur, float weight)
    {
        AnimPose bindPose = *cur.GetPoseAt(0, &animation->GetBindPose());
        AnimPose* pose = cur.GetPose(&animation->GetBindPose());
        for(auto& kv : pose->poses)
        {
            gfxm::transform bp = bindPose.poses[kv.first];
            gfxm::vec3 basePos = bp.position();
            gfxm::quat baseRot = bp.rotation();
            gfxm::vec3 baseScl = bp.scale();
            
            gfxm::transform& p = currentPose.poses[kv.first];
            gfxm::vec3 pos = kv.second.position();
            gfxm::quat rot = kv.second.rotation();
            gfxm::vec3 scl = kv.second.scale();
            pos = pos - basePos;
            rot = rot * gfxm::inverse(baseRot);
            scl = scl - baseScl;
            p.translate(gfxm::lerp(gfxm::vec3(0,0,0), pos, weight));
            p.rotate(gfxm::slerp(gfxm::quat(0,0,0,1), rot, weight));
            p.scale(p.scale() + gfxm::lerp(gfxm::vec3(0,0,0), scl, weight));
        }
        gfxm::vec3 posDelta = gfxm::lerp(
            gfxm::vec3(0,0,0), 
            cur.GetRootMotionDeltaPosition(),
            weight
        );
        gfxm::quat rotDelta = gfxm::slerp(
            gfxm::quat(0,0,0,1), 
            cur.GetRootMotionDeltaRotation(),
            weight
        );
        
        rootMotionPosDelta += *(Au::Math::Vec3f*)&posDelta;
        rootMotionRotDelta = *(Au::Math::Quat*)&rotDelta * rootMotionRotDelta;
    }
    void ApplyBlend(AnimTrack::Cursor cur, float weight)
    {
        AnimPose* pose = cur.GetPose(&animation->GetBindPose());

        for(auto& kv : pose->poses)
        {
            gfxm::transform& p = currentPose.poses[kv.first];

            gfxm::vec3 pos = kv.second.position();
            gfxm::quat rot = kv.second.rotation();
            gfxm::vec3 scl = kv.second.scale();
            p.position(gfxm::lerp(p.position(), pos, weight));
            p.rotation(gfxm::slerp(p.rotation(), rot, weight));
            p.scale(gfxm::lerp(p.scale(), scl, weight));
        }
        rootMotionPosDelta = 
            Au::Math::Lerp(
                Au::Math::Vec3f(0,0,0), 
                *(Au::Math::Vec3f*)&cur.GetRootMotionDeltaPosition(),
                weight
            );
        rootMotionRotDelta = 
            Au::Math::Slerp(
                Au::Math::Quat(0,0,0,1),
                 *(Au::Math::Quat*)&cur.GetRootMotionDeltaRotation(),
                 weight
            ); 
    }
    void ApplyPlain(AnimTrack::Cursor cur)
    {
        AnimPose* pose = cur.GetPose(&animation->GetBindPose());
        for(auto& kv : pose->poses)
        {
            gfxm::transform& p = currentPose.poses[kv.first];
            
            gfxm::vec3 pos = kv.second.position();
            gfxm::quat rot = kv.second.rotation();
            gfxm::vec3 scl = kv.second.scale();
            p.position(pos);
            p.rotation(rot);
            p.scale(scl);   
        }
        rootMotionPosDelta = *(Au::Math::Vec3f*)&cur.GetRootMotionDeltaPosition();
        rootMotionRotDelta = *(Au::Math::Quat*)&cur.GetRootMotionDeltaRotation();
    }
    Au::Math::Vec3f rootMotionPosDelta;
    Au::Math::Quat rootMotionRotDelta;

    AnimTrack::Cursor GetAnimCursor(const std::string& anim)
    {
        return anims[anim]->GetCursor();
    }
    
    void Tick(float dt)
    {
        this->dt = dt;
        if(animation.empty())
            return;
        currentPose = animation->GetBindPose();

        currentCursor += dt * animation->FrameRate();
        targetCursor += dt * animation->FrameRate();
        
        blend += blendStep * dt;
        if(blend >= 1.0f)
        {
            currentCursor = targetCursor;
            blend = 0.0f;
            blendStep = 0.0f;
        }

        ApplyPlain(currentCursor);
        if(blend > 0.0f && blend < 1.0f && targetCursor.Valid())
        {
            std::cout << blend << std::endl;
            ApplyBlend(targetCursor, blend);
        }
    }
    float dt;
    AnimPose currentPose;
    void Finalize()
    {
        

        for(auto& kv : currentPose.poses)
        {
            Transform* t = animNodes[kv.first];
            if(t)
            {
                gfxm::vec3 pos = kv.second.position();
                gfxm::quat rot = kv.second.rotation();
                gfxm::vec3 scl = kv.second.scale();
                t->Position(
                    *(Au::Math::Vec3f*)&pos
                );
                t->Rotation(
                    *(Au::Math::Quat*)&rot
                );
                t->Scale(
                    *(Au::Math::Vec3f*)&scl
                );
            }
        }

        if(rootMotionSource)
        {
            rootMotionSource->ToWorldDirection(rootMotionPosDelta);
            Get<Transform>()->Translate(rootMotionPosDelta);

            using quat = Au::Math::Quat;
            quat rp_world = rootMotionSource->GetParentRotation();
            quat rm = rootMotionRotDelta;
            quat q = rp_world * rm * Au::Math::Inverse(rp_world);
            Get<Transform>()->Rotate(q);
        }
    }
    
    void Update(float time)
    {        
        for(auto c : children)
        {
            c->Tick(time);
        }
    }
    
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

    asset<Animation> animation;
    std::map<std::string, AnimTrack*> anims;    
    //-- Root anim only
    std::set<Animator*> children;
};

#endif
