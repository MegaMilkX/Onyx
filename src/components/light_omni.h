#ifndef COMPONENT_LIGHT_OMNI_H
#define COMPONENT_LIGHT_OMNI_H

#include "../scene_object.h"
#include "renderer.h"

#include <aurora/math.h>

class LightDirect : public SceneObject::Component
{
public:
    ~LightDirect()
    {
        GetObject()->Root()->GetComponent<Renderer>()->RemoveLightDirect(this);
    }
    
    void Color(float r, float g, float b)
    { color = Au::Math::Vec3f(r, g, b); }
    
    Au::Math::Vec3f Color()
    { return color; }
    
    void Direction(float x, float y, float z)
    { direction = Au::Math::Vec3f(x, y, z); }
    Au::Math::Vec3f Direction()
    { return direction; }
    
    void OnCreate()
    {
        GetObject()->Root()->GetComponent<Renderer>()->AddLightDirect(this);
    }
private:
    Au::Math::Vec3f color;
    Au::Math::Vec3f direction;
};

class LightOmni : public SceneObject::Component
{
public:
    ~LightOmni()
    {
        GetObject()->Root()->GetComponent<Renderer>()->RemoveLightOmni(this);
    }

    void Color(float r, float g, float b)
    {
        color = Au::Math::Vec3f(r, g, b);
    }
    void Intensity(float i)
    {
        intensity = i;
    }
    
    Au::Math::Vec3f Color()
    { return color; }

    void OnCreate()
    {
        GetObject()->Root()->GetComponent<Renderer>()->AddLightOmni(this);
    }
private:
    Au::Math::Vec3f color;
    float intensity;
};

#endif
