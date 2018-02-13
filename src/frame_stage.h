#ifndef FRAME_STAGE_H
#define FRAME_STAGE_H

#include <string>
#include "scene_object.h"

class FrameStage
{
public:
    ~FrameStage() {}
    virtual int Priority() = 0;
    virtual std::string VertexShader() = 0;
    virtual std::string PixelShader() = 0;
    virtual void Build(SceneObject* root) = 0;
    virtual void Run() = 0;
    
    void Init()
    {
        shaderProgram = glCreateProgram();
        GLuint vs = _initShader(VertexShader(), GL_VERTEX_SHADER);
        GLuint ps = _initShader(PixelShader(), GL_FRAGMENT_SHADER);
        glLinkProgram(shaderProgram);
    }
    void Cleanup()
    {
        glDeleteProgram(shaderProgram);
    }
    void Bind()
    {
        glUseProgram(shaderProgram);
    }
private:
    GLuint _initShader(const std::string& source, GLuint type)
    {
        const char* csource = source.c_str();
        Gluint vs = glCreateShader(GL_VERTEX_SHADER);
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
    GLuint shaderProgram;
};

#endif
