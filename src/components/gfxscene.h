#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/math.h>
#include <aurora/transform.h>
#include "../scene_object.h"

#include "gfxscene/gfxrenderunit.h"

class Mesh;
class LightOmni;
class GFXScene : public SceneObject::Component
{
friend Mesh;
public:
    void AddRenderUnit(GFXRenderUnit renderUnit);

    void Render(Au::GFX::Device* device, 
        const Au::Math::Mat4f& perspective,
        const Au::Math::Mat4f& transform);

    void AddLightOmni(LightOmni* light)
    {
        RemoveLightOmni(light);
        lightsOmni.push_back(light);
    }
    void RemoveLightOmni(LightOmni* light)
    {
        for(unsigned i = 0; i < lightsOmni.size(); ++i)
        {
            if(lightsOmni[i] == light)
            {
                lightsOmni.erase(lightsOmni.begin() + i);
                break;
            }
        }
    }
        
    virtual void OnCreate();
private:
    void AddMesh(Mesh* mesh);
    std::vector<Mesh*> meshes;
    std::vector<LightOmni*> lightsOmni;
    std::vector<GFXRenderUnit> renderUnits;
    
    Au::GFX::Uniform uniModelMat4f;
    Au::GFX::Uniform uniViewMat4f;
    Au::GFX::Uniform uniProjMat4f;
    Au::GFX::Uniform uniLightOmniPos;
    Au::GFX::Uniform uniLightOmniRGB;
};

#endif
