#ifndef QUAD_H
#define QUAD_H

#include <scene_object.h>
#include <asset.h>
#include <texture2d.h>

class OverlayRoot;
class Quad : public SceneObject::Component
{
public:
    float width = 100, height = 100;
    asset<Texture2D> image;

    void OnCreate()
    {
        GetObject()->Root()->GetComponent<OverlayRoot>();
    }
};

#endif
