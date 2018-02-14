#include "renderer.h"

#include "mesh.h"
#include "skeleton.h"
#include "material.h"
#include "light_omni.h"
#include "camera.h"
#include "text_mesh.h"

Renderer::Renderer()
{
    Dirty();
}

bool Renderer::Init(Au::GFX::Device* gfxDevice)
{
    _gfxDevice = gfxDevice;
    return true;
}

void Renderer::OnCreate()
{
    GetObject()->GetComponent<Camera>();
}

#include <iostream>
void Renderer::Render()
{
    _currentCamera->Render(_gfxDevice);
}

void Renderer::Render(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform)
{
    (this->*renderFn)(projection, transform);
}

// ===================
// Private
// ===================

void Renderer::_renderRebuildScene(
    const Au::Math::Mat4f& projection,
    const Au::Math::Mat4f& transform)
{
    //std::sort(frameStages.begin(), frameStages.end());
    for(auto& kv : frameStages)
    {
        kv.second->Build(GetObject());
    }
    
    renderFn = &Renderer::_render;
}

void Renderer::_render(
    const Au::Math::Mat4f& projection,
    const Au::Math::Mat4f& transform)
{
    for(auto& kv : frameStages)
    {
        kv.second->Run();
    }
}
