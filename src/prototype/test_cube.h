#ifndef TEST_CUBE_H
#define TEST_CUBE_H

#include <scene_object.h>
#include <transform.h>
#include <model.h>
#include "interactable.h"

class TestCube : public SceneObject::Component
{
public:
    void Update()
    {
        if(trig->IsTriggered())
        {
            trig->Reset();
            Get<Transform>()->Rotate(0.5f, gfxm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    void OnCreate()
    {
        trig = Get<Trigger>();
        Get<Interactable>();
        Model* m = Get<Model>();
        m->mesh.set("cube");
        m->material.set("material1");
    }
private:
    Trigger* trig;
};

#endif