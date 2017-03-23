#ifndef COMPONENT_MESH_H
#define COMPONENT_MESH_H

#include "../scene_object.h"
#include "transform.h"
#include "gfxscene.h"

class Mesh : public SceneObject::Component
{
public:
    virtual void OnCreate()
    {
        transform = GetParentObject()->GetComponent<Transform>();
        gfxScene = GetParentObject()->Root()->GetComponent<GFXScene>();
        gfxScene->AddMesh(this);
    }
private:
    Transform* transform;
    GFXScene* gfxScene;
};

#endif
