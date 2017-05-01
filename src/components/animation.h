#ifndef ANIMATION_H
#define ANIMATION_H

#include <fstream>

#include <aurora/media/fbx.h>
#include <aurora/curve.h>

#include "../scene_object.h"
#include "transform.h"

struct AnimData
{
    AnimData& GetChild(const std::string& name)
    { return children[name]; }
    Au::Curve& GetAnim(const std::string& name)
    { return anims[name]; }
    void FrameRate(float fps) { this->fps = fps; }
    float FrameRate() { return fps; }
    
private:
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
                                                
                        unsigned curveCount = curveNode->CurveCount();
                        for(unsigned k = 0; k < curveCount; ++k)
                        {
                            Au::Media::FBX::AnimationCurve* curve = 
                                curveNode->GetCurve(k);
                                
                            std::string curveName = curve->GetName();
                            if(curveName == "d|X")
                                curveName = "x";
                            else if(curveName == "d|Y")
                                curveName = "y";
                            else if(curveName == "d|Z")
                                curveName = "z";
                            
                            unsigned keyCount = curve->KeyframeCount();
                            for(unsigned l = 0; l < keyCount; ++l)
                            {
                                Au::Media::FBX::Keyframe* kf =
                                    curve->GetKeyframe(l);
                                float frame = (float)(kf->frame / timePerFrame);
                                float value = kf->value;
                                
                                animData->GetChild(objectName).GetAnim(stackName)
                                    [propName]
                                    [curveName]
                                    [frame] = value;
                            }
                        }
                    }
                }
            }
        }
        
        return animData;
    }
};

class Animation : public SceneObject::Component
{
public:
    void SetAnimData(const std::string& name)
    {
        SetAnimData(Resource<AnimData>::Get(name));
    }
    
    void SetAnimData(AnimData* data)
    {
        animData = data;
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

            }
            else if(propName == "Rotation")
            {

            }
            else if(propName == "Scale")
            {

            }
        }
        Play(name);
    }
    
    void FrameRate(float fps) { this->fps = fps; }
    void Play(const std::string& name){ anim = &anims[name]; }
    
    void Tick(float time)
    {        
        cursor += time * fps;
        int loopCount = (int)(cursor / anim->Length());
        float overflow = loopCount * anim->Length();
        cursor -= overflow;
        
        anim->Evaluate(cursor);
        
        Au::Curve& px = (*anim)["Position"]["x"];
        Au::Curve& py = (*anim)["Position"]["y"];
        Au::Curve& pz = (*anim)["Position"]["z"];
        Au::Curve& rx = (*anim)["Rotation"]["x"];
        Au::Curve& ry = (*anim)["Rotation"]["y"];
        Au::Curve& rz = (*anim)["Rotation"]["z"];
        Au::Curve& sx = (*anim)["Scale"]["x"];
        Au::Curve& sy = (*anim)["Scale"]["y"];
        Au::Curve& sz = (*anim)["Scale"]["z"];
        
        Transform* t = GetObject()->GetComponent<Transform>();
        t->Position(px.value, py.value, pz.value);
        t->Rotation(rx.value, ry.value, rz.value);
        t->Scale(sx.value, sy.value, sz.value);
    }
    
    void Update(float time)
    {
        for(unsigned i = 0; i < children.size(); ++i)
            children[i]->Tick(time);
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
        anim = &anims[""];
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

    AnimData* animData;
    float cursor;
    float fps;
    Au::Curve* anim;
    std::map<std::string, Au::Curve> anims;
    
    std::vector<Animation*> children;
};

#endif
