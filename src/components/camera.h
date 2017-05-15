#ifndef COMPONENT_CAMERA_H
#define COMPONENT_CAMERA_H

#include <aurora/gfx.h>

#include "transform.h"
#include "../scene_object.h"
#include "renderer.h"

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
        renderer->Render(
            projection,
            transform->GetTransform()
        );
    }

    virtual void OnCreate()
    {
        transform = GetObject()->GetComponent<Transform>();
        renderer = GetObject()->Root()->GetComponent<Renderer>();
    }
private:
    Au::Math::Mat4f projection;

    Transform* transform;
    Renderer* renderer;
};

#endif
