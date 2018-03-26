#include "renderer.h"

#include "model.h"
#include "skeleton.h"
#include "material.h"
#include "light_omni.h"
#include "camera.h"
#include "text_mesh.h"

Renderer::Renderer()
{
}

bool Renderer::Init(Au::GFX::Device* gfxDevice)
{
    _gfxDevice = gfxDevice;
    return true;
}

void Renderer::OnCreate()
{
    GetObject()->GetComponent<Camera>();
}

#include <iostream>
void Renderer::Render()
{
    _currentCamera->Render(_gfxDevice);
}

void Renderer::Render(const Au::Math::Mat4f& projection,
        const Au::Math::Mat4f& transform)
{
    frameGraph.set_data(
        FrameCommon{
            projection,
            Au::Math::Inverse(transform),
            transform[3],
            1920, 1080,
            GetObject() 
        }
    );
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 1920, 1080);
    frameGraph.run();
}
