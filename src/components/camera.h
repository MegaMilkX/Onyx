#ifndef COMPONENT_CAMERA_H
#define COMPONENT_CAMERA_H

#include <aurora/gfx.h>

#include "transform.h"
#include "../object.h"
#include "gfxscene.h"

class Camera : public Object::Component
{
public:
    void Render(Au::GFX::Device* device)
    {
        gfxScene->Render(
            perspective,
            transform->GetTransform()
        );
    }

    virtual void OnCreate()
    {
        transform = GetObject()->GetComponent<Transform>();
        gfxScene = GetObject()->Root()->GetComponent<GFXScene>();
    }
private:
    Au::Math::Mat4f perspective;

    Transform* transform;
    GFXScene* gfxScene;
};

#endif
