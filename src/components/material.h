#ifndef COMPONENT_MATERIAL_H
#define COMPONENT_MATERIAL_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>

#include <aurora/glslstitch.h>
#include <aurora/lua.h>

#include "../scene_object.h"
#include "renderer.h"

#include "../resource.h"

#include "texture2d.h"

class Material
{
public:
    struct Layer
    {
        Layer(int index, const std::string& name, const std::string& blend)
        : index(index), name(name), blend(blend) {}
        
        bool operator<(const Layer& other)
        {
            return index < other.index;
        }
        
        int index;
        std::string name;
        std::string blend;
    };
    
    Material()
    {
        Au::GLSLStitch::MakeSnippets(
            #include "material/snippets.glsl"
            ,
            vertSnips,
            fragSnips,
            genericSnips
        );
    }

    void SetLayer(
        int index, 
        const std::string& name, 
        const std::string& blend = "add"
        )
    {
        layers.push_back(Layer(index, name, blend));
        std::sort(layers.begin(), layers.end());
    }
    
    std::map<std::string, Texture2D*> _textures2D;
    std::map<int, Au::GFX::Texture2D*> _samplers2D;
    std::map<Au::GFX::Uniform, float> _uniformsFloat;
    std::map<Au::GFX::Uniform, Au::Math::Vec2f> _uniformsVec2f;
    std::map<Au::GFX::Uniform, Au::Math::Vec3f> _uniformsVec3f;
    std::map<Au::GFX::Uniform, Au::Math::Vec4f> _uniformsVec4f;
    
    void SetTexture2D(const std::string& uniform, const std::string& resource)
    {
        _textures2D[uniform] = Resource<Texture2D>::Get(resource);
    }
    void SetFloat(const std::string& uniform, float value) 
    {
        _uniformsFloat[Au::GFX::GetUniform<float>(uniform)] = value;
    }
    void SetVec2(const std::string& uniform, float x, float y) 
    {
        _uniformsVec2f[Au::GFX::GetUniform<Au::Math::Vec2f>(uniform)] = Au::Math::Vec2f(x, y);
    }
    void SetVec3(const std::string& uniform, float x, float y, float z) 
    {
        _uniformsVec3f[Au::GFX::GetUniform<Au::Math::Vec3f>(uniform)] = Au::Math::Vec3f(x, y, z);
    }
    void SetVec4(const std::string& uniform, float x, float y, float z, float w) 
    {
        _uniformsVec4f[Au::GFX::GetUniform<Au::Math::Vec4f>(uniform)] = Au::Math::Vec4f(x, y, z, w);
    }
    void BindParameters()
    {
        std::map<int, Au::GFX::Texture2D*>::iterator itSampler2D;
        std::map<Au::GFX::Uniform, float>::iterator itFloat;
        std::map<Au::GFX::Uniform, Au::Math::Vec2f>::iterator itVec2f;
        std::map<Au::GFX::Uniform, Au::Math::Vec3f>::iterator itVec3f;
        std::map<Au::GFX::Uniform, Au::Math::Vec4f>::iterator itVec4f;
        
        for(itSampler2D = _samplers2D.begin(); itSampler2D != _samplers2D.end(); ++itSampler2D)
        {
            itSampler2D->second->Bind(itSampler2D->first);
        }
        for(itFloat = _uniformsFloat.begin(); itFloat != _uniformsFloat.end(); ++itFloat)
        {
            Au::GFX::Uniform u = itFloat->first;
            u = itFloat->second;
        }
        for(itVec2f = _uniformsVec2f.begin(); itVec2f != _uniformsVec2f.end(); ++itVec2f)
        {    
            Au::GFX::Uniform u = itVec2f->first;
            u = itVec2f->second;
        }
        for(itVec3f = _uniformsVec3f.begin(); itVec3f != _uniformsVec3f.end(); ++itVec3f)
        {
            Au::GFX::Uniform u = itVec3f->first;
            u = itVec3f->second;
        }
        for(itVec4f = _uniformsVec4f.begin(); itVec4f != _uniformsVec4f.end(); ++itVec4f)
        {
            Au::GFX::Uniform u = itVec4f->first;
            u = itVec4f->second;
        }
    }
    
    Au::GFX::RenderState* Finalize(Renderer* renderer, const std::string& vertexShaderSnippets)
    {
        Au::GFX::Device* gfxDevice = renderer->GetDevice();
        
        Au::GFX::RenderState* renderState;
        
        std::string vshader;
        std::string fshader;
        
        std::vector<Au::GLSLStitch::Snippet> extVertSnips;
        std::vector<Au::GLSLStitch::Snippet> extFragSnips;
        std::vector<Au::GLSLStitch::Snippet> extGenericSnips;
        
        Au::GLSLStitch::MakeSnippets(
            vertexShaderSnippets,
            extVertSnips,
            extFragSnips,
            extGenericSnips
        );
        
        extVertSnips.insert(extVertSnips.end(), vertSnips.begin(), vertSnips.end());
        extFragSnips.insert(extFragSnips.end(), fragSnips.begin(), fragSnips.end());
        
        std::vector<Au::GLSLStitch::Snippet> specFragSnips = extFragSnips;
        
        Au::GLSLStitch::Snippet vSnip = 
            Au::GLSLStitch::AssembleSnippet(
                extVertSnips,
                R"(
                    in vec4 PositionScreen;
                    gl_Position = PositionScreen;
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
        LinkSnippets(vSnip, fSnip, extVertSnips);
        
        vshader = Au::GLSLStitch::Finalize(vSnip);
        fshader = Au::GLSLStitch::Finalize(fSnip);
        
        renderState = gfxDevice->CreateRenderState();  

        std::string pp;
        _setupPreprocessorDirectives(vSnip, pp, renderer);
        vshader = pp + vshader;
        _setupPreprocessorDirectives(fSnip, pp, renderer);
        fshader = pp + fshader;
        
        //std::cout << "== VERTEX =========" << std::endl;
        //std::cout << vshader << std::endl;
        //std::cout << "== FRAGMENT =======" << std::endl;
        //std::cout << fshader << std::endl;
        
        Au::GFX::Shader* shaderVertex = gfxDevice->CreateShader(Au::GFX::Shader::VERTEX);
        shaderVertex->Source(vshader);
        std::cout << shaderVertex->StatusString() << std::endl;
        Au::GFX::Shader* shaderFragment = gfxDevice->CreateShader(Au::GFX::Shader::PIXEL);
        shaderFragment->Source(fshader);
        std::cout << shaderFragment->StatusString() << std::endl;
        
        
        renderState->SetShader(shaderVertex);
        renderState->SetShader(shaderFragment);
        
        _gatherUniforms(vSnip, renderState, renderer);
        _gatherUniforms(fSnip, renderState, renderer);
        
        std::map<std::string, Texture2D*>::iterator itTexture;
        for(itTexture = _textures2D.begin(); itTexture != _textures2D.end(); ++itTexture)
        {
            int layer = renderState->GetSampler2DLayer(itTexture->first);
            _samplers2D[layer] = gfxDevice->CreateTexture2D();
            itTexture->second->Fill(_samplers2D[layer]);
        }
        
        std::cout << renderState->StatusString() << std::endl;
        
        return renderState;
    }
private:
    std::vector<Layer> layers;    
    
    int _toInt(const std::string& str)
    {
        long int i;
        char* end;
        i = strtol(str.c_str(), &end, 10);
        return (int)i;
    }
    
    void _setupPreprocessorDirectives(
        Au::GLSLStitch::Snippet& snip, 
        std::string& pp,
        Renderer* renderer)
    {
        pp = "";
        std::set<std::string> lines;
        for(unsigned i = 0; i < snip.other.size(); ++i)
        {
            Au::GLSLStitch::Variable& var =
                snip.other[i];
                
            int sz = 0;
            sz = _toInt(var.arraySize);
            if(sz == 0 && !var.arraySize.empty())
            {
                sz = renderer->GetInt(var.arraySize);
                lines.insert(std::string("#define ") + 
                    var.arraySize + 
                    " " + 
                    std::to_string(sz) + 
                    "\n");
            }
        }
        
        std::set<std::string>::iterator it = lines.begin();
        for(it; it != lines.end(); ++it)
        { pp += (*it); }
        
        pp = std::string("#version 450\n") + pp;
    }
    
    void _gatherUniforms(
        Au::GLSLStitch::Snippet& snip, 
        Au::GFX::RenderState* renderState,
        Renderer* renderer)
    {
        GLuint textureLayer = 0;
        for(unsigned i = 0; i < snip.other.size(); ++i)
        {
            Au::GLSLStitch::Variable& var =
                snip.other[i];
            
            int sz = 0;
            sz = _toInt(var.arraySize);
            if(sz == 0)
            {
                sz = renderer->GetInt(var.arraySize);
            }
            
            if(sz == 0) sz = 1;
            
            std::cout << var.name << ": " << sz << std::endl;
            
            //if(var.type == "bool")
            //    renderState->AddUniform<bool>(var.name, sz);
            //else 
            if(var.type == "int")
                renderState->AddUniform<int>(var.name, sz);
            else if(var.type == "uint")
                renderState->AddUniform<unsigned int>(var.name, sz);
            else if(var.type == "float")
                renderState->AddUniform<float>(var.name, sz);
            //else if(var.type == "double")
            //    renderState->AddUniform<double>(var.name, sz);
            else if(var.type == "vec2")
                renderState->AddUniform<Au::Math::Vec2f>(var.name, sz);
            else if(var.type == "vec3")
                renderState->AddUniform<Au::Math::Vec3f>(var.name, sz);
            else if(var.type == "vec4")
                renderState->AddUniform<Au::Math::Vec4f>(var.name, sz);
            else if(var.type == "mat3")
                renderState->AddUniform<Au::Math::Mat3f>(var.name, sz);
            else if(var.type == "mat4")
                renderState->AddUniform<Au::Math::Mat4f>(var.name, sz);
            else if(var.type == "sampler2d")
            {
                renderState->AddSampler2D(var.name, textureLayer);
                textureLayer++;
            }
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

class MaterialReaderLUA : public Resource<Material>::Reader
{
public:
    Material* operator()(const std::string& filename)
    {
        Material* material = 0;
        
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if(!file.is_open())
            return 0;
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer((unsigned int)size);
        if(file.read(buffer.data(), size))
        {
            material = new Material();
            
            Au::Lua lua;
            lua.Init();
            lua.Bind(&Material::SetLayer, "SetLayer");
            lua.Bind(&Material::SetTexture2D, "SetTexture2D");
            lua.Bind(&Material::SetFloat, "SetFloat");
            lua.Bind(&Material::SetVec2, "SetVec2");
            lua.Bind(&Material::SetVec3, "SetVec3");
            lua.Bind(&Material::SetVec4, "SetVec4");
            lua.SetGlobal(material, "Material");
            lua.LoadSource(std::string(buffer.begin(), buffer.end()));
            lua.Cleanup();
        }
        
        file.close();
        
        return material;
    }
};

#endif
