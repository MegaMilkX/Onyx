#ifndef COMPONENT_MESH_H
#define COMPONENT_MESH_H

#include <fstream>

#include <aurora/media/fbx.h>

#include "../scene_object.h"
#include "transform.h"
#include "renderer.h"
#include "material.h"

#include "../resource.h"

#undef GetObject

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
        if(!file.is_open())
            return 0;
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer((unsigned int)size);
        if(file.read(buffer.data(), size))
        {
            meshData = new MeshData();
            
            Au::Media::FBX::Reader fbxReader;
            fbxReader.ReadFile(buffer.data(), buffer.size());
            fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
            
            std::vector<Au::Media::FBX::Bone> bones = fbxReader.GetBones();
            
            int meshCount = fbxReader.MeshCount();
            if(meshCount > 0)
            {                
                Au::Media::FBX::Mesh& fbxMesh = fbxReader.GetMesh(0);
                int vertexCount = fbxMesh.VertexCount();
                
                meshData->SetAttribArray<Au::Position>(fbxMesh.GetVertices());
                meshData->SetAttribArray<Au::Normal>(fbxMesh.GetNormals(0));
                meshData->SetIndices(fbxMesh.GetIndices<unsigned short>());
                
                std::vector<Au::Math::Vec4f> boneIndices;
                std::vector<Au::Math::Vec4f> boneWeights;
                std::vector<int> boneDataCount;
                boneIndices.resize(vertexCount, Au::Math::Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
                boneWeights.resize(vertexCount, Au::Math::Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
                boneDataCount.resize(vertexCount, 0);
                for(unsigned j = 0; j < bones.size(); ++j)
                {
                    unsigned boneIndex = j;
                    Au::Media::FBX::Bone& bone = bones[j];
                    if(bone.meshName != fbxMesh.name)
                        continue;
                    
                    for(unsigned k = 0; k < bone.indices.size() && k < bone.weights.size(); ++k)
                    {
                        int32_t vertexIndex = bone.indices[k];
                        float weight = bone.weights[k];
                        if(weight < 0.01f)
                            continue;
                        
                        int& dataCount = boneDataCount[vertexIndex];
                        if(dataCount > 3)
                            continue;
                        
                        boneIndices[vertexIndex][dataCount] = (float)boneIndex;
                        boneWeights[vertexIndex][dataCount] = weight;
                        dataCount++;
                    }
                }
                
                for(int i = 0; i < vertexCount; ++i)
                {
                    Au::Math::Normalize(boneWeights[i]);
                    /*
                    std::cout << i << std::endl;
                    for(unsigned j = 0; j < 4; ++j)
                        std::cout << boneIndices[i][j] << " ";
                    std::cout << std::endl;
                    for(unsigned j = 0; j < 4; ++j)
                        std::cout << boneWeights[i][j] << " ";
                    std::cout << std::endl;
                    */
                }
                
                meshData->SetAttribArray<Au::BoneIndex4>(boneIndices);
                meshData->SetAttribArray<Au::BoneWeight4>(boneWeights);
            }
        }
        
        file.close();
        
        return meshData;
    }
};

class Mesh : public SceneObject::Component
{
friend Renderer;
public:
    Mesh()
    : dirty(true),
    transform(0),
    renderer(0),
    material(0),
    meshData(0),
    renderState(0),
    mesh(0)
    {
        uniModelMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixModel");
        vertexShaderSource =
            R"(
                in vec4 PositionWorld;
                gl_Position = PositionWorld;
            )";
    }
    
    ~Mesh()
    {
        
    }
    
    void SetMesh(const std::string& name)
    { SetMesh(Resource<MeshData>::Get(name)); }
    
    void SetMesh(MeshData* meshData)
    { 
        this->meshData = meshData;
        _dirty();
    }
    
    void SetMaterial(const std::string& name)
    { SetMaterial(Resource<Material>::Get(name)); }
    
    void SetMaterial(Material* mat)
    {
        this->material = mat;
        _dirty();
    }
    
    void VertexShaderSource(const std::string& source) { vertexShaderSource = source; }
    std::string& VertexShaderSource() { return vertexShaderSource; }
    
    void Build()
    {
        if(!dirty)
            return;
        dirty = false;
        if(!material)
            return;
        renderState = material->Finalize(renderer, vertexShaderSource);
        if(!meshData)
            return;
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
        renderer = GetObject()->Root()->GetComponent<Renderer>();
        renderer->AddMesh(this);
    }
protected:
    void _dirty()
    {
        dirty = true;
        renderer->Dirty();
    }

    void _setupMesh()
    {
        mesh = renderer->GetDevice()->CreateMesh();
        mesh->Format(material->AttribFormat());
        meshData->FillMesh(mesh);
    }
    
    bool dirty;

    Transform* transform;
    Renderer* renderer;
    
    Material* material;
    MeshData* meshData;
    Au::GFX::RenderState* renderState;
    Au::GFX::Mesh* mesh;
    
    Au::GFX::Uniform uniModelMat4f;
    
    std::string vertexShaderSource;
};

class DebugTransformIcon : public Mesh
{
public:

    virtual void OnCreate()
    {
        Mesh::OnCreate();
        static Au::GFX::Mesh* m = 
            CreateCrossMesh();
        static Au::GFX::RenderState* rs = 
            CreateCrossRS();
            
        mesh = m;
        renderState = rs;
    }
protected:
    Au::GFX::Mesh* CreateCrossMesh()
    {
        Au::GFX::Device& gfxDevice = *GetObject()->Root()->GetComponent<Renderer>()->GetDevice();
        std::vector<float> vertices =
        { 
            -0.00f, 0.0f, 0.0f, 
            1.0f, 0.0f, 0.0f, 
            0.0f, -0.0f, 0.0f, 
            0.0f, 1.0f, 0.0f, 
            0.0f, 0.0f, -0.0f, 
            0.0f, 0.0f, 1.0f
        };
          
        std::vector<float> colors =
        {
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f
        };

        std::vector<unsigned short> indices =
        { 0, 1, 2, 3, 4, 5 };

        Au::GFX::Mesh* mesh = gfxDevice.CreateMesh();
        mesh->PrimitiveType(Au::GFX::Mesh::LINE);
        mesh->Format(Au::Position() << Au::ColorRGB());
        mesh->VertexAttribByInfo(Au::Position(), (unsigned char*)vertices.data(), vertices.size() * sizeof(float));
        mesh->VertexAttribByInfo(Au::ColorRGB(), (unsigned char*)colors.data(), vertices.size() * sizeof(float));
        mesh->IndexData(indices);
        
        return mesh;
    }
    
    Au::GFX::RenderState* CreateCrossRS()
    {
        Au::GFX::Device& gfxDevice = *GetObject()->Root()->GetComponent<Renderer>()->GetDevice();
        Au::GFX::Shader* shaderVertex = gfxDevice.CreateShader(Au::GFX::Shader::VERTEX);
        shaderVertex->Source(R"(#version 140
            uniform mat4 MatrixModel;
            uniform mat4 MatrixView;
            uniform mat4 MatrixProjection;
            in vec3 Position;
            in vec3 ColorRGB;
            out vec3 color;
            void main()
            {
                color = ColorRGB;
                gl_Position = MatrixProjection * MatrixView * MatrixModel * vec4(Position, 1.0);
            })");
        std::cout << shaderVertex->StatusString() << std::endl;
        
        Au::GFX::Shader* shaderPixel = gfxDevice.CreateShader(Au::GFX::Shader::PIXEL);
        shaderPixel->Source(R"(#version 140
            in vec3 color;
            out vec4 fragOut;
            void main()
            {            
                fragOut = vec4(color, 1.0);
            })");
        std::cout << shaderPixel->StatusString() << std::endl;
        
        Au::GFX::RenderState* renderState = gfxDevice.CreateRenderState();
        renderState->AttribFormat(Au::Position() << Au::ColorRGB());
        renderState->SetShader(shaderVertex);
        renderState->SetShader(shaderPixel);
        renderState->AddUniform<Au::Math::Mat4f>("MatrixModel");
        renderState->AddUniform<Au::Math::Mat4f>("MatrixView");
        renderState->AddUniform<Au::Math::Mat4f>("MatrixProjection");
        //renderState->DepthTest(false);
        
        return renderState;
    }
};

#endif
