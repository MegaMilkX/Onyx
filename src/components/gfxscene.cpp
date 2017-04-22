#include "gfxscene.h"

#include "mesh.h"
#include "material.h"
#include "light_omni.h"

void GFXScene::OnCreate()
{
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
    
    uniViewMat4f = Au::Math::Inverse(transform);
    uniProjMat4f = projection;
    
    for(unsigned i = 0; i < meshes.size(); ++i)
    {
        meshes[i]->Render(device);
    }
}