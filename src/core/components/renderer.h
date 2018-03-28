#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/gfx.h>
#include <aurora/math.h>
#include <aurora/transform.h>
#include "../scene_object.h"

#include "../lib/task_graph.h"

#undef GetObject

struct FrameCommon
{
    Au::Math::Mat4f projection;
    Au::Math::Mat4f view;
    Au::Math::Vec3f viewPos;
    float width, height;
    SceneObject* scene;
};

class Camera;
class Renderer : public SceneObject::Component
{
public:
    Renderer();

    bool Init(Au::GFX::Device* gfxDevice);
    
    task_graph::graph& GetFrameGraph() { return frameGraph; }
    
    Au::GFX::Device* GetDevice() { return _gfxDevice; }
    
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
private:
    task_graph::graph frameGraph;

    Au::GFX::Device* _gfxDevice;
    
    Camera* _currentCamera;
    
    Au::Math::Vec3f ambientColor;
    Au::Math::Vec3f rimColor;
};

#endif
