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
    SetInt("LIGHT_OMNI_COUNT", 1);
    SetInt("LIGHT_DIRECT_COUNT", 1);
    SetInt("MAX_BONE_COUNT", 1);
    return true;
}

void Renderer::AddLightOmni(LightOmni* light)
{
    RemoveLightOmni(light);
    lightsOmni.push_back(light);
    SetInt("LIGHT_OMNI_COUNT", lightsOmni.size());
    uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos", lightsOmni.size());
    uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB", lightsOmni.size());
}

void Renderer::RemoveLightOmni(LightOmni* light)
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

void Renderer::AddLightDirect(LightDirect* light)
{
    RemoveLightDirect(light);
    lightsDirect.push_back(light);
    SetInt("LIGHT_DIRECT_COUNT", lightsDirect.size());
    uniLightDirect = Au::GFX::GetUniform<Au::Math::Vec3f>("LightDirect", lightsDirect.size());
    uniLightDirectRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightDirectRGB", lightsDirect.size());
}

void Renderer::RemoveLightDirect(LightDirect* light)
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

void Renderer::AddMesh(Mesh* mesh)
{
    RemoveMesh(mesh);
    meshes.push_back(mesh);
    
    Dirty();
}

void Renderer::RemoveMesh(Mesh* mesh)
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

void Renderer::AddSkeleton(Skeleton* skel)
{
    RemoveSkeleton(skel);
    RemoveMesh(skel->GetObject()->GetComponent<Mesh>());
    skeletons.push_back(skel);
}

void Renderer::RemoveSkeleton(Skeleton* skel)
{
    for(unsigned i = 0; i < skeletons.size(); ++i)
    {
        if(skeletons[i] == skel)
        {
            skeletons.erase(skeletons.begin() + i);
            break;
        }
    }
}

void Renderer::AddTextMesh(TextMesh* textMesh)
{
    textMeshes.insert(textMesh);
}
void Renderer::RemoveTextMesh(TextMesh* textMesh)
{
    textMeshes.erase(textMesh);
}

void Renderer::OnCreate()
{
    GetObject()->GetComponent<Camera>();
    
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
    for(unsigned i = 0; i < meshes.size(); ++i)
    {
        meshes[i]->Build();
    }
    
    for(unsigned i = 0; i < skeletons.size(); ++i)
    {
        skeletons[i]->GetObject()->GetComponent<Mesh>()->Build();
    }
    
    renderFn = &Renderer::_render;
}

void Renderer::_render(
    const Au::Math::Mat4f& projection,
    const Au::Math::Mat4f& transform)
{
    for(unsigned i = 0; i < lightsOmni.size(); ++i)
    {
        uniLightOmniRGB[i] = lightsOmni[i]->Color();
        uniLightOmniPos[i] = lightsOmni[i]->GetObject()->GetComponent<Transform>()->WorldPosition();
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
    
    for(unsigned i = 0; i < skeletons.size(); ++i)
    {
        skeletons[i]->Update();
        skeletons[i]->Bind();
        skeletons[i]->GetObject()->GetComponent<Mesh>()->Render(_gfxDevice);
    }
    
    for(TextMesh* tm : textMeshes)
    {
        tm->Render(_gfxDevice);
    }
}
