#ifndef GUI_BOX_H
#define GUI_BOX_H

#include "gui_root.h"
#include "../overlay/quad.h"
#include "../overlay/text2d.h"
#include <functional>

class GuiBox : public SceneObject::Component
{
public:
    void OnMouseEnter()
    {
        titleBar->Get<Quad>()->SetColor(0.7f, 0.7f, 0.7f, 1.0f);
    }
    void OnMouseLeave()
    {
        titleBar->Get<Quad>()->SetColor(0.4f, 0.4f, 0.4f, 1.0f);
        drag = false;
    }
    void OnMouseMove(const eMouseMove* e)
    {
        if(drag)
            Get<Transform>()->Translate(e->dx, e->dy, 0.0f);
    }
    void OnMouseDown(const eMouseDown* e)
    {
        if(e->key == Au::Input::KEY_LBUTTON)
        {
            drag = true;
        }
    }
    void OnMouseUp(const eMouseUp* e)
    {
        if(e->key == Au::Input::KEY_LBUTTON)
        {
            drag = false;
        }
    }

    bool drag = false;

    void OnCreate()
    {
        Get<GuiListener>()->onMouseEnter = std::bind(&GuiBox::OnMouseEnter, this);
        Get<GuiListener>()->onMouseLeave = std::bind(&GuiBox::OnMouseLeave, this);
        Get<GuiListener>()->onMouseMove = std::bind(&GuiBox::OnMouseMove, this, std::placeholders::_1);
        Get<GuiListener>()->onMouseDown = std::bind(&GuiBox::OnMouseDown, this, std::placeholders::_1);
        Get<GuiListener>()->onMouseUp = std::bind(&GuiBox::OnMouseUp, this, std::placeholders::_1);

        titleBar = Object()->CreateObject();
        titleBar->Get<Quad>()->quad.height = 25;
        titleBar->Get<Quad>()->quad.width = 300;
        titleBar->Get<Quad>()->quad.color = { 0.4f, 0.4f, 0.4f, 1.0f };
        titleBar->Get<Transform>()->AttachTo(Get<Transform>());

        clientArea = Object()->CreateObject();
        clientArea->Get<Quad>()->quad.height = 500;
        clientArea->Get<Quad>()->quad.width = 300;
        clientArea->Get<Quad>()->quad.color = { 0.2f, 0.2f, 0.2f, 1.0f };
        clientArea->Get<Transform>()->AttachTo(Get<Transform>());
        clientArea->Get<Transform>()->Translate(0.0f, 25.0f, 0.0f);

        SceneObject* title = Object()->CreateObject();
        title->Get<Text2d>()->SetText("Window");
        title->Get<Text2d>()->SetSize(14);
        title->Get<Transform>()->AttachTo(Get<Transform>());
        title->Get<Transform>()->Translate(5.0f, 0.0f, 0.0f);
/*
        Quad* q = Get<Quad>();
        //q->SetImage("25544144");
        q->SetSize(100, 100);
        q->SetColor(0.4f, 0.4f, 0.4f, 1.0f);*/
    }

    SceneObject* titleBar;
    SceneObject* clientArea;
};

#endif
