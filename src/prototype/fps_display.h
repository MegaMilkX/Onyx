#ifndef FPS_DISPLAY_H
#define FPS_DISPLAY_H

#include <game_state.h>
#include <overlay/overlay_root.h>

class FpsDisplay : public SceneObject::Component
{
public:
    void Update(float dt)
    {
        float fps = 1.0f / dt;
        std::string s = std::to_string((int)(fps + prevFps * 0.5f));
        text->SetText(
            std::string("FPS: ") + s +
            "\n" +
            "Frame: " + std::to_string(GameState::FrameCount()));
        prevFps = fps;
    }

    void OnCreate()
    {
        text = GetComponent<Text2d>();
        text->SetSize(16);
        text->Get<Transform>()->Position(0.0f, 0.0f, 0.0f);
    }
private:
    Text2d* text;
    float prevFps = 60;
};

#endif
