#ifndef FRAME_STAGE_H
#define FRAME_STAGE_H

#include <string>
#include "scene_object.h"

class FrameStage
{
public:
    ~FrameStage() {}
    virtual int Priority() = 0;
    virtual bool Init() = 0;
    virtual void Cleanup() = 0;
    virtual void Build(SceneObject* root) = 0;
    virtual void Run() = 0;
private:
    
    
};

#endif
