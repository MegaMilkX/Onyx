#include "renderer.h"

#include "mesh.h"
#include "material.h"
#include "light_omni.h"

void Renderer::OnCreate()
{
    uniViewMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixView");
    uniProjMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixProjection");
    
    uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos");
    uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB");
    
    uniLightDirect = Au::GFX::GetUniform<Au::Math::Vec3f>("LightDirect");
    uniLightDirectRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightDirectRGB");

    uniAmbientColor = Au::GFX::GetUniform<Au::Math::Vec3f>("UniformAmbientColor");
    uniRimColor = Au::GFX::GetUniform<Au::Math::Vec3f>("RimColor");
}

#include <iostream>
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
    
    
    renderFn = &Renderer::_render;
}

void Renderer::_render(
    const Au::Math::Mat4f& projection,
    const Au::Math::Mat4f& transform)
{
    for(unsigned i = 0; i < lightsOmni.size(); ++i)
    {
        uniLightOmniRGB[i] = lightsOmni[i]->Color();
        uniLightOmniPos[i] = lightsOmni[i]->GetObject()->GetComponent<Transform>()->Position();
    }
    
    for(unsigned i = 0; i < lightsDirect.size(); ++i)
    {
        uniLightDirectRGB[i] = lightsDirect[i]->Color();
        uniLightDirect[i] = lightsDirect[i]->Direction();
    }
    
    uniAmbientColor = ambientColor;
    uniRimColor = rimColor;
    
    uniViewMat4f = Au::Math::Inverse(transform);
    uniProjMat4f = projection;
    
    for(unsigned i = 0; i < meshes.size(); ++i)
    {
        meshes[i]->Render(_gfxDevice);
    }
}
