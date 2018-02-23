#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/gfx.h>
#include <aurora/math.h>
#include <aurora/transform.h>
#include "../scene_object.h"
#include "../frame_stage.h"

class SolidMesh;
class LightOmni;
class LightDirect;
class Camera;
class TextMesh;
class Renderer : public SceneObject::Component
{
friend SolidMesh;
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
        
    virtual void OnCreate();
    
    template<typename T>
    T* GetStage()
    {
        _gfxDevice->SetContextCurrent();
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
    
    Au::Math::Vec3f ambientColor;
    Au::Math::Vec3f rimColor;
};

#endif
