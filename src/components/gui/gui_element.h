#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include <aurora/math.h>
#include <scene_object.h>

class GuiElement : public SceneObject::Component
{
public:
    Au::Math::Vec2f dimensions;
    Au::Math::Vec2f center;
    Au::Math::Vec4f margin;
    Au::Math::Vec4f padding;
    int layer;
};

#endif
