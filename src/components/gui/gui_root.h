#ifndef GUI_ROOT_H
#define GUI_ROOT_H

#include <scene_object.h>
#include <lib/event.h>
#include <game_state.h>
#include <transform.h>
#include <functional>

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
    virtual void OnMouseDown(const eMouseDown* e) {}
    virtual void OnMouseUp(const eMouseUp* e) {}
    virtual void OnMouseMove(const eMouseMove* e) {}
    virtual void OnMouseEnter() {}
    virtual void OnMouseLeave() {}
    void ProcMouseMove(const eMouseMove* e)
    {
        Au::Math::Vec3f pos = transform->WorldPosition();
        if(e->x > pos.x && 
            e->x < pos.x + bbox.z &&
            e->y > pos.y &&
            e->y < pos.y + bbox.w)
        {
            OnMouseMove(e);
            if(!mouseOver)
            {
                OnMouseEnter();
                mouseOver = true;
            }
        }
        else
        {
            mouseOver = false;
            OnMouseLeave();
        }
    }
    void ProcMouseDown(const eMouseDown* e)
    {
        if(mouseOver) OnMouseDown(e);
    }
    void ProcMouseUp(const eMouseUp* e)
    {
        OnMouseUp(e);
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
            for(auto l : listeners)
                l->ProcMouseDown(e);
        }
        while(auto e = dispMouseUp.poll())
        {
            for(auto l : listeners)
                l->ProcMouseUp(e);
        }
    }
    void AddListener(GuiListenerBase* l) { listeners.insert(l); }
    void RemoveListener(GuiListenerBase* l) { listeners.erase(l); }
private:
    std::set<GuiListenerBase*> listeners;
    event_dispatcher<eMouseDown> dispMouseDown;
    event_dispatcher<eMouseUp> dispMouseUp;
    event_dispatcher<eMouseMove> dispMouseMove;
};

class GuiListener : public GuiListenerBase
{
public:
    std::function<void(const eMouseDown*)> onMouseDown;
    std::function<void(const eMouseUp*)> onMouseUp;
    std::function<void(const eMouseMove*)> onMouseMove;
    std::function<void(void)> onMouseEnter;
    std::function<void(void)> onMouseLeave;

    ~GuiListener()
    {
        Object()->Root()->Get<GuiRoot>()->RemoveListener(this);
    }

    void OnCreate()
    {
        GuiListenerBase::OnCreate();
        Object()->Root()->Get<GuiRoot>()->AddListener(this);
    }
private:
    virtual void OnMouseDown(const eMouseDown* e) { if(onMouseDown) onMouseDown(e); }
    virtual void OnMouseUp(const eMouseUp* e) { if(onMouseUp) onMouseUp(e); }
    virtual void OnMouseMove(const eMouseMove* e) { if(onMouseMove) onMouseMove(e); }
    virtual void OnMouseEnter() { if(onMouseEnter) onMouseEnter(); }
    virtual void OnMouseLeave() { if(onMouseLeave) onMouseLeave(); }
};

#endif
