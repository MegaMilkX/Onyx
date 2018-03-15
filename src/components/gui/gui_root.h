#ifndef GUI_ROOT_H
#define GUI_ROOT_H

#include <scene_object.h>
#include <lib/event.h>
#include <game_state.h>

class GuiRoot : public SceneObject::Component
{
public:
    void Update()
    {
        while(auto e = dispMouseDown.poll())
        {
            std::cout << "MouseDown: " << e->x << " " << e->y << std::endl;
        }
    }
private:
    event_dispatcher<eMouseDown> dispMouseDown;
};

#endif
