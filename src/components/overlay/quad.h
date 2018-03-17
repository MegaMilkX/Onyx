#ifndef QUAD_H
#define QUAD_H

#include <scene_object.h>
#include "../../util/gfx_quad.h"

class OverlayRoot;
class Quad : public SceneObject::Component
{
public:
    GfxQuad quad;

    void SetSize(float width, float height)
    {
        quad.width = width;
        quad.height = height;
    }

    void SetColor(float r, float g, float b, float a)
    {
        quad.color = { r, g, b, a };
    }

    void SetImage(const std::string& name)
    {
        quad.image.set(name);
    }

    void OnCreate()
    {
        Object()->Root()->GetComponent<OverlayRoot>();
    }
};

#endif
