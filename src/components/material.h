#ifndef COMPONENT_MATERIAL_H
#define COMPONENT_MATERIAL_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include <aurora/glslstitch.h>

#include "../scene_object.h"
#include "gfxscene.h"

class Material : public SceneObject::Component
{
public:
    struct Layer
    {
        Layer(int index, const std::string& name)
        : index(index), name(name), blend("add") {}
        
        bool operator<(const Layer& other)
        {
            return index < other.index;
        }
        
        int index;
        std::string name;
        std::string blend;
    };

    void SetLayer(int index, const std::string& name)
    {
        layers.push_back(Layer(index, name));
        std::sort(layers.begin(), layers.end());
    }
    
    void BlendLayer(int index, const std::string& name)
    {
        
    }
    
    void Finalize()
    {
        std::vector<Au::GLSLStitch::Snippet> specFragSnips = fragSnips;
        
        std::string vshader;
        std::string fshader;
        
        Au::GLSLStitch::Snippet vSnip = 
        Au::GLSLStitch::AssembleSnippet(
            vertSnips,
            R"(
                in vec3 PositionWorld;
                gl_Position = PositionWorld;
            )"
        );
        
        Au::GLSLStitch::Snippet fSnip =
            Au::GLSLStitch::MakeSnippet(
                R"(
                    in vec4 input;
                    out vec4 fragOut;
                    fragOut = input;
                )"
            );
        
        if(layers.empty())
        {
            return;
        }
        else if(layers.size() == 1)
        {
            fSnip.RenameInput("input", layers[0].name);            
        }
        else
        {
            std::string firstInput = layers[0].name;
            for(unsigned i = 1; i < layers.size(); ++i)
            {
                Au::GLSLStitch::Snippet blendSnip;
                if(!_getGenericSnippet(layers[i].blend, blendSnip))
                    continue;
                blendSnip.RenameInput("first", firstInput);
                blendSnip.RenameInput("second", layers[i].name);
                firstInput = blendSnip.name + std::to_string(i - 1);
                blendSnip.RenameOutput("result", firstInput);
                specFragSnips.push_back(blendSnip);
            }
            fSnip.RenameInput("input", firstInput);
        }
        
        fSnip = Au::GLSLStitch::AssembleSnippet(
            specFragSnips,
            fSnip
        );
        
        Au::GLSLStitch::CleanupOutputs(
            fSnip, 
            Au::GLSLStitch::MakeSnippet("in vec4 fragOut;")
        );
        LinkSnippets(vSnip, fSnip, vertSnips);
        
        vshader = Au::GLSLStitch::Finalize(vSnip);
        fshader = Au::GLSLStitch::Finalize(fSnip);
        
        vshader = std::string("#version 130\n") + vshader;
        fshader = std::string("#version 130\n") + fshader;
        
        std::cout << "== VERTEX =========" << std::endl;
        std::cout << vshader << std::endl;
        std::cout << "== FRAGMENT =======" << std::endl;
        std::cout << fshader << std::endl;
        
        Au::GFX::Shader* shaderVertex = gfxDevice->CreateShader(Au::GFX::Shader::VERTEX);
        shaderVertex->Source(vshader);
        std::cout << shaderVertex->StatusString() << std::endl;
        Au::GFX::Shader* shaderFragment = gfxDevice->CreateShader(Au::GFX::Shader::PIXEL);
        shaderFragment->Source(fshader);
        std::cout << shaderFragment->StatusString() << std::endl;
        
        renderState = gfxDevice->CreateRenderState();
        renderState->AttribFormat(Au::Position() << Au::Normal() << Au::ColorRGB());
        renderState->SetShader(shaderVertex);
        renderState->SetShader(shaderFragment);
        
        _gatherUniforms(vSnip);
        _gatherUniforms(fSnip);
        
        std::cout << renderState->StatusString() << std::endl;
    }
    
    void Bind(Au::GFX::Device* device)
    {
        device->Bind(renderState);
    }
    
    virtual void OnCreate()
    {
        Au::GLSLStitch::MakeSnippets(
            #include "material/snippets.glsl"
            ,
            vertSnips,
            fragSnips,
            genericSnips
        );
        
        gfxScene = GetObject()->Root()->GetComponent<GFXScene>();
        gfxDevice = gfxScene->GetDevice();
    }
private:
    GFXScene* gfxScene;
    Au::GFX::Device* gfxDevice;
    Au::GFX::RenderState* renderState;

    std::vector<Layer> layers;
    
    void _gatherUniforms(Au::GLSLStitch::Snippet& snip)
    {
        for(unsigned i = 0; i < snip.other.size(); ++i)
        {
            Au::GLSLStitch::Variable& var =
                snip.other[i];
            
            //if(var.type == "bool")
            //    renderState->AddUniform<bool>(var.name);
            //else 
            if(var.type == "int")
                renderState->AddUniform<int>(var.name);
            else if(var.type == "uint")
                renderState->AddUniform<unsigned int>(var.name);
            else if(var.type == "float")
                renderState->AddUniform<float>(var.name);
            //else if(var.type == "double")
            //    renderState->AddUniform<double>(var.name);
            else if(var.type == "vec2")
                renderState->AddUniform<Au::Math::Vec2f>(var.name);
            else if(var.type == "vec3")
                renderState->AddUniform<Au::Math::Vec3f>(var.name);
            else if(var.type == "vec4")
                renderState->AddUniform<Au::Math::Vec4f>(var.name);
            else if(var.type == "mat3")
                renderState->AddUniform<Au::Math::Mat3f>(var.name);
            else if(var.type == "mat4")
                renderState->AddUniform<Au::Math::Mat4f>(var.name);
            
        }
    }
    
    bool _getGenericSnippet(const std::string& name, Au::GLSLStitch::Snippet& snip)
    {
        for(unsigned i = 0; i < genericSnips.size(); ++i)
            if(genericSnips[i].name == name)
            {
                snip = genericSnips[i];
                return true;
            }
        return false;
    }

    std::vector<Au::GLSLStitch::Snippet> vertSnips;
    std::vector<Au::GLSLStitch::Snippet> fragSnips;
    std::vector<Au::GLSLStitch::Snippet> genericSnips;
    
};

#endif
