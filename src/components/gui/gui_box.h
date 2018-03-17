#ifndef GUI_BOX_H
#define GUI_BOX_H

#include "gui_root.h"
#include "../overlay/quad.h"

class GuiBox : public GuiListenerBase
{
public:
    void OnMouseEnter(const eMouseMove* e)
    {
        Get<Quad>()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    }
    void OnMouseLeave()
    {
        Get<Quad>()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    }

    ~GuiBox()
    {
        Object()->Root()->Get<GuiRoot>()->RemoveListener(this);
    }
    void OnCreate()
    {
        GuiListenerBase::OnCreate();

        Object()->Root()->Get<GuiRoot>()->AddListener(this);
        Quad* q = Get<Quad>();
        q->SetImage("25544144");
        q->SetSize(100, 100);
    }
};

#endif
