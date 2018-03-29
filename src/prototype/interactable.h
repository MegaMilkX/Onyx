#ifndef INTERACTABLE_H
#define INTERACTABLE_H

#include <scene_object.h>
#include <collision/collider.h>
#include "trigger.h"

class Interactable : public SceneObject::Component
{
public:
    void Activate()
    {
        trig->Activate();
    }

    void OnCreate()
    {
        trig = Get<Trigger>();
        collider = Get<SphereCollider>();
    }
private:
    SphereCollider* collider;
    Trigger* trig;
};

#endif
