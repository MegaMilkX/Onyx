#ifndef COMPONENT_TRANSFORM_H
#define COMPONENT_TRANSFORM_H

#include <aurora/math.h>

#include "../scene_object.h"

class Transform : public SceneObject::Component
{
public:
    Au::Math::Mat4f GetTransform()
    {
        return Au::Math::Mat4f(1.0f);
    }

    virtual void OnCreate()
    {
        
    }
};

#endif
