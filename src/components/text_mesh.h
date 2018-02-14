#ifndef TEXT_MESH_H
#define TEXT_MESH_H

#include "../scene_object.h"

#include "renderer.h"

#include <font_data.h>
#include "../lib/nimbusmono_bold.otf.h"

struct TextVertex
{
    float x, y, z;
    float u, v;
    float r, g, b;
};

class TextMesh : public SceneObject::Component
{
public:
    TextMesh()
    {
        static FontData* fd = CreateDefaultFontData();
        fontData = fd;
    }
    void SetText(const std::string& text)
    {
        this->text = text;
        RebuildMesh();
    }
    
    void RebuildMesh()
    {
        std::vector<TextVertex> vertexData;
        std::vector<unsigned short> indexData;
        float advX = 0.0f, advY = 0.0f;
        unsigned short index = 0;
        for(unsigned i = 0; i < text.size(); ++i)
        {
            char charCode = text[i];
            GlyphInfo* glyph = fontData->GetGlyph(charCode);
            vertexData.push_back(
                { advX, 0.0f, 0.0f,
                  0.0f, 0.0f,
                  1.0f, 1.0f, 1.0f }
            );
            vertexData.push_back(
                { advX + glyph->width, 0.0f, 0.0f,
                  1.0f, 0.0f,
                  1.0f, 1.0f, 1.0f }
            );
            vertexData.push_back(
                { advX + glyph->width, glyph->height, 0.0f,
                  1.0f, 1.0f,
                  1.0f, 1.0f, 1.0f }
            );
            vertexData.push_back(
                { advX, glyph->height, 0.0f,
                  0.0f, 1.0f,
                  1.0f, 1.0f, 1.0f }
            );
            advX += glyph->advX;
            advY += glyph->advY;
            indexData.push_back(index + 0);
            indexData.push_back(index + 1);
            indexData.push_back(index + 2);
            indexData.push_back(index + 2);
            indexData.push_back(index + 3);
            indexData.push_back(index + 0);
            index += 4;
        }
        mesh->VertexData(vertexData);
        mesh->IndexData(indexData);
    }
    
    void Render(Au::GFX::Device* device)
    {
        uniModelMat4f = GetComponent<Transform>()->GetTransform();
        GetRenderState(device)->Bind();
        mesh->Bind();
        mesh->GetSubMesh(0)->Render();
    }
    
    void OnCreate()
    {
        renderer = GetObject()->Root()->GetComponent<Renderer>();
        
        mesh = renderer->GetDevice()->CreateMesh();
        mesh->Format(Au::Position() << Au::UV() << Au::ColorRGB());
        
        uniModelMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixModel");
    }
    
    static Au::GFX::RenderState* GetRenderState(Au::GFX::Device* device)
    {
        static Au::GFX::RenderState* rs = 
            CreateTextRenderState(device);
        return rs;
    }
    
protected:
    Au::GFX::Uniform uniModelMat4f;

    static Au::GFX::RenderState* CreateTextRenderState(Au::GFX::Device* gfxDevice)
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
                fragPos = vec3(MatrixModel * vec4(Position, 1.0));
                color = ColorRGB;
                normal = (MatrixModel * vec4(Normal, 0.0)).xyz;
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
                    float dist = distance(LightOmniPos[i], fragPos);
                    vec3 diffuse = LightOmniRGB[i] * diff * (1.0 / (1.0 + 0.5 * dist + 3.0 * dist * dist));
                    
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

    Renderer* renderer;
    Au::GFX::RenderState* renderState;
    Au::GFX::Mesh* mesh;
    
    FontData* CreateDefaultFontData()
    {
        FontData* fd = new FontData();
        fd->LoadMemory((void*)nimbusmono_bold_otf, sizeof(nimbusmono_bold_otf));
        return fd;
    }

    std::string text;
    FontData* fontData;
};

#endif
