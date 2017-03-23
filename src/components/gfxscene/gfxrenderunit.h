#ifndef GFX_RENDER_UNIT_H
#define GFX_RENDER_UNIT_H

#include <aurora/gfx.h>
#include "../transform.h"

struct GFXRenderUnit
{
    Au::GFX::RenderState* renderState;
    Au::GFX::Mesh* mesh;
    Transform* transform;
};

#endif
