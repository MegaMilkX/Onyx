#ifndef COMPONENT_LIGHT_OMNI_H
#define COMPONENT_LIGHT_OMNI_H

#include "../scene_object.h"

#include <aurora/math.h>

class LightOmni : public SceneObject::Component
{
public:
    void Color(float r, float g, float b)
    {
        color = Au::Math::Vec3f(r, g, b);
    }
    void Intensity(float i)
    {
        intensity = i;
    }

    void OnCreate()
    {
    }
private:
    Au::Math::Vec3f color;
    float intensity;
};

#endif
