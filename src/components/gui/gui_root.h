#ifndef GUI_ROOT_H
#define GUI_ROOT_H

#include <scene_object.h>
#include <lib/event.h>
#include <game_state.h>
#include <transform.h>

class GuiListenerBase : public SceneObject::Component
{
public:
    GuiListenerBase()
    : bbox(0, 0, 100, 100), 
    mouseOver(false) {}
    Au::Math::Vec4i bbox;
    bool mouseOver;
    Transform* transform;

    virtual ~GuiListenerBase(){}
    virtual void OnMouseDown() {}
    virtual void OnMouseUp() {}
    virtual void OnMouseEnter(const eMouseMove*) {}
    virtual void OnMouseLeave() {}
    void ProcMouseMove(const eMouseMove* e)
    {
        Au::Math::Vec3f pos = transform->WorldPosition();
        if(e->x > pos.x && 
            e->x < pos.x + bbox.z &&
            e->y > pos.y &&
            e->y < pos.y + bbox.w)
        {
            if(!mouseOver)
            {
                OnMouseEnter(e);
                mouseOver = true;
            }
        }
        else
        {
            mouseOver = false;
            OnMouseLeave();
        }
    }

    void OnCreate()
    {
        transform = Get<Transform>();
    }
};

class GuiViewBase
{

};

class GuiRoot : public SceneObject::Component
{
public:
    void Update()
    {
        while(auto e = dispMouseMove.poll())
        {
            for(auto l : listeners)
                l->ProcMouseMove(e);
        }
        while(auto e = dispMouseDown.poll())
        {
            std::cout << "MouseDown: " << e->x << " " << e->y << std::endl;
        }
    }
    void AddListener(GuiListenerBase* l) { listeners.insert(l); }
    void RemoveListener(GuiListenerBase* l) { listeners.erase(l); }
private:
    std::set<GuiListenerBase*> listeners;
    event_dispatcher<eMouseDown> dispMouseDown;
    event_dispatcher<eMouseMove> dispMouseMove;
};

#endif
