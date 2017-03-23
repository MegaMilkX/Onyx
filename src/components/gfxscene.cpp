#include "gfxscene.h"

#include "mesh.h"
#include "light_omni.h"

void GFXScene::OnCreate()
{
    uniModelMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixModel");
    uniViewMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixView");
    uniProjMat4f = Au::GFX::GetUniform<Au::Math::Mat4f>("MatrixProjection");
    
    uniLightOmniPos = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniPos", 3);
    uniLightOmniRGB = Au::GFX::GetUniform<Au::Math::Vec3f>("LightOmniRGB", 3);    
}

void GFXScene::AddMesh(Mesh* mesh)
{
    meshes.push_back(mesh);
}

void GFXScene::AddRenderUnit(GFXRenderUnit renderUnit)
{
    renderUnits.push_back(renderUnit);
}
#include <iostream>
void GFXScene::Render(Au::GFX::Device* device, const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform)
{
    for(unsigned j = 0; j < 3 && j < lightsOmni.size(); ++j)
    {
        uniLightOmniRGB.Set(lightsOmni[j]->Color(), j);
        uniLightOmniPos.Set(lightsOmni[j]->GetParentObject()->GetComponent<Transform>()->Position(), j);
    }
    
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