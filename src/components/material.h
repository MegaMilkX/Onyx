#ifndef COMPONENT_MATERIAL_H
#define COMPONENT_MATERIAL_H

#include <vector>
#include <string>
#include <algorithm>

#include <aurora/glslstitch.h>

#include "../scene_object.h"

class Material : public SceneObject::Component
{
public:
    struct Layer
    {
        Layer(int index, const std::string& name)
        : index(index), name(name), blend("multiply") {}
        
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
                firstInput = blendSnip.name + std::to_string(i);
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
        
        std::cout << "== VERTEX =========" << std::endl;
        std::cout << vshader << std::endl;
        std::cout << "== FRAGMENT =======" << std::endl;
        std::cout << fshader << std::endl;
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
        
        
    }
private:
    std::vector<Layer> layers;
    
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
