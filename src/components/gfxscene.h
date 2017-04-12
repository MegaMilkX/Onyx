#ifndef COMPONENT_GFXSCENE_H
#define COMPONENT_GFXSCENE_H

#include <vector>

#include <aurora/gfx.h>
#include <aurora/math.h>
#include <aurora/transform.h>
#include "../scene_object.h"

class Mesh;
class LightOmni;
class GFXScene : public SceneObject::Component
{
friend Mesh;
public:
    bool Init(Au::GFX::Device* gfxDevice)
    {
        _gfxDevice = gfxDevice;        
        return true;
    }

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
    
    void AddMesh(Mesh* mesh)
    {
        RemoveMesh(mesh);
        meshes.push_back(mesh);
    }
    void RemoveMesh(Mesh* mesh)
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
        
    virtual void OnCreate();
private:
    Au::GFX::Device* _gfxDevice;

    std::vector<Mesh*> meshes;
    std::vector<LightOmni*> lightsOmni;
    
    Au::GFX::Uniform uniModelMat4f;
    Au::GFX::Uniform uniViewMat4f;
    Au::GFX::Uniform uniProjMat4f;
    Au::GFX::Uniform uniLightOmniPos;
    Au::GFX::Uniform uniLightOmniRGB;
};

#endif
