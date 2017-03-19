#include "game_state.h"
#include <aurora/media/fbx.h>
#include <aurora/transform.h>
#include <iostream>
#include <fstream>

Au::GFX::Mesh* LoadMesh(Au::GFX::Device* gfxDevice, const std::string& path)
{
    Au::GFX::Mesh* mesh = gfxDevice->CreateMesh();
    mesh->Format(Au::Position() << 
                Au::Normal() << 
                Au::ColorRGB() << 
                Au::BoneWeight4() << 
                Au::BoneIndex4());
    
    // FBX Loading WIP =========================
    
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if(file.read(buffer.data(), size))
    {
        Au::Media::FBX::Reader fbxReader;
        fbxReader.ReadFile(buffer.data(), buffer.size());
        fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
        //fbxReader.Print();
        
        int meshCount = fbxReader.MeshCount();
        if(meshCount > 0)
        {
            Au::Media::FBX::Mesh& fbxMesh = fbxReader.GetMesh(0);
            std::vector<float> vertices = fbxMesh.GetVertices();
            mesh->VertexAttrib<Au::Position>(vertices);
            std::vector<float> normals = fbxMesh.GetNormals(0);
            mesh->VertexAttrib<Au::Normal>(normals);
            std::vector<unsigned short> indices = fbxMesh.GetIndices<unsigned short>();
            mesh->IndexData(indices);
            
            Au::Media::FBX::Skin skin = fbxMesh.GetSkin();
            for(unsigned i = 0; i < skin.DeformerCount(); ++i)
            {
                Au::Media::FBX::SkinDeformer* deformer = 
                    skin.GetDeformer(i);
                    
                
            }
            //std::vector<int> boneIndices;
            //std::vector<unsigned char> boneWeights;
            //std::vector<Au::Media::FBX::Deformer> deformers = fbxMesh.GetDeformers();
        }
    }
    // =========================================
    
    return mesh;
}

Au::GFX::RenderState* CreateRenderState(Au::GFX::Device* gfxDevice)
{
    Au::GFX::Shader* shaderVertex = gfxDevice->CreateShader(Au::GFX::Shader::VERTEX);
    shaderVertex->Source(R"(#version 130
        uniform mat4 MatrixModel;
        uniform mat4 MatrixView;
        uniform mat4 MatrixProjection;
        in vec3 Position;
        in vec3 Normal;
        in vec3 ColorRGB;
        out vec3 color;
        out vec3 normal;
        
        out vec3 fragPos;
        
        uniform mat4 Bones[32];
        in vec4 BoneWeight4;
        in ivec4 BoneIndex4;
        
        void main()
        {
            vec3 skinnedPos = (BoneWeight4[0] * (Bones[BoneIndex4[0]] * vec4(Position, 1.0)) + 
                              BoneWeight4[1] * (Bones[BoneIndex4[1]] * vec4(Position, 1.0)) + 
                              BoneWeight4[2] * (Bones[BoneIndex4[2]] * vec4(Position, 1.0)) + 
                              BoneWeight4[3] * (Bones[BoneIndex4[3]] * vec4(Position, 1.0))
                              ).xyz;
            vec3 skinnedNormal = BoneWeight4[0] * (mat3(Bones[BoneIndex4[0]]) * Normal) +
                                 BoneWeight4[1] * (mat3(Bones[BoneIndex4[1]]) * Normal) +
                                 BoneWeight4[2] * (mat3(Bones[BoneIndex4[2]]) * Normal) +
                                 BoneWeight4[3] * (mat3(Bones[BoneIndex4[3]]) * Normal);
            
            skinnedNormal = normalize(skinnedNormal);
            
            fragPos = vec3(MatrixModel * vec4(Position, 1.0));
            color = ColorRGB;
            normal = (MatrixModel * vec4(Normal, 1.0)).xyz;
            gl_Position = MatrixProjection * MatrixView * MatrixModel * vec4(Position, 1.0);
    })");
    std::cout << shaderVertex->StatusString() << std::endl;
    
    Au::GFX::Shader* shaderPixel = gfxDevice->CreateShader(Au::GFX::Shader::PIXEL);
    shaderPixel->Source(R"(#version 130
        in vec3 color;
        in vec3 normal;
        in vec3 fragPos;
        
        uniform vec3 LightOmniPos[3];
        uniform vec3 LightOmniRGB[3];
        
        out vec4 fragColor;
        
        void main()
        {
            vec3 ambient_color = vec3(0.2, 0.2, 0.2);            
            vec3 result = ambient_color;
            
            for(int i = 0; i < 3; i++)
            {
                vec3 lightDir = normalize(LightOmniPos[i] - fragPos);
                float diff = max(dot(normal, lightDir), 0.0);
                vec3 diffuse = diff * LightOmniRGB[i];
                result += diffuse;
            }
            
            fragColor = vec4(result, 1.0);
    })");
    std::cout << shaderPixel->StatusString() << std::endl;
    
    Au::GFX::RenderState* renderState = gfxDevice->CreateRenderState();
    renderState->AttribFormat(Au::Position() << Au::Normal() << Au::ColorRGB());
    renderState->SetShader(shaderVertex);
    renderState->SetShader(shaderPixel);
    renderState->AddUniform<Au::Math::Mat4f>("MatrixModel");
    renderState->AddUniform<Au::Math::Mat4f>("MatrixView");
    renderState->AddUniform<Au::Math::Mat4f>("MatrixProjection");
    renderState->AddUniform<Au::Math::Vec3f>("LightOmniPos", 3);
    renderState->AddUniform<Au::Math::Vec3f>("LightOmniRGB", 3);
    renderState->AddUniform<Au::Math::Mat4f>("Bones", 32);
    
    std::cout << renderState->StatusString() << std::endl;
    
    return renderState;
}

class StateTest : public GameState
{
public:
    virtual void OnInit()
    {
        mesh = LoadMesh(GFXDevice(), "miku.fbx");
        renderState = CreateRenderState(GFXDevice());
        
        projection = Au::Math::Perspective(fov, 16.0f/9.0f, 0.1f, zfar);
        view.Translate(Au::Math::Vec3f(0.0f, 1.5f, 7.0f));
        
        uniModelMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixModel");
        uniViewMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixView");
        uniProjMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixProjection");
        
        uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos", 3);
        uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB", 3);
        uniLightOmniPos.Set(Au::Math::Vec3f(0.0f, 1.5f, 2.5f), 0);
        uniLightOmniRGB.Set(Au::Math::Vec3f(0.8f, 0.6f, 0.2f), 0);
        uniLightOmniPos.Set(Au::Math::Vec3f(4.0f, 0.0f, 0.0f), 1);
        uniLightOmniRGB.Set(Au::Math::Vec3f(0.6f, 0.8f, 0.2f), 1);
        uniLightOmniPos.Set(Au::Math::Vec3f(-4.0f, 0.0f, 0.0f), 2);
        uniLightOmniRGB.Set(Au::Math::Vec3f(0.8f, 0.6f, 0.8f), 2);
        
        lx = 0.0f; ly = 0.0f;
    }
    virtual void OnSwitch()
    {
        
    }
    virtual void OnCleanup()
    {
        
    }
    virtual void OnUpdate()
    {
        
    }
    virtual void OnRender(Au::GFX::Device* device)
    {
        device->Bind(renderState);
        device->Set(uniModelMat4f, model.GetTransform());
        device->Set(uniViewMat4f, Au::Math::Inverse(view.GetTransform()));
        device->Set(uniProjMat4f, projection);
        device->Bind(mesh);
        device->Render();
    }
    
    virtual void MouseMove(int x, int y)
    {
        uniLightOmniPos.Set(Au::Math::Vec3f(lx += x * 0.01f, ly -= y * 0.01f, 2.5f), 0);
    }
private:
    Au::GFX::RenderState* renderState;
    Au::GFX::Mesh* mesh;
    Au::Math::Transform model;
    Au::Math::Transform view;
    Au::Math::Mat4f projection;
    
    Au::GFX::Uniform uniModelMat4f;
    Au::GFX::Uniform uniViewMat4f;
    Au::GFX::Uniform uniProjMat4f;
    Au::GFX::Uniform uniLightOmniPos;
    Au::GFX::Uniform uniLightOmniRGB;
    
    float fov = 1.6f;
    float zfar = 100.0f;
    
    float lx, ly;
};

int main()
{
    GameState::Init();
    GameState::Push<StateTest>();
    while(GameState::Update());
    GameState::Cleanup();
    return 0;
}