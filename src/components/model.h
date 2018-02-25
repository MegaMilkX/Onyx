#ifndef COMPONENT_MODEL_H
#define COMPONENT_MODEL_H

#include "../scene_object.h"
#include "transform.h"
#include "renderer.h"
#include <material.h>

#include <mesh_data.h>

#include "../util/gl_helpers.h"
#include "../util/gl_render_state.h"

#include <mutex>

#include "../asset.h"

#undef GetObject

struct RenderUnitSolid
{    
    GLuint vao;
    int indexCount;
    GLuint texDiffuse;

    int vertexSize;
    Transform* transform;
};

struct ShaderProgramSolid
{
    GLuint id;
    GLuint uniProjection;
    GLuint uniView;
    GLuint uniModel;
    GLuint uniAmbientColor;
};

struct ShaderProgramSkin
{
    GLuint id;
    GLuint uniProjection;
    GLuint uniView;
    GLuint uniModel;
    GLuint uniAmbientColor;
    GLuint uniBoneInverseTransforms;
    GLuint uniBoneTransforms;
};

struct SolidMeshDrawData
{
    ShaderProgramSolid shaderSolid;
    ShaderProgramSkin shaderSkin;
    std::vector<RenderUnitSolid> renderUnits;
    std::vector<RenderUnitSolid> skinUnits;
};

struct SolidDrawStats
{
    int drawCalls;
};

inline void ShaderSolidInit(const FrameCommon& frame, SolidMeshDrawData& out);
inline void ShaderSkinInit(const FrameCommon& frame, SolidMeshDrawData& out);
inline void SolidMeshDrawInitUnits(const FrameCommon& frame, SolidMeshDrawData& out);

inline void SolidMeshDraw(
    const FrameCommon& frame, 
    const SolidMeshDrawData& in)
{
    glUseProgram(in.shaderSolid.id);
    
    glUniformMatrix4fv(
        in.shaderSolid.uniProjection, 1, GL_FALSE,
        (float*)&frame.projection
    );
    glUniformMatrix4fv(
        in.shaderSolid.uniView, 1, GL_FALSE,
        (float*)&frame.view
    );
    
    glUniform3f(
        in.shaderSolid.uniAmbientColor,
        0.4f, 0.3f, 0.2f
    );
    
    for(const RenderUnitSolid& unit : in.renderUnits)
    {
        glUniformMatrix4fv(
            in.shaderSolid.uniModel, 1, GL_FALSE,
            (float*)&unit.transform->GetTransform()
        );
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, unit.texDiffuse);
        
        glBindVertexArray(unit.vao);
        glDrawElements(GL_TRIANGLES, unit.indexCount, GL_UNSIGNED_SHORT, (void*)0);
    }
}

class Model : public SceneObject::Component
{
friend Renderer;
public:
    Model();    
    ~Model();

    asset<MeshData> mesh;
    asset<Material> material;
    
    virtual void OnCreate();
    virtual std::string Serialize() ;
    virtual void Deserialize(const std::string& data) ;
protected:    
    bool dirty;

    Transform* transform;
    Renderer* renderer;

    std::string materialName;
    std::string meshName;    
    std::string subMeshName;
};

void ShaderSolidInit(const FrameCommon& frame, SolidMeshDrawData& out)
{
    gl::Shader vs;
    gl::Shader fs;
    vs.Init(GL_VERTEX_SHADER);
    vs.Source(
        #include "../shaders/solid_vs.glsl"
    );
    vs.Compile();
    fs.Init(GL_FRAGMENT_SHADER);
    fs.Source(
        #include "../shaders/solid_fs.glsl"
    );
    fs.Compile();

    gl::ShaderProgram* prog = new gl::ShaderProgram;
    prog->AttachShader(&vs);
    prog->AttachShader(&fs);

    prog->BindAttrib(0, "Position");
    prog->BindAttrib(1, "UV");
    prog->BindAttrib(2, "Normal");

    prog->BindFragData(0, "fragOut");

    prog->Link();

    glUniform1i(prog->GetUniform("DiffuseTexture"), 0);

    out.shaderSolid.id = prog->GetId();
    out.shaderSolid.uniProjection = prog->GetUniform("MatrixProjection");
    out.shaderSolid.uniView = prog->GetUniform("MatrixView");
    out.shaderSolid.uniModel = prog->GetUniform("MatrixModel");
    out.shaderSolid.uniAmbientColor = prog->GetUniform("AmbientColor");

    std::cout << "Solid shader initialized" << std::endl;
}

void ShaderSkinInit(const FrameCommon& frame, SolidMeshDrawData& out)
{
    GLuint program = glCreateProgram();
    _initShader(
        #include "../shaders/skin_vs.glsl"
        , program,
        GL_VERTEX_SHADER
    );
    _initShader(
        #include "../shaders/solid_fs.glsl"
        , program,
        GL_FRAGMENT_SHADER
    );
    glBindAttribLocation(program, 0, "Position");
    glBindAttribLocation(program, 1, "UV");
    glBindAttribLocation(program, 2, "Normal");
    glBindAttribLocation(program, 3, "BoneIndex4");
    glBindAttribLocation(program, 4, "BoneWeight4");
    
    glBindFragDataLocation(program, 0, "fragOut");
    
    glLinkProgram(program);

    std::string error_str;
    int result;
    int info_log_len;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_len);
    if (info_log_len > 1)
    {
        std::vector<char> ShaderErrorMessage(info_log_len + 1);
        glGetProgramInfoLog(program, info_log_len, NULL, &ShaderErrorMessage[0]);
        error_str = &ShaderErrorMessage[0];
        std::cout << error_str << std::endl;
    }
    
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "DiffuseTexture"), 0);

    out.shaderSkin.id = program;
    out.shaderSkin.uniProjection = glGetUniformLocation(program, "MatrixProjection");
    out.shaderSkin.uniView = glGetUniformLocation(program, "MatrixView");
    out.shaderSkin.uniModel = glGetUniformLocation(program, "MatrixModel");
    out.shaderSkin.uniAmbientColor = glGetUniformLocation(program, "AmbientColor");
    
    std::cout << "Skin shader initialized" << std::endl;
}

void SolidMeshDrawInitUnits(const FrameCommon& frame, SolidMeshDrawData& out)
{
    Renderer* renderer = frame.scene->GetComponent<Renderer>();
    std::vector<Model*> meshes = frame.scene->FindAllOf<Model>();
    for(Model* mesh : meshes)
    {
        if(!mesh->mesh.get())
            continue;
        
        RenderUnitSolid unit;
        unit.transform = mesh->GetComponent<Transform>();
        
        unit.vao = mesh->mesh.get()->GetVao({
            { "Position", 3, GL_FLOAT, GL_FALSE },
            { "UV", 2, GL_FLOAT, GL_FALSE },
            { "Normal", 3, GL_FLOAT, GL_FALSE }
        });
        unit.indexCount = mesh->mesh.get()->GetIndexCount();
        
        Texture2D* tex = 
            Resource<Texture2D>::Get(mesh->material.get()->GetString("Diffuse"));
        unit.texDiffuse = tex->GetGlName();
        
        out.renderUnits.push_back(unit);
    }
    
    std::cout << "Created " << out.renderUnits.size() << " render units" << std::endl;
}

/*
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
*/
#endif
