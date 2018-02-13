#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/gfx.h>
#include <aurora/math.h>
#include <aurora/transform.h>
#include "../scene_object.h"
#include "../frame_stage.h"

class Mesh;
class Skeleton;
class LightOmni;
class LightDirect;
class Camera;
class TextMesh;
class Renderer : public SceneObject::Component
{
friend Mesh;
public:
    Renderer();

    bool Init(Au::GFX::Device* gfxDevice);
    
    Au::GFX::Device* GetDevice() { return _gfxDevice; }

    void Dirty() { renderFn = &Renderer::_renderRebuildScene; }
    
    void Render();
    void Render(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform);
    
    void CurrentCamera(Camera* cam) { _currentCamera = cam; }
    Camera* CurrentCamera() { return _currentCamera; }
    
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
    
    void AddTextMesh(TextMesh* textMesh);
    void RemoveTextMesh(TextMesh* textMesh);
    
    void SetInt(const std::string& name, int value)
    { _intMap[name] = value; }
    int GetInt(const std::string& name)
    { return _intMap[name]; }
        
    virtual void OnCreate();
    
    template<typename T>
    T* GetStage()
    {
        T* stage = FindStage<T>();
        if (!stage)
        {
            stage = new T();
            stage->Init();
            frameStages.insert(std::make_pair(TypeInfo<T>::Index(), stage));
            return stage;
        }
        else
            return stage;
    }
    
    template<typename T>
    T* FindStage()
    {
        std::map<typeindex, FrameStage*>::iterator it;
        it = frameStages.find(TypeInfo<T>::Index());
        if(it == frameStages.end())
            return 0;
        else
            return (T*)it->second;
    }
private:
    std::map<typeindex, FrameStage*> frameStages;

    void _renderRebuildScene(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform);
    void _render(const Au::Math::Mat4f& perspective,
        const Au::Math::Mat4f& transform);

    void(Renderer::*renderFn)(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform);

    Au::GFX::Device* _gfxDevice;
    
    Camera* _currentCamera;

    std::vector<Mesh*> meshes;
    std::vector<Skeleton*> skeletons;
    std::vector<LightOmni*> lightsOmni;
    std::vector<LightDirect*> lightsDirect;
    std::set<TextMesh*> textMeshes;
    
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
