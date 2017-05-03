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
#include "gfxscene.h"

#include "../resource.h"

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
    
    Au::AttribFormat AttribFormat()
    { return _attribFormat; }

    void SetLayer(
        int index, 
        const std::string& name, 
        const std::string& blend = "add"
        )
    {
        layers.push_back(Layer(index, name, blend));
        std::sort(layers.begin(), layers.end());
    }
    
    void BlendLayer(int index, const std::string& name)
    {
        
    }
    
    Au::GFX::RenderState* Finalize(GFXScene* gfxScene)
    {
        Au::GFX::Device* gfxDevice = gfxScene->GetDevice();
        Au::GFX::RenderState* renderState;
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
        
        renderState = gfxDevice->CreateRenderState();  

        std::string pp;
        _setupPreprocessorDirectives(vSnip, pp, gfxScene);
        vshader = pp + vshader;
        _setupPreprocessorDirectives(fSnip, pp, gfxScene);
        fshader = pp + fshader;
        
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
        
        
        renderState->SetShader(shaderVertex);
        renderState->SetShader(shaderFragment);
        
        _gatherUniforms(vSnip, renderState, gfxScene);
        _gatherUniforms(fSnip, renderState, gfxScene);
        _deductAttribFormat(vSnip, renderState);
        
        std::cout << renderState->StatusString() << std::endl;
        
        return renderState;
    }
private:
    std::vector<Layer> layers;
    Au::AttribFormat _attribFormat;
    
    std::vector<Au::AttribInfo>& _getAttribList()
    {
        static std::vector<Au::AttribInfo> attribs =
            _getAttribListInit();
        return attribs;
    }
    
    static bool _compareAttribByNameLen(const Au::AttribInfo& first, const Au::AttribInfo& second)
    { return first.name.size() > second.name.size(); }
    
    std::vector<Au::AttribInfo> _getAttribListInit()
    {
        std::vector<Au::AttribInfo> result;
        
        result.push_back(Au::Position());
        result.push_back(Au::Normal());
        result.push_back(Au::Tangent());
        result.push_back(Au::Bitangent());
        result.push_back(Au::UV());
        result.push_back(Au::UVW());
        result.push_back(Au::ColorRGBA());
        result.push_back(Au::ColorRGB());
        result.push_back(Au::BoneWeight4());
        result.push_back(Au::BoneIndex4());
        
        std::sort(result.begin(), result.end(), &_compareAttribByNameLen);
        
        return result;
    }
    
    int _tryMatchStr(
        const std::string& str, 
        std::string& token
        )
    {
        if(str.size() < token.size())
            return 0;
        
        for(unsigned i = 0; i < token.size(); ++i)
        {
            if(token[i] != str[i])
                return 0;
        }

        return token.size();
    }
    
    void _deductAttribFormat(Au::GLSLStitch::Snippet& snip, Au::GFX::RenderState* renderState)
    {
        Au::AttribFormat attribFormat;
        std::vector<Au::AttribInfo> attribs = _getAttribList();
                
        for(unsigned i = 0; i < snip.inputs.size(); ++i)
        {
            Au::GLSLStitch::Variable& var =
                snip.inputs[i];
        
            for(unsigned j = 0; j < attribs.size(); ++j)
            {
                Au::AttribInfo& attr = attribs[j];
                int r = _tryMatchStr(var.name, attr.name);
                if(r)
                {
                    attribFormat << attr;
                    break;
                }
            }
        }
        //attribFormat.Print();
        renderState->AttribFormat(attribFormat);
        _attribFormat = attribFormat;
    }
    
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
        GFXScene* gfxScene)
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
                sz = gfxScene->GetInt(var.arraySize);
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
        
        pp = std::string("#version 140\n") + pp;
    }
    
    void _gatherUniforms(
        Au::GLSLStitch::Snippet& snip, 
        Au::GFX::RenderState* renderState,
        GFXScene* gfxScene)
    {
        for(unsigned i = 0; i < snip.other.size(); ++i)
        {
            Au::GLSLStitch::Variable& var =
                snip.other[i];
            
            int sz = 0;
            sz = _toInt(var.arraySize);
            if(sz == 0)
            {
                sz = gfxScene->GetInt(var.arraySize);
            }
            
            if(sz == 0) sz = 1;
            
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
            lua.SetGlobal(material, "Material");
            lua.LoadSource(std::string(buffer.begin(), buffer.end()));
            lua.Cleanup();
        }
        
        return material;
    }
};

#endif
