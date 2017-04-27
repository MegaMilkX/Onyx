#ifndef ANIMATION_H
#define ANIMATION_H

#include <fstream>

#include <aurora/media/fbx.h>

#include "../scene_object.h"

class AnimData
{
    
};

struct AnimDataReaderFBX : public Resource<AnimData>::Reader
{
    AnimData* operator()(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer((unsigned int)size);
        if(file.read(buffer.data(), size))
        {            
            Au::Media::FBX::Reader fbxReader;
            fbxReader.ReadFile(buffer.data(), buffer.size());
            fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
            
            
        }
        
        return 0;
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
        
    }
    
    virtual void OnCreate()
    {
        
    }
};

#endif
