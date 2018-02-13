#include "frame_stage.h"

#include "components/mesh.h"

#define STAGE_PRIORITY_STATIC_GEOMETRY 0

class FrameStageStatic : public FrameStage
{
public:
    virtual int Priority() { return STAGE_PRIORITY_STATIC_GEOMETRY; }
    virtual std::string VertexShader()
    {
        return R"(#version 330
            
        )";
    }
    virtual std::string PixelShader()
    {
        return R"(#version 330
            
        )";
    }
    virtual void Build(SceneObject* root)
    {
        glBindAttribLocation(shaderProgram, 0, "Position");
        glBindAttribLocation(shaderProgram, 1, "UV");
        glBindAttribLocation(shaderProgram, 2, "Normal");
        
        glBindFragDataLocation(program, 0, "outDiffuse");
        
        // Traverse scene, find all static meshes,
        // lights and stuff
        meshes = root->FindAllOf<Mesh>();
    }
    virtual void Run()
    {
        glUseProgram(shaderProgram);
        for(Mesh* mesh : meshes)
        {
            
        }
    }
private:
    std::vector<Mesh*> meshes;
};