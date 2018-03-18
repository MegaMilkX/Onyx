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
        //Get<Quad>()->SetColor(0.4f, 0.0f, 0.0f, 1.0f);
    }
    void OnMouseLeave()
    {
        //Get<Quad>()->SetColor(0.4f, 0.4f, 0.4f, 1.0f);
    }

    void OnCreate()
    {
        Get<GuiListener>()->onMouseEnter = std::bind(&GuiBox::OnMouseEnter, this);
        Get<GuiListener>()->onMouseLeave = std::bind(&GuiBox::OnMouseLeave, this);

        SceneObject* titleBar = Object()->CreateObject();
        titleBar->Get<Quad>()->quad.height = 25;
        titleBar->Get<Quad>()->quad.width = 300;
        titleBar->Get<Quad>()->quad.color = { 0.4f, 0.4f, 0.4f, 1.0f };
        titleBar->Get<Transform>()->AttachTo(Get<Transform>());

        SceneObject* clientArea = Object()->CreateObject();
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
};

#endif
