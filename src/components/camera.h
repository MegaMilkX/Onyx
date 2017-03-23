#ifndef COMPONENT_CAMERA_H
#define COMPONENT_CAMERA_H

#include <aurora/gfx.h>

#include "transform.h"
#include "../scene_object.h"
#include "gfxscene.h"

class Camera : public SceneObject::Component
{
public:
    Camera()
    {
        Perspective(1.6f, 16.0f/9.0f, 0.1f, 100.0f);
    }
    
    void Ortho(){}
    void Perspective(float fov, float aspect, float zNear, float zFar)
    {
        projection = Au::Math::Perspective(fov, aspect, zNear, zFar);
    }

    void Render(Au::GFX::Device* device)
    {
        gfxScene->Render(
            device,
            projection,
            transform->GetTransform()
        );
    }

    virtual void OnCreate()
    {
        transform = GetParentObject()->GetComponent<Transform>();
        gfxScene = GetParentObject()->Root()->GetComponent<GFXScene>();
    }
private:
    Au::Math::Mat4f projection;

    Transform* transform;
    GFXScene* gfxScene;
};

#endif