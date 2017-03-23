#include "gfxscene.h"

#include "mesh.h"

void GFXScene::OnCreate()
{
    uniModelMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixModel");
    uniViewMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixView");
    uniProjMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixProjection");
    
    uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos", 3);
    uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB", 3);
    uniLightOmniPos.Set(Au::Math::Vec3f(0.0f, 1.5f, 2.5f), 0);
    uniLightOmniRGB.Set(Au::Math::Vec3f(0.8f, 0.6f, 0.2f), 0);
    uniLightOmniPos.Set(Au::Math::Vec3f(4.0f, 0.0f, 0.0f), 1);
    uniLightOmniRGB.Set(Au::Math::Vec3f(0.6f, 0.8f, 0.2f), 1);
    uniLightOmniPos.Set(Au::Math::Vec3f(-4.0f, 0.0f, 0.0f), 2);
    uniLightOmniRGB.Set(Au::Math::Vec3f(0.8f, 0.6f, 0.8f), 2);
    
    
}

void GFXScene::AddMesh(Mesh* mesh)
{
    meshes.push_back(mesh);
}

void GFXScene::AddRenderUnit(GFXRenderUnit renderUnit)
{
    renderUnits.push_back(renderUnit);
}

void GFXScene::Render(Au::GFX::Device* device, const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform)
{
    for(unsigned i = 0; i < renderUnits.size(); ++i)
    {
        device->Bind(renderUnits[i].renderState);
        device->Set(uniModelMat4f, renderUnits[i].transform->GetTransform());
        device->Set(uniViewMat4f, Au::Math::Inverse(transform));
        device->Set(uniProjMat4f, projection);
        device->Bind(renderUnits[i].mesh);
        device->Render();
    }
}