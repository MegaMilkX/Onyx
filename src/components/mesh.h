#ifndef COMPONENT_MESH_H
#define COMPONENT_MESH_H

#include "../scene_object.h"
#include "transform.h"
#include "renderer.h"
#include <material.h>

#include <mesh_data.h>

#include "../frame_stage_static.h"

#undef GetObject

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
    mesh(0),
    subMesh(0)
    {
    }
    
    ~Mesh()
    {
        
    }
    
    void SetMesh(const std::string& name)
    { 
        meshName = name;
        SetMesh(Resource<MeshData>::Get(name)); 
    }
    
    void SetMesh(MeshData* meshData)
    { 
        this->meshData = meshData;
    }
    
    MeshData* GetMeshData() { return meshData; }
    
    void SetSubMesh(const std::string& name)
    {
        subMeshName = name;
    }
    void SetSubMesh(unsigned int i)
    {
        if(i == 0)
        {
            subMeshName = "";
            return;
        }
        
        if(i > meshData->subDataArray.size())
            return;
        
        subMeshName = meshData->subDataArray[i - 1].name;
    }
    
    void SetMaterial(const std::string& name)
    {
        materialName = name;
        SetMaterial(Resource<Material>::Get(name)); 
    }
    
    void SetMaterial(Material* mat)
    {
        this->material = mat;
    }
    
    Material* GetMaterial() { return material; }
    
    void VertexShaderSource(const std::string& source) { vertexShaderSource = source; }
    std::string& VertexShaderSource() { return vertexShaderSource; }

    virtual void OnCreate()
    {
        transform = GetObject()->GetComponent<Transform>();
        renderer = GetObject()->Root()->GetComponent<Renderer>();
        
        renderer->GetStage<FrameStageStatic>();
    }
    virtual std::string Serialize() 
    {
        using json = nlohmann::json;
        json j = json::object();
        j["Material"] = materialName;
        j["Mesh"] = meshName;
        j["SubMesh"] = subMeshName;
        return j.dump(); 
    }
    virtual void Deserialize(const std::string& data) 
    {
        using json = nlohmann::json;
        json j = json::parse(data);
        if(j.is_null())
            return;
        if(j["Mesh"].is_string())
        {
            SetMesh(j["Mesh"].get<std::string>());
        }
        if(j["SubMesh"].is_string())
        {
            SetSubMesh(j["SubMesh"].get<std::string>());
        }
        if(j["Material"].is_string())
        {
            SetMaterial(j["Material"].get<std::string>());
        }
    }
protected:    
    bool dirty;

    Transform* transform;
    Renderer* renderer;
    
    std::string materialName;
    Material* material;
    std::string meshName;
    MeshData* meshData;
    Au::GFX::RenderState* renderState;
    Au::GFX::Mesh* mesh;
    Au::GFX::Mesh::SubMesh* subMesh;
    std::string subMeshName;
    
    Au::GFX::Uniform uniModelMat4f;
    
    std::string vertexShaderSource;
};

class DebugTransformIcon : public Mesh
{
public:

    virtual void Build()
    {
        static Au::GFX::Mesh* m = 
            CreateCrossMesh();
        static Au::GFX::RenderState* rs = 
            CreateCrossRS();
            
        mesh = m;
        subMesh = m->GetSubMesh(0);
        renderState = rs;
    }

    virtual void OnCreate()
    {
        Mesh::OnCreate();
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
            0.8f, 0.2f, 0.2f,
            0.8f, 0.2f, 0.2f,
            0.2f, 0.8f, 0.2f,
            0.2f, 0.8f, 0.2f,
            0.2f, 0.2f, 0.8f,
            0.2f, 0.2f, 0.8f
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
