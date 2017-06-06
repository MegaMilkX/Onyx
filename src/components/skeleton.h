#ifndef SKELETON_H
#define SKELETON_H

#include "renderer.h"
#include "animation.h"
#include "mesh.h"

struct BoneData
{
    BoneData(const std::string& name, Au::Math::Mat4f transform, const std::string& parentName)
    : name(name), transform(transform), parentName(parentName)
    {}
    ~BoneData()
    { for(unsigned i = 0; i < children.size(); ++i) delete children[i]; }
    
    std::string name;
    std::string parentName;
    Au::Math::Mat4f transform;
    std::vector<BoneData*> children;
};

struct SkeletonData
{
    SkeletonData()
    : boneCount(0)
    {}
    ~SkeletonData()
    { for(unsigned i = 0; i < rootBones.size(); ++i) delete rootBones[i]; }
    void AddBone(const std::string& name, Au::Math::Mat4f transform, const std::string& parentName)
    {
        BoneData* bd = new BoneData(name, transform, parentName);
        rootBones.push_back(bd);
        boneCount++;
    }
    void Finalize()
    {
        std::vector<BoneData*>& bones = 
            this->rootBones;
        std::vector<BoneData*> rootBones;
        std::vector<unsigned> rootBoneIndices;
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            BoneData* parent = GetBoneData(bones[i]->parentName);
            if(parent)
                parent->children.push_back(bones[i]);
            else
                rootBoneIndices.push_back(i);
        }
        
        for(unsigned i = 0; i < rootBoneIndices.size(); ++i)
            rootBones.push_back(bones[rootBoneIndices[i]]);
        bones = rootBones;
    }
    int BoneCount() { return boneCount; }
    BoneData* GetBoneData(const std::string& name)
    {
        std::vector<BoneData*>& bones = 
            this->rootBones;
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            if(bones[i]->name == name)
                return bones[i];
        }
        return 0;
    }
    std::vector<BoneData*> rootBones;
    int boneCount;
};

struct SkeletonDataReaderFBX : public Resource<SkeletonData>::Reader
{
    SkeletonData* operator()(const std::string& filename)
    {
        SkeletonData* skel = 0;
        
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if(!file.is_open())
            return 0;
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
                Au::Media::FBX::Bone* pb = GetBoneByUID(bones, bones[i].puid);
                std::string parentName = pb ? pb->name : "";
                skel->AddBone(bones[i].name, bones[i].transform, parentName);
            }
            skel->Finalize();
            
            //fbxReader.GetHelpers();
        }
        
        file.close();
        
        return skel;
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
    Skeleton()
    {
        skinShaderSource =
            R"(
                in vec3 SkinWorld;
                gl_Position = SkinWorld;
            )";
            
        uniformBoneInverseBinds = Au::GFX::GetUniform<Au::Math::Mat4f>("BoneInverseBindTransforms");
        uniformBoneTransforms = Au::GFX::GetUniform<Au::Math::Mat4f>("BoneTransforms");
    }
    
    ~Skeleton()
    {
        renderer->RemoveSkeleton(this);
    }
    
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
        
        SortAndFinalizeBoneArrays();
        
        int maxBoneCount = renderer->GetInt("MAX_BONE_COUNT");
        if((int)bones.size() > maxBoneCount)
            renderer->SetInt("MAX_BONE_COUNT", bones.size());
    }
    
    void Bind()
    {
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            uniformBoneInverseBinds[i] = boneInverseBindTransforms[i];
            uniformBoneTransforms[i] = boneTransforms[i];
        }
    }
    
    void Update()
    {
        for(unsigned i = 0; i < bones.size(); ++i)
            boneTransforms[i] = 
                Au::Math::Inverse(GetObject()->GetComponent<Transform>()->GetTransform()) *
                bones[i]->GetTransform();
    }

    virtual void OnCreate()
    {
        GetObject()->
            GetComponent<Mesh>()->
                VertexShaderSource(skinShaderSource);
        
        renderer = GetObject()->Root()->GetComponent<Renderer>();
        renderer->AddSkeleton(this);
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
        bones.push_back(boneTransform);
        
        boneTransform->SetTransform(bone->transform);
        boneTransform->AttachTo(parentObject->GetComponent<Transform>());
        
        for(unsigned i = 0; i < bone->children.size(); ++i)
        {
            BoneData* boneData = bone->children[i];
            CreateBone(boneData, boneObject);
        }
    }
    
    static bool CompBoneTransforms(Transform*& first, Transform*& second)
    {
        return first->GetObject()->Name() < second->GetObject()->Name();
    }
    
    void SortAndFinalizeBoneArrays()
    {
        std::sort(bones.begin(), bones.end(), &CompBoneTransforms);
        boneInverseBindTransforms.resize(bones.size());
        boneTransforms.resize(bones.size());
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            boneInverseBindTransforms[i] = 
                Au::Math::Inverse(
                    Au::Math::Inverse(GetObject()->GetComponent<Transform>()->GetTransform()) *
                    bones[i]->GetTransform()
                );
            boneTransforms[i] = 
                Au::Math::Inverse(GetObject()->GetComponent<Transform>()->GetTransform()) *
                bones[i]->GetTransform();
        }
        
        uniformBoneInverseBinds = 
            Au::GFX::GetUniform<Au::Math::Mat4f>("BoneInverseBindTransforms", bones.size());
        uniformBoneTransforms = 
            Au::GFX::GetUniform<Au::Math::Mat4f>("BoneTransforms", bones.size());
    }

    SkeletonData* skelData;
    
    Renderer* renderer;
    
    std::vector<Transform*> bones;
    std::vector<Au::Math::Mat4f> boneInverseBindTransforms;
    std::vector<Au::Math::Mat4f> boneTransforms;
    
    Au::GFX::Uniform uniformBoneInverseBinds;
    Au::GFX::Uniform uniformBoneTransforms;
    
    std::string skinShaderSource;
};

#endif
