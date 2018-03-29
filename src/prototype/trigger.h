#ifndef TRIGGER_H
#define TRIGGER_H

#include <scene_object.h>

class Trigger : public SceneObject::Component
{
public:
    Trigger()
    : isTriggered(false) {}

    void Activate()
    {
        isTriggered = true;
    }

    void Reset()
    {
        isTriggered = false;
    }

    bool IsTriggered()
    {
        return isTriggered;
    }
private:
    bool isTriggered;
};

#endif
