#ifndef FRAME_STAGE_2D_H
#define FRAME_STAGE_2D_H

#include "frame_stage.h"

#include "components/transform.h"
#include "resources/mesh_data.h"
#include "resources/texture2d.h"
#include "components/renderer.h"

#include "util/gl_helpers.h"

#define STAGE_PRIORITY_2D 1000

class FrameStage2d : public FrameStage
{
public:
    virtual int Priority() { return STAGE_PRIORITY_2D; }
    std::string VertexShader();
    std::string PixelShader();
    virtual bool Init()
    {
        shaderProgram = glCreateProgram();
        GLuint vs = _initShader(VertexShader(), shaderProgram, GL_VERTEX_SHADER);
        GLuint ps = _initShader(PixelShader(), shaderProgram, GL_FRAGMENT_SHADER);
        
        glBindAttribLocation(shaderProgram, 0, "Position");
        glBindAttribLocation(shaderProgram, 1, "UV");
        
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
    
    virtual void Build(SceneObject* root);
    virtual void Run();
private:
    GLuint shaderProgram;
    std::vector<TextMesh*> textMeshes;
    
    Renderer* renderer;
};

#endif
