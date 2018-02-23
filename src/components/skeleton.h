#ifndef SKELETON_H
#define SKELETON_H

#include "renderer.h"

#include <algorithm>

#undef GetObject

struct BoneData
{
    BoneData(const std::string& name, Au::Math::Mat4f transform, const std::string& parentName, bool isBone)
    : name(name), transform(transform), parentName(parentName), isBone(isBone)
    {}
    ~BoneData()
    { for(unsigned i = 0; i < children.size(); ++i) delete children[i]; }
    
    std::string name;
    std::string parentName;
    bool isBone;
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
    void AddNode(const std::string& name, Au::Math::Mat4f transform, const std::string& parentName, bool isBone)
    {
        BoneData* bd = new BoneData(name, transform, parentName, isBone);
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
            fbxReader.ReadMemory(buffer.data(), buffer.size());
            fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
            
            for(unsigned i = 0; i < fbxReader.ModelCount(); ++i)
            {
                Au::Media::FBX::Model* fbxModel = 
                    fbxReader.GetModel(i);
                Au::Media::FBX::Model* fbxParentModel = 
                    fbxReader.GetModelByUID(fbxModel->parentUID);
                std::string parentName = fbxParentModel ? fbxParentModel->name : "";
                skel->AddNode(fbxModel->name, fbxModel->transform, parentName, fbxModel->IsBone());
            }
            
            skel->Finalize();
            
            /*
            std::vector<Au::Media::FBX::Bone> bones = 
                fbxReader.GetBones();
            
            for(unsigned i = 0; i < bones.size(); ++i)
            {
                Au::Media::FBX::Bone* pb = GetBoneByUID(bones, bones[i].puid);
                std::string parentName = pb ? pb->name : "";
                skel->AddBone(bones[i].name, bones[i].transform, parentName);
            }
            skel->Finalize();
            */
            //fbxReader.GetHelpers();
            //fbxReader.Print();
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
            #vertex MatrixSkin
                in vec4 BoneIndex4;
                in vec4 BoneWeight4;
                uniform mat4 BoneInverseBindTransforms[MAX_BONE_COUNT];
                uniform mat4 BoneTransforms[MAX_BONE_COUNT];
                out mat4 MatrixSkin;
                
                int bi0 = int(BoneIndex4.x);
                int bi1 = int(BoneIndex4.y);
                int bi2 = int(BoneIndex4.z);
                int bi3 = int(BoneIndex4.w);
                MatrixSkin = 
                    BoneTransforms[bi0] * BoneInverseBindTransforms[bi0] * BoneWeight4.x +
                    BoneTransforms[bi1] * BoneInverseBindTransforms[bi1] * BoneWeight4.y +
                    BoneTransforms[bi2] * BoneInverseBindTransforms[bi2] * BoneWeight4.z +
                    BoneTransforms[bi3] * BoneInverseBindTransforms[bi3] * BoneWeight4.w;
                
            #vertex PositionModel
                in vec3 Position;
                in mat4 MatrixSkin;
                out vec4 PositionModel;
                
                PositionModel =
                    MatrixSkin *
                    vec4(Position, 1.0);
                    
            #vertex NormalModel
                in vec3 Normal;
                in mat4 MatrixSkin;
                uniform mat4 MatrixModel;
                out vec3 NormalModel;
                NormalModel = normalize((MatrixModel * MatrixSkin * vec4(Normal, 0.0)).xyz);
            )";
            
        uniformBoneInverseBinds = Au::GFX::GetUniform<Au::Math::Mat4f>("BoneInverseBindTransforms");
        uniformBoneTransforms = Au::GFX::GetUniform<Au::Math::Mat4f>("BoneTransforms");
    }
    
    ~Skeleton()
    {
        
    }
    
    void SetData(const std::string& name)
    {
        resourceName = name;
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
        /*
        int maxBoneCount = renderer->GetInt("MAX_BONE_COUNT");
        if((int)bones.size() > maxBoneCount)
            renderer->SetInt("MAX_BONE_COUNT", bones.size());
        */
    }
    
    void Bind(GLuint shaderProgram)
    {/*
        for(unsigned i = 0; i < bones.size(); ++i)
        {
            uniformBoneInverseBinds[i] = boneInverseBindTransforms[i];
            uniformBoneTransforms[i] = boneTransforms[i];
        }
        */
        GLuint loc = glGetUniformLocation(shaderProgram, "BoneInverseBindTransforms[0]");
        glUniformMatrix4fv(
            loc, 
            (std::min)((unsigned)32, boneInverseBindTransforms.size()), 
            GL_FALSE, 
            (GLfloat*)boneInverseBindTransforms.data()
        );
        
        loc = glGetUniformLocation(shaderProgram, "BoneTransforms[0]");
        glUniformMatrix4fv(
            loc, 
            (std::min)((unsigned)32, boneTransforms.size()), 
            GL_FALSE, 
            (GLfloat*)boneTransforms.data()
        );
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
        renderer = GetObject()->Root()->GetComponent<Renderer>();
    }
    virtual std::string Serialize() 
    { 
        using json = nlohmann::json;
        json j = json::object();
        j["Data"] = resourceName;
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data)
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Data"].is_string())
        {
            SetData(j["Data"].get<std::string>());
        }
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
        
        std::cout << boneObject->Name() << std::endl;
        
        Transform* boneTransform = boneObject->GetComponent<Transform>();
        
        if(bone->isBone)
        {
            bones.push_back(boneTransform);
        }
        
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
        }
        
        Update();
        
        uniformBoneInverseBinds = 
            Au::GFX::GetUniform<Au::Math::Mat4f>("BoneInverseBindTransforms", bones.size());
        uniformBoneTransforms = 
            Au::GFX::GetUniform<Au::Math::Mat4f>("BoneTransforms", bones.size());
    }

    SkeletonData* skelData;
    std::string resourceName;
    
    Renderer* renderer;
    
    std::vector<Transform*> bones;
    std::vector<Au::Math::Mat4f> boneInverseBindTransforms;
    std::vector<Au::Math::Mat4f> boneTransforms;
    
    Au::GFX::Uniform uniformBoneInverseBinds;
    Au::GFX::Uniform uniformBoneTransforms;
    
    std::string skinShaderSource;
};

#endif
