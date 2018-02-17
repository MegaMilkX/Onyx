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
    std::string VertexShader();
    std::string PixelShader();
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
    virtual void Run();
private:
    GLuint shaderProgram;
    std::vector<RenderUnit> units;
    
    std::vector<LightDirect*> lightsDirect;
    std::vector<LightOmni*> lightsOmni;
    
    Renderer* renderer;
};

#endif
