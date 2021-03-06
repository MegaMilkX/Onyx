#ifndef GUI_LISTENER_H
#define GUI_LISTENER_H

#include <scene_object.h>
#include "gui_root.h"

class GuiListener : public SceneObject::Component
{
public:
    void OnCreate()
    {
        Object()->Root()->Get<GuiRoot>();
    }
};

#endif
