#ifndef COMPONENT_CAMERA_H
#define COMPONENT_CAMERA_H

#include <aurora/gfx.h>

#include "transform.h"
#include "../scene_object.h"
#include "gfxscene.h"

class Camera : public SceneObject::Component
{
public:
    void Render(Au::GFX::Device* device)
    {
        gfxScene->Render(
            device,
            perspective,
            transform->GetTransform()
        );
    }

    virtual void OnCreate()
    {
        transform = GetParentObject()->GetComponent<Transform>();
        gfxScene = GetParentObject()->Root()->GetComponent<GFXScene>();
    }
private:
    Au::Math::Mat4f perspective;

    Transform* transform;
    GFXScene* gfxScene;
};

#endif
