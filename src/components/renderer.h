#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/gfx.h>
#include <aurora/math.h>
#include <aurora/transform.h>
#include "../scene_object.h"

class Mesh;
class LightOmni;
class LightDirect;
class Renderer : public SceneObject::Component
{
friend Mesh;
public:
    Renderer()
    : renderFn(&Renderer::_renderRebuildScene)
    {}

    bool Init(Au::GFX::Device* gfxDevice)
    {
        _gfxDevice = gfxDevice;     
        SetInt("LIGHT_OMNI_COUNT", 1);
        SetInt("LIGHT_DIRECT_COUNT", 1);
        return true;
    }
    
    Au::GFX::Device* GetDevice() { return _gfxDevice; }

    void Render(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform);
        
    void AmbientColor(float r, float g, float b)
    { 
        ambientColor = Au::Math::Vec3f(r, g, b);
        glClearColor (r, g, b, 1.0f);        
    }
    void RimColor(float r, float g, float b)
    { rimColor = Au::Math::Vec3f(r, g, b); }

    void AddLightOmni(LightOmni* light)
    {
        RemoveLightOmni(light);
        lightsOmni.push_back(light);
        SetInt("LIGHT_OMNI_COUNT", lightsOmni.size());
        uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos", lightsOmni.size());
        uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB", lightsOmni.size());
    }
    void RemoveLightOmni(LightOmni* light)
    {
        for(unsigned i = 0; i < lightsOmni.size(); ++i)
        {
            if(lightsOmni[i] == light)
            {
                lightsOmni.erase(lightsOmni.begin() + i);
                break;
            }
        }
        SetInt("LIGHT_OMNI_COUNT", lightsOmni.size());
        uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos", lightsOmni.size());
        uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB", lightsOmni.size());
    }
    
    void AddLightDirect(LightDirect* light)
    {
        RemoveLightDirect(light);
        lightsDirect.push_back(light);
        SetInt("LIGHT_DIRECT_COUNT", lightsDirect.size());
        uniLightDirect = Au::GFX::GetUniform<Au::Math::Vec3f>("LightDirect", lightsDirect.size());
        uniLightDirectRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightDirectRGB", lightsDirect.size());
    }
    void RemoveLightDirect(LightDirect* light)
    {
        for(unsigned i = 0; i < lightsDirect.size(); ++i)
        {
            if(lightsDirect[i] == light)
            {
                lightsDirect.erase(lightsDirect.begin() + i);
                break;
            }
        }
        SetInt("LIGHT_DIRECT_COUNT", lightsDirect.size());
        uniLightDirect = Au::GFX::GetUniform<Au::Math::Vec3f>("LightDirect", lightsDirect.size());
        uniLightDirectRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightDirectRGB", lightsDirect.size());
    }
    
    void AddMesh(Mesh* mesh)
    {
        RemoveMesh(mesh);
        meshes.push_back(mesh);
    }
    void RemoveMesh(Mesh* mesh)
    {
        for(unsigned i = 0; i < meshes.size(); ++i)
        {
            if(meshes[i] == mesh)
            {
                meshes.erase(meshes.begin() + i);
                break;
            }
        }
    }
    
    void SetInt(const std::string& name, int value)
    { _intMap[name] = value; }
    int GetInt(const std::string& name)
    { return _intMap[name]; }
        
    virtual void OnCreate();
private:
    void _renderRebuildScene(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform);
    void _render(const Au::Math::Mat4f& perspective,
        const Au::Math::Mat4f& transform);

    void(Renderer::*renderFn)(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform);

    Au::GFX::Device* _gfxDevice;

    std::vector<Mesh*> meshes;
    std::vector<LightOmni*> lightsOmni;
    std::vector<LightDirect*> lightsDirect;
    
    std::map<std::string, int> _intMap;
    
    Au::Math::Vec3f ambientColor;
    Au::Math::Vec3f rimColor;
    
    Au::GFX::Uniform uniViewMat4f;
    Au::GFX::Uniform uniProjMat4f;
    Au::GFX::Uniform uniLightOmniPos;
    Au::GFX::Uniform uniLightOmniRGB;
    Au::GFX::Uniform uniLightDirect;
    Au::GFX::Uniform uniLightDirectRGB;
    
    Au::GFX::Uniform uniAmbientColor;
    Au::GFX::Uniform uniRimColor;
};

#endif
