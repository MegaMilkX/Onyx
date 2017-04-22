#ifndef COMPONENT_MESH_H
#define COMPONENT_MESH_H

#include <fstream>

#include <aurora/media/fbx.h>

#include "../scene_object.h"
#include "transform.h"
#include "gfxscene.h"
#include "material.h"

#include "../resource.h"

class MeshData
{
public:
    void FillMesh(Au::GFX::Mesh* mesh)
    {
        std::vector<Au::AttribInfo> fmt = mesh->Format();
        
        for(unsigned i = 0; i < fmt.size(); ++i)
        {
            std::map<Au::AttribInfo, std::vector<unsigned char>>::iterator it =
                attribArrays.find(fmt[i]);
            if(it == attribArrays.end())
                continue;
            
            mesh->VertexAttribByInfo(fmt[i], it->second);
        }
        mesh->IndexData(indices);
    }
    
    template<typename ATTR, typename T>
    void SetAttribArray(const std::vector<T>& data)
    {
        std::vector<unsigned char> bytes(
            (unsigned char*)data.data(), 
            (unsigned char*)data.data() + data.size() * sizeof(T)
        );
        
        attribArrays[ATTR()] = bytes;
    }
    
    void SetIndices(const std::vector<unsigned short>& data)
    { indices = data; }
    
    std::map<Au::AttribInfo, std::vector<unsigned char>> attribArrays;
    std::vector<unsigned short> indices;
};

class MeshReaderFBX : public Resource<MeshData>::Reader
{
public:
    MeshData* operator()(const std::string& filename)
    {
        MeshData* meshData = 0;
        
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer((unsigned int)size);
        if(file.read(buffer.data(), size))
        {
            meshData = new MeshData();
            
            Au::Media::FBX::Reader fbxReader;
            fbxReader.ReadFile(buffer.data(), buffer.size());
            fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
            
            int meshCount = fbxReader.MeshCount();
            if(meshCount > 0)
            {                
                Au::Media::FBX::Mesh& fbxMesh = fbxReader.GetMesh(0);
                
                meshData->SetAttribArray<Au::Position>(fbxMesh.GetVertices());
                meshData->SetAttribArray<Au::Normal>(fbxMesh.GetNormals(0));
                meshData->SetIndices(fbxMesh.GetIndices<unsigned short>());
                
                Au::Media::FBX::Skin skin = fbxMesh.GetSkin();
                for(int i = 0; i < skin.DeformerCount(); ++i)
                {
                    Au::Media::FBX::SkinDeformer* deformer = 
                        skin.GetDeformer(i);
                }
                //std::vector<int> boneIndices;
                //std::vector<unsigned char> boneWeights;
                //std::vector<Au::Media::FBX::Deformer> deformers = fbxMesh.GetDeformers();
            }
        }
        
        return meshData;
    }
};

class Mesh : public SceneObject::Component
{
friend GFXScene;
public:
    Mesh()
    : transform(0),
    gfxScene(0),
    material(0),
    meshData(0),
    renderState(0),
    mesh(0)
    {
        uniModelMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixModel");
    }
    
    ~Mesh()
    {
        
    }
    
    void SetMesh(const std::string& name)
    { SetMesh(Resource<MeshData>::Get(name)); }
    
    void SetMesh(MeshData* meshData)
    { 
        this->meshData = meshData;
        if(material)
            _setupMesh();
    }
    
    void SetMaterial(const std::string& name)
    { SetMaterial(Resource<Material>::Get(name)); }
    
    void SetMaterial(Material* mat)
    {
        this->material = mat;
        this->renderState = mat->Finalize(gfxScene->GetDevice());
        if(meshData)
            _setupMesh();
    }
    
    void Render(Au::GFX::Device* device)
    {
        uniModelMat4f = GetObject()->GetComponent<Transform>()->GetTransform();
        
        device->Bind(renderState);
        device->Bind(mesh);
        device->Render();
    }

    virtual void OnCreate()
    {
        transform = GetObject()->GetComponent<Transform>();
        gfxScene = GetObject()->Root()->GetComponent<GFXScene>();
        gfxScene->AddMesh(this);
    }
private:
    void _setupMesh()
    {
        mesh = gfxScene->GetDevice()->CreateMesh();
        mesh->Format(material->AttribFormat());
        meshData->FillMesh(mesh);
    }

    Transform* transform;
    GFXScene* gfxScene;
    
    Material* material;
    MeshData* meshData;
    Au::GFX::RenderState* renderState;
    Au::GFX::Mesh* mesh;
    
    Au::GFX::Uniform uniModelMat4f;
};

#endif
