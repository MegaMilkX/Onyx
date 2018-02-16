#ifndef FRAME_STAGE_STATIC_H
#define FRAME_STAGE_STATIC_H

#include "frame_stage.h"

#include <aurora/gfx.h>

#include "components/transform.h"
#include "resources/mesh_data.h"
#include "resources/texture2d.h"
#include "components/renderer.h"

#define STAGE_PRIORITY_STATIC_GEOMETRY 0

class Mesh;
class LightDirect;
class LightOmni;
inline GLuint _initShader(const std::string& source, GLuint shaderProgram, GLuint type)
{
    const char* csource = source.c_str();
    GLuint vs = glCreateShader(type);
    glShaderSource(vs, 1, &csource, 0);
    glCompileShader(vs);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 1)
    {
        std::vector<char> ShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(vs, InfoLogLength, NULL, &ShaderErrorMessage[0]);
        std::cout << &ShaderErrorMessage[0] << std::endl;
    }

    glAttachShader(shaderProgram, vs);
    return vs;
}

struct GLVertexBufferDesc
{
    std::string name;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    GLenum hint;
};

class GLVertexArrayObject
{
public:
    void Init(const std::vector<GLVertexBufferDesc>& buffersDesc)
    {
        this->buffersDesc = buffersDesc;
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        for(unsigned i = 0; i < buffersDesc.size(); ++i)
        {
            GLuint buf;
            glGenBuffers(1, &buf);
            glBindBuffer(GL_ARRAY_BUFFER, buf);
            buffers.push_back(buf);
        }
        glGenBuffers(1, &indexBuf);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    }
    void Cleanup()
    {
        buffersDesc.clear();
        for(GLuint buf : buffers)
        {
            glDeleteBuffers(1, &buf);
        }
        buffers.clear();
        glDeleteVertexArrays(1, &vao);
    }
    void FillArrayBuffer(const std::string& name, const std::vector<unsigned char>& data)
    {
        GLVertexBufferDesc desc;
        int bufId = _getBuf(name, desc);
        if(bufId == -1)
            return;
        void* pData = (void*)data.data();
        size_t szData = data.size();
        glBindBuffer(GL_ARRAY_BUFFER, buffers[bufId]);
        glBufferData(GL_ARRAY_BUFFER, szData, pData, desc.hint);
        glEnableVertexAttribArray(bufId);
        glVertexAttribPointer(bufId, desc.size, desc.type, desc.normalized, desc.stride, 0);
    }
    void FillIndexBuffer(const std::vector<unsigned short>& data)
    {
        size_t szData = data.size() * sizeof(unsigned short);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, szData, (void*)data.data(), GL_STATIC_DRAW);
        indexCount = szData / sizeof(unsigned short);
    }
    void Bind()
    {
        glBindVertexArray(vao);
    }
    void DrawElements(GLenum primitiveType)
    {
        glDrawElements(primitiveType, indexCount, GL_UNSIGNED_SHORT, (void*)0);
    }
private:
    int _getBuf(const std::string& name, GLVertexBufferDesc& descOut)
    {
        int i = 0;
        for(GLVertexBufferDesc& desc : buffersDesc)
        {
            if(desc.name == name)
            {
                descOut = desc;
                return i;
            }
            ++i;
        }
        return -1;
    }
    int indexCount;
    GLuint vao;
    std::vector<GLVertexBufferDesc> buffersDesc;
    std::vector<GLuint> buffers;
    GLuint indexBuf;
};

class FrameStageStatic : public FrameStage
{
public:
    virtual int Priority() { return STAGE_PRIORITY_STATIC_GEOMETRY; }
    std::string VertexShader()
    {
        return R"(#version 450
            in vec3 Position;
            in vec2 UV;
            in vec3 Normal;

            out vec2 UVFrag;
            out vec3 NormalModel;
            out vec3 FragPosWorld;

            uniform mat4 MatrixModel;
            uniform mat4 MatrixView;
            uniform mat4 MatrixProjection;

            void main()
            {
                vec4 PositionScreen;
                vec4 PositionModel;
                
                FragPosWorld = vec3 ( MatrixModel * PositionModel ) ; 
                NormalModel = normalize ( ( MatrixModel * vec4 ( Normal , 0.0 ) ) . xyz ) ; 
                UVFrag = UV ; 
                PositionModel = vec4 ( Position , 1.0 ) ; 
                PositionScreen = MatrixProjection * MatrixView * MatrixModel * PositionModel ; 
                gl_Position = PositionScreen ; 
            }
        )";
    }
    std::string PixelShader()
    {
        return R"(#version 450
            #define LIGHT_DIRECT_COUNT 2
            #define LIGHT_OMNI_COUNT 2
            in vec2 UVFrag;
            in vec3 NormalModel;
            in vec3 FragPosWorld;

            out vec4 fragOut;

            uniform sampler2D DiffuseTexture;
            uniform vec3 AmbientColor;
            uniform vec3 LightDirectRGB[LIGHT_DIRECT_COUNT];
            uniform vec3 LightDirect[LIGHT_DIRECT_COUNT];
            uniform vec3 LightOmniRGB[LIGHT_OMNI_COUNT];
            uniform vec3 LightOmniPos[LIGHT_OMNI_COUNT];

            void main()
            {
                vec4 add2;
                vec4 add1;
                vec4 multiply0;
                vec4 Diffuse;
                vec4 Ambient;
                vec4 LightDirectLambert;
                vec4 LightOmniLambert;
                vec3 LightDirection[LIGHT_OMNI_COUNT];
                
                for (int i = 0; i < LIGHT_OMNI_COUNT; i++) 
                { 
                    LightDirection[i] = normalize(LightOmniPos[i] - FragPosWorld); 
                } 
                
                LightOmniLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
                for (int i = 0; i < LIGHT_OMNI_COUNT; i++) 
                { 
                    float diff = max(dot(NormalModel, LightDirection[i]) , 0.0); 
                    float dist = distance ( LightOmniPos [ i ] , FragPosWorld ) ; 
                    LightOmniLambert += vec4 ( LightOmniRGB [ i ] * diff * ( 1.0 / ( 1.0 + 0.5 * dist + 3.0 * dist * dist ) ) , 1.0 ) ; 
                } 
                
                LightDirectLambert = vec4 ( 0.0 , 0.0 , 0.0 , 1.0 ) ; 
                for (int i = 0; i < LIGHT_DIRECT_COUNT; ++i)
                { 
                    float diff = max ( dot ( NormalModel , - LightDirect [ i ] ) , 0.0 ) ; 
                    LightDirectLambert += vec4 ( LightDirectRGB [ i ] * diff , 1.0 ) ; 
                } 
                
                Ambient = vec4 ( AmbientColor , 1.0 ) ; 
                Diffuse = texture ( DiffuseTexture , UVFrag ) ; 
                multiply0 = Diffuse * Ambient ; 
                add1 = multiply0 + LightDirectLambert ; 
                add2 = add1 + LightOmniLambert ; 
                fragOut = add2 ; 
            }
        )";
    }
    virtual bool Init()
    {
        shaderProgram = glCreateProgram();
        std::cout << "shaderProgram: " << shaderProgram << std::endl;
        GLuint vs = _initShader(VertexShader(), shaderProgram, GL_VERTEX_SHADER);
        GLuint ps = _initShader(PixelShader(), shaderProgram, GL_FRAGMENT_SHADER);
        
        glBindAttribLocation(shaderProgram, 0, "Position");
        glBindAttribLocation(shaderProgram, 1, "UV");
        glBindAttribLocation(shaderProgram, 2, "Normal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragOut");
        
        glLinkProgram(shaderProgram);
        
        std::string error_str;
        int result;
        int info_log_len;
        glValidateProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &result);
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &info_log_len);
        if (info_log_len > 1)
        {
            std::vector<char> ShaderErrorMessage(info_log_len + 1);
            glGetProgramInfoLog(shaderProgram, info_log_len, NULL, &ShaderErrorMessage[0]);
            error_str = &ShaderErrorMessage[0];
            std::cout << error_str << std::endl;
        }
        
        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "DiffuseTexture"), 0);
        
        return true;
    }
    virtual void Cleanup()
    {
        glDeleteProgram(shaderProgram);
    }
    
    struct RenderUnit
    {
        GLVertexArrayObject vao;
        void Init()
        {
            vao.Init({
                { "Position", 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, GL_STATIC_DRAW },
                { "UV", 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, GL_STATIC_DRAW },
                { "Normal", 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, GL_STATIC_DRAW }
            });
        }
        void Cleanup()
        {
            vao.Cleanup();
        }
        void FillMesh(MeshData* meshData)
        {     
            vao.FillArrayBuffer("Position", meshData->GetAttribBytes<Au::Position>());
            vao.FillArrayBuffer("UV", meshData->GetAttribBytes<Au::UV>());
            vao.FillArrayBuffer("Normal", meshData->GetAttribBytes<Au::Normal>());
            vao.FillIndexBuffer(meshData->GetIndices());
        }
        void Bind()
        {
            vao.Bind();
        }
        
        Au::GFX::Texture2D* diffuse;

        int vertexSize;
        Transform* transform;
    };
    
    virtual void Build(SceneObject* root);
    virtual void Bind()
    {
        //glUseProgram(shaderProgram);
    }
    virtual void Run();
private:
    GLuint shaderProgram;
    std::vector<RenderUnit> units;
    
    std::vector<LightDirect*> lightsDirect;
    std::vector<LightOmni*> lightsOmni;
    
    Renderer* renderer;
};

#endif
