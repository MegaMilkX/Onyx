#ifndef ANIMATION_H
#define ANIMATION_H

#include <fstream>

#include <aurora/media/fbx.h>
#include <aurora/curve.h>

#include "../scene_object.h"
#include "transform.h"

struct AnimData
{
    unsigned ChildCount() { return children.size(); }
    AnimData& GetChild(const std::string& name)
    { 
        AnimData& data = children[name];
        data.Name(name);
        return data;
    }
    AnimData& GetChild(unsigned i)
    {
        std::map<std::string, AnimData>::iterator it =
            children.begin();
        for(unsigned j = 0; j < i; ++j)
            ++it;
        return it->second;
    }
    unsigned AnimCount() { return anims.size(); }
    Au::Curve& GetAnim(const std::string& name)
    { return anims[name]; }
    Au::Curve& GetAnim(unsigned i)
    {
        std::map<std::string, Au::Curve>::iterator it =
            anims.begin();
        for(unsigned j = 0; j < i; ++j)
            ++it;
        return it->second;
    }
    void FrameRate(float fps) { this->fps = fps; }
    float FrameRate() { return fps; }
    
    std::string& Name() { return name; }
    void Name(const std::string& name) { this->name = name; }
    
private:
    std::string name;
    std::map<std::string, AnimData> children;
    std::map<std::string, Au::Curve> anims;
    float fps;
};

struct AnimDataReaderFBX : public Resource<AnimData>::Reader
{
    AnimData* operator()(const std::string& filename)
    {
        AnimData* animData = 0;
        
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if(!file.is_open())
            return 0;
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer((unsigned int)size);
        if(file.read(buffer.data(), size))
        {
            animData = new AnimData();
            
            Au::Media::FBX::Reader fbxReader;
            fbxReader.ReadMemory(buffer.data(), buffer.size());
            fbxReader.DumpFile(filename);
            fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
            
            std::vector<Au::Media::FBX::AnimationStack> stacks =
                fbxReader.GetAnimationStacks();
            double fps = fbxReader.GetFrameRate();
            
            double timePerFrame = Au::Media::FBX::TimeSecond / fps;
            animData->FrameRate((float)fps);
            for(unsigned i = 0; i < stacks.size(); ++i)
            {
                std::string stackName = stacks[i].GetName();
                double length = stacks[i].GetLength() / timePerFrame;
                if(length < 2.0)
                    continue;
                
                std::cout << "AnimStack " << stackName << " len: " << length << std::endl;
                
                std::vector<Au::Media::FBX::SceneNode> nodes = stacks[i].GetAnimatedNodes();
                for(unsigned j = 0; j < nodes.size(); ++j)
                {
                    std::string nodeName = nodes[j].Name();
                    Au::Curve& anim = animData->GetChild(nodeName).GetAnim(stackName);
                    float frame = 0.0f;
                    anim.Length((float)length);
                    
                    std::cout << "  CurveNode " << nodeName << std::endl;
                    
                    for(double t = 0.0f; t < length * timePerFrame; t += timePerFrame)
                    {
                        Au::Math::Vec3f pos = 
                            stacks[i].EvaluatePosition(nodes[j], (int64_t)t);
                        anim["Position"]["x"][frame] = pos.x;
                        anim["Position"]["y"][frame] = pos.y;
                        anim["Position"]["z"][frame] = pos.z;
                        
                        Au::Math::Quat rot = 
                            stacks[i].EvaluateRotation(nodes[j], (int64_t)t);
                        anim["Rotation"]["x"][frame] = rot.x;
                        anim["Rotation"]["y"][frame] = rot.y;
                        anim["Rotation"]["z"][frame] = rot.z;
                        anim["Rotation"]["w"][frame] = rot.w;
                        
                        Au::Math::Vec3f scale = 
                            stacks[i].EvaluateScale(nodes[j], (int64_t)t);
                        anim["Scale"]["x"][frame] = scale.x;
                        anim["Scale"]["y"][frame] = scale.y;
                        anim["Scale"]["z"][frame] = scale.z;
                        
                        frame += 1.0f;
                    }
                }
            }
        }
        
        file.close();
        
        return animData;
    }

private:
    void FillPropCurve(Au::Curve& curve, Au::Media::FBX::AnimationCurve* fbxCurve, double timePerFrame)
    {
        std::string elemName = fbxCurve->GetName();
        if(elemName == "d|X")
            elemName = "x";
        else if(elemName == "d|Y")
            elemName = "y";
        else if(elemName == "d|Z")
            elemName = "z";
        
        unsigned keyCount = fbxCurve->KeyframeCount();
        for(unsigned l = 0; l < keyCount; ++l)
        {
            Au::Media::FBX::Keyframe* kf =
                fbxCurve->GetKeyframe(l);
            float frame = (float)(kf->frame / timePerFrame);
            float value = kf->value;
            
            curve[elemName][frame] = value;
        }
    }
};

class AnimTrack
{
public:
    enum ANIM_PROPS
    {
        NONE     = 0,
        POSITION = 1,
        ROTATION = 2,
        SCALE    = 4
    };
    
    AnimTrack()
    : cursor(0.0f), fps(0.0f), props(NONE)
    {}
    AnimTrack(Au::Curve& anim)
    : cursor(0.0f), fps(0.0f), props(NONE)
    {
        this->anim = anim;
        unsigned propCurveCount = anim.CurveCount();
        for(unsigned i = 0; i < propCurveCount; ++i)
        {
            std::string propName = anim.GetCurveName(i);
            Au::Curve* propCurve = anim.GetCurve(i);
            
            if(propName == "Position")
            {
                props = ANIM_PROPS((int)props | (int)POSITION);
            }
            else if(propName == "Rotation")
            {
                props = ANIM_PROPS((int)props | (int)ROTATION);
            }
            else if(propName == "Scale")
            {
                props = ANIM_PROPS((int)props | (int)SCALE);
            }
        }
    }
    
    void ResetCursor() { cursor = 0.0f; }
    
    Au::Math::Vec3f& GetPosition(Au::Math::Vec3f& defaultValue)
    { 
        if(props & POSITION)
        {
            Au::Curve& px = anim["Position"]["x"];
            Au::Curve& py = anim["Position"]["y"];
            Au::Curve& pz = anim["Position"]["z"];
            position = Au::Math::Vec3f(px.value, py.value, pz.value);
            return position;
        }
        else
        {
            return defaultValue;
        }
    }
    Au::Math::Quat& GetRotation(Au::Math::Quat& defaultValue) 
    { 
        if(props & ROTATION)
        {
            Au::Curve& rx = anim["Rotation"]["x"];
            Au::Curve& ry = anim["Rotation"]["y"];
            Au::Curve& rz = anim["Rotation"]["z"];
            Au::Curve& rw = anim["Rotation"]["w"];
            rotation = Au::Math::Normalize(
                Au::Math::Quat(
                    rx.value,
                    ry.value,
                    rz.value,
                    rw.value
                )
            );
            return rotation;
        }
        else
        {
            return defaultValue;
        }
    }
    Au::Math::Vec3f& GetScale(Au::Math::Vec3f& defaultValue) 
    { 
        if(props & SCALE)
        {
            Au::Curve& sx = anim["Scale"]["x"];
            Au::Curve& sy = anim["Scale"]["y"];
            Au::Curve& sz = anim["Scale"]["z"];
            scale = Au::Math::Vec3f(sx.value, sy.value, sz.value);
            return scale;
        }
        else
        {
            return defaultValue;
        }
    }
    
    void Tick(float dt)
    {
        cursor += dt;
        int loopCount = (int)(cursor / anim.Length());
        float overflow = loopCount * anim.Length();
        cursor -= overflow;
        anim.Evaluate(cursor);
    }
    
private:
    float cursor;
    float fps;
    Au::Curve anim;
    ANIM_PROPS props;
    
    Au::Math::Vec3f position;
    Au::Math::Quat rotation;
    Au::Math::Vec3f scale;
};

class Animation : public SceneObject::Component
{
public:
    Animation()
    : fps(0.0f), blend(0.0f), blendStep(0.0f) {}
    /*
    void SetAnimData(const std::string& name)
    {
        SetAnimData(Resource<AnimData>::Get(name));
    }
    
    void SetAnimData(AnimData* data)
    {
        animData = data;
        FrameRate(animData->FrameRate());
        for(unsigned i = 0; i < animData->ChildCount(); ++i)
        {
            AnimData& childData = animData->GetChild(i);
            SceneObject* o = GetObject()->FindObject(childData.Name());
            if(!o)
                continue;
            for(unsigned j = 0; j < childData.AnimCount(); ++j)
            {
                Au::Curve& anim = childData.GetAnim(j);
                o->GetComponent<Animation>()->SetAnim(anim.Name(), anim);
                o->GetComponent<Animation>()->FrameRate(animData->FrameRate());
            }
        }
    }
    */
    void SetAnim(const std::string& name, const std::string& resourceName)
    {
        animResourceName = resourceName;
        SetAnim(name, Resource<AnimData>::Get(resourceName));
    }
    
    void SetAnim(const std::string& name, AnimData* data)
    {
        animName = name;
        FrameRate(data->FrameRate());
        for(unsigned i = 0; i < data->ChildCount(); ++i)
        {
            AnimData& childData = data->GetChild(i);
            SceneObject* o = GetObject()->FindObject(childData.Name());
            if(!o)
                continue;
            
            Au::Curve& anim = childData.GetAnim(0);
            o->GetComponent<Animation>()->SetAnim(name, anim);
            o->GetComponent<Animation>()->FrameRate(data->FrameRate());
            child_anims.push_back(o->GetComponent<Animation>());
        }
    }
    
    void SetAnim(const std::string& name, Au::Curve& curve)
    {
        anims[name] = AnimTrack(curve);
        Play(name);
    }
    
    void FrameRate(float fps) { this->fps = fps; }
    void Play(const std::string& name)
    { 
        anim = anims[name];
        anim.ResetCursor();
        for(unsigned i = 0; i < child_anims.size(); ++i)
        {
            child_anims[i]->Play(name);
        }
    }
    void BlendOverTime(const std::string& to, float t)
    {
        blend = 0.0f;
        blendStep = 1.0f/t;
        animBlendTarget = anims[to];
        animBlendTarget.ResetCursor();
        for(unsigned i = 0; i < child_anims.size(); ++i)
        {
            child_anims[i]->BlendOverTime(to, t);
        }
    }
    
    void Tick(float dt)
    {
        blend += blendStep * dt;
        anim.Tick(dt * fps);
        animBlendTarget.Tick(dt * fps);
        if(blend >= 1.0f)
        {
            blendStep = 0.0f;
            anim = animBlendTarget;
        }

        Transform* t = GetObject()->GetComponent<Transform>();
        
        Au::Math::Vec3f pos0 = anim.GetPosition(t->Position());
        Au::Math::Quat rot0 = anim.GetRotation(t->Rotation());
        Au::Math::Vec3f scl0 = anim.GetScale(t->Scale());
        Au::Math::Vec3f pos1 = animBlendTarget.GetPosition(t->Position());
        Au::Math::Quat rot1 = animBlendTarget.GetRotation(t->Rotation());
        Au::Math::Vec3f scl1 = animBlendTarget.GetScale(t->Scale());
        
        t->Position(Au::Math::Lerp(pos0, pos1, blend));
        t->Rotation(Au::Math::Slerp(rot0, rot1, blend));
        t->Scale(Au::Math::Lerp(scl0, scl1, blend));
    }
    
    void Update(float time)
    {        
        for(unsigned i = 0; i < children.size(); ++i)
        {
            children[i]->Tick(time);
        }
    }
    
    ~Animation()
    {
        GetObject()->Root()->GetComponent<Animation>()->_removeChild(this);
    }
    
    virtual void OnCreate()
    {
        if(GetObject()->IsRoot())
        {
            return;
        }
        anim = anims[""];
        animBlendTarget = anims[""];
        GetObject()->Root()->GetComponent<Animation>()->_addChild(this);
    }
    virtual std::string Serialize() 
    { 
        using json = nlohmann::json;
        json j = json::object();
        j["Anim"] = animName;
        j["AnimData"] = animResourceName;
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Anim"].is_string())
        {
            animName = j["Anim"].get<std::string>();
        }
        if(j["AnimData"].is_string())
        {
            animResourceName = j["AnimData"].get<std::string>();
        }
        if(!animResourceName.empty())
        {
            SetAnim(animName, animResourceName);
        }
    }
private:
    void _addChild(Animation* anim)
    {
        _removeChild(anim);
        children.push_back(anim);
    }
    
    void _removeChild(Animation* anim)
    {
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i] == anim)
            {
                children.erase(children.begin() + i);
                break;
            }
        }
    }
    
    std::string animResourceName;
    std::string animName;
    
    float fps;
    float blend;
    float blendStep;
    AnimTrack animBlendTarget;
    AnimTrack anim;
    std::map<std::string, AnimTrack> anims;
    
    std::vector<Animation*> child_anims;
    
    //-- Root anim only
    std::vector<Animation*> children;
};

#endif
