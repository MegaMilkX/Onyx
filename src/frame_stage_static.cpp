#include "frame_stage_static.h"

#include "components/mesh.h"
#include "components/camera.h"

void FrameStageStatic::Build(SceneObject* root)
{
    renderer = root->GetComponent<Renderer>();
    
    std::vector<Mesh*> meshes = root->FindAllOf<Mesh>();
    for(Mesh* mesh : meshes)
    {
        if(!mesh->GetMeshData())
            continue;
        RenderUnit unit;
        unit.Init();
        unit.transform = mesh->GetComponent<Transform>();
        unit.FillMesh(mesh->GetMeshData());
        unit.diffuse = new Au::GFX::Texture2D();
        Resource<Texture2D>::Get("test")->Fill(unit.diffuse);
        units.push_back(unit);
    }
}

void FrameStageStatic::Run()
{
    glUseProgram(shaderProgram);
    
    Camera* cam = renderer->CurrentCamera();
    
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "MatrixProjection"), 
        1, 
        GL_FALSE, 
        (float*)&cam->Projection()
    );
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "MatrixView"), 
        1, 
        GL_FALSE, 
        (float*)&cam->InverseTransform()
    );
    
    for(RenderUnit& unit : units)
    {
        unit.Bind();
        if(unit.diffuse)
            unit.diffuse->Bind(0);
        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "MatrixModel"), 
            1, 
            GL_FALSE, 
            (float*)&unit.transform->GetTransform()
        );
        glDrawElements(GL_TRIANGLES, unit.indexCount, GL_UNSIGNED_SHORT, (void*)unit.offset);
    }
}