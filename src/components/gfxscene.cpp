#include "gfxscene.h"

#include "mesh.h"
#include "material.h"
#include "light_omni.h"

void GFXScene::OnCreate()
{
    uniModelMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixModel");
    uniViewMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixView");
    uniProjMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixProjection");
    
    uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos");
    uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB");

    uniAmbientColor = Au::GFX::GetUniform<Au::Math::Vec3f>("UniformAmbientColor");
}

#include <iostream>
void GFXScene::Render(Au::GFX::Device* device, const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform)
{
    if(!lightsOmni.empty())
    {
        uniLightOmniRGB = lightsOmni[0]->Color();
        uniLightOmniPos = lightsOmni[0]->GetObject()->GetComponent<Transform>()->Position();
    }
    
    uniAmbientColor = Au::Math::Vec3f(0.1f, 0.1f, 0.1f);
    
    for(unsigned i = 0; i < meshes.size(); ++i)
    {
        meshes[i]->GetObject()->GetComponent<Material>()->Bind(device);
        device->Set(uniModelMat4f, meshes[i]->GetObject()->GetComponent<Transform>()->GetTransform());
        device->Set(uniViewMat4f, Au::Math::Inverse(transform));
        device->Set(uniProjMat4f, projection);
        device->Bind(meshes[i]->mesh);
        device->Render();
    }
}