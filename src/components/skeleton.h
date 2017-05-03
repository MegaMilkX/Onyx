#ifndef SKELETON_H
#define SKELETON_H

#include "animation.h"

struct BoneData
{
    ~BoneData()
    { for(unsigned i = 0; i < children.size(); ++i) delete children[i]; }
    
    std::string name;
    std::string parentName;
    Au::Math::Mat4f transform;
    std::vector<BoneData*> children;
};

struct SkeletonData
{
    ~SkeletonData()
    { for(unsigned i = 0; i < rootBones.size(); ++i) delete rootBones[i]; }
    std::vector<BoneData*> rootBones;
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
            
            std::vector<Au::Media::FBX::Bone> bones = 
                fbxReader.GetBones();
            
            for(unsigned i = 0; i < bones.size(); ++i)
            {
                BoneData* bd = new BoneData();
                bd->name = bones[i].name;
                bd->transform = bones[i].transform;
                Au::Media::FBX::Bone* parent = GetBoneByUID(bones, bones[i].puid);
                if(parent) bd->parentName = parent->name;
                skel->rootBones.push_back(bd);
            }
            
            MakeBoneTree(skel);
            
            //fbxReader.GetHelpers();
        }
        
        return skel;
    }
    
    void MakeBoneTree(SkeletonData* skel)
    {
        std::vector<BoneData*>& bones = 
            skel->rootBones;
        std::vector<BoneData*> rootBones;
        std::vector<unsigned> rootBoneIndices;
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            BoneData* parent = GetBoneData(skel, bones[i]->parentName);
            if(parent)
                parent->children.push_back(bones[i]);
            else
                rootBoneIndices.push_back(i);
        }
        
        for(unsigned i = 0; i < rootBoneIndices.size(); ++i)
            rootBones.push_back(bones[rootBoneIndices[i]]);
        bones = rootBones;
    }
    
    BoneData* GetBoneData(SkeletonData* skel, const std::string& name)
    {
        std::vector<BoneData*>& bones = 
            skel->rootBones;
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            if(bones[i]->name == name)
                return bones[i];
        }
        return 0;
    }
    
    Au::Media::FBX::Bone* GetBoneByUID(std::vector<Au::Media::FBX::Bone>& bones, int64_t uid)
    {
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            if(bones[i].uid == uid)
                return &bones[i];
        }
        return 0;
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
        for(unsigned i = 0; i < skelData->rootBones.size(); ++i)
        {
            BoneData* bone = skelData->rootBones[i];
            CreateBone(bone, GetObject());
        }
    }

    virtual void OnCreate()
    {
        
    }
private:
    void CreateBone(BoneData* bone, SceneObject* parentObject)
    {
        SceneObject* boneObject = parentObject->FindObject(bone->name);
        if(!boneObject)
        {
            boneObject = parentObject->CreateObject();
            boneObject->Name(bone->name);
        }
        
        Transform* boneTransform = boneObject->GetComponent<Transform>();
        boneTransform->SetTransform(bone->transform);
        boneTransform->AttachTo(parentObject->GetComponent<Transform>());
        
        for(unsigned i = 0; i < bone->children.size(); ++i)
        {
            BoneData* boneData = bone->children[i];
            CreateBone(boneData, boneObject);
        }
    }

    SkeletonData* skelData;
};

#endif
