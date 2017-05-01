#ifndef SKELETON_H
#define SKELETON_H

#include "animation.h"

struct SkeletonData
{
    
};

struct SkeletonDataReaderFBX : public Resource<SkeletonData>::Reader
{
    SkeletonData* operator()(const std::string& filename)
    {
        SkeletonData* skel = 0;
        
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer((unsigned int)size);
        if(file.read(buffer.data(), size))
        {
            skel = new SkeletonData();
            
            Au::Media::FBX::Reader fbxReader;
            fbxReader.ReadFile(buffer.data(), buffer.size());
            fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
            
            fbxReader.GetBones();
            //fbxReader.GetHelpers();
        }
        
        return skel;
    }
};

class Skeleton : public SceneObject::Component
{
public:
    void SetData(const std::string& name)
    {
        SetData(Resource<SkeletonData>::Get(name));
    }
    
    void SetData(SkeletonData* data)
    {
        skelData = data;
    }

    virtual void OnCreate()
    {
        
    }
private:
    SkeletonData* skelData;
};

#endif
