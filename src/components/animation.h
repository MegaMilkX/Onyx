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
            fbxReader.ReadFile(buffer.data(), buffer.size());
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
                
                int layerCount = stacks[i].LayerCount();
                if(layerCount > 0)
                {
                    Au::Media::FBX::AnimationLayer* layer =
                        stacks[i].GetLayer(0);
                    
                    for(unsigned j = 0; j < layer->CurveNodeCount(); ++j)
                    {
                        Au::Media::FBX::AnimationCurveNode* curveNode = 
                            layer->GetCurveNode(j);
                        
                        std::string objectName = curveNode->GetObjectName();
                        animData->GetChild(objectName).GetAnim(stackName).Length((float)length);
                        std::string propName = "";
                        if(curveNode->GetPropertyName() == "Lcl Translation")
                            propName = "Position";
                        else if(curveNode->GetPropertyName() == "Lcl Rotation")
                            propName = "Rotation";
                        else if(curveNode->GetPropertyName() == "Lcl Scaling")
                            propName = "Scale";
                        
                        Au::Curve& propCurve = animData->GetChild(objectName).GetAnim(stackName)[propName];
                        
                        unsigned curveCount = curveNode->CurveCount();
                        for(unsigned k = 0; k < curveCount; ++k)
                        {
                            Au::Media::FBX::AnimationCurve* fbxCurve = 
                                curveNode->GetCurve(k);
                                
                            FillPropCurve(propCurve, fbxCurve, timePerFrame);
                        }
                    }
                }
            }
        }
        
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

class Animation : public SceneObject::Component
{
public:
    enum ANIM_PROPS
    {
        NONE     = 0,
        POSITION = 1,
        ROTATION = 2,
        SCALE    = 4
    };
    
    Animation() : cursor(0.0f), props(NONE), fps(0.0f) {}

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
    
    void SetAnim(const std::string& name, Au::Curve& curve)
    {
        anims[name] = curve;
        unsigned propCurveCount = curve.CurveCount();
        for(unsigned i = 0; i < propCurveCount; ++i)
        {
            std::string propName = anims[name].GetCurveName(i);
            Au::Curve* propCurve = anims[name].GetCurve(i);
            
            Transform* transform = 
                GetObject()->GetComponent<Transform>();
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
        Play(name);
    }
    
    void FrameRate(float fps) { this->fps = fps; }
    void Play(const std::string& name)
    { 
        anim = anims[name];
        cursor = 0.0f;
    }
    
    void Tick(float time)
    {
        cursor += time * fps;
        int loopCount = (int)(cursor / anim.Length());
        float overflow = loopCount * anim.Length();
        cursor -= overflow;
        
        anim.Evaluate(cursor);

        Transform* t = GetObject()->GetComponent<Transform>();
        if(props & POSITION)
        {
            Au::Curve& px = anim["Position"]["x"];
            Au::Curve& py = anim["Position"]["y"];
            Au::Curve& pz = anim["Position"]["z"];
            t->Position(px.value, py.value, pz.value);
        }
        if(props & ROTATION)
        {
            Au::Curve& rx = anim["Rotation"]["x"];
            Au::Curve& ry = anim["Rotation"]["y"];
            Au::Curve& rz = anim["Rotation"]["z"];
            t->Rotation(rx.value, ry.value, rz.value);
        }
        if(props & SCALE)
        {
            Au::Curve& sx = anim["Scale"]["x"];
            Au::Curve& sy = anim["Scale"]["y"];
            Au::Curve& sz = anim["Scale"]["z"];
            t->Scale(sx.value, sy.value, sz.value);
        }
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
        GetObject()->Root()->GetComponent<Animation>()->_addChild(this);
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
    
    ANIM_PROPS props;

    AnimData* animData;
    float cursor;
    float fps;
    Au::Curve anim;
    std::map<std::string, Au::Curve> anims;
    
    std::vector<Animation*> children;
};

#endif
