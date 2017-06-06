#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/gfx.h>
#include <aurora/math.h>
#include <aurora/transform.h>
#include "../scene_object.h"

class Mesh;
class Skeleton;
class LightOmni;
class LightDirect;
class Renderer : public SceneObject::Component
{
friend Mesh;
public:
    Renderer();

    bool Init(Au::GFX::Device* gfxDevice);
    
    Au::GFX::Device* GetDevice() { return _gfxDevice; }

    void Dirty() { renderFn = &Renderer::_renderRebuildScene; }
    
    void Render(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform);
        
    void AmbientColor(float r, float g, float b)
    { 
        ambientColor = Au::Math::Vec3f(r, g, b);
        glClearColor (r, g, b, 1.0f);        
    }
    void RimColor(float r, float g, float b)
    { rimColor = Au::Math::Vec3f(r, g, b); }

    void AddLightOmni(LightOmni* light);
    void RemoveLightOmni(LightOmni* light);
    
    void AddLightDirect(LightDirect* light);
    void RemoveLightDirect(LightDirect* light);
    
    void AddMesh(Mesh* mesh);
    void RemoveMesh(Mesh* mesh);
    
    void AddSkeleton(Skeleton* skel);
    void RemoveSkeleton(Skeleton* skel);
    
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
    std::vector<Skeleton*> skeletons;
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
