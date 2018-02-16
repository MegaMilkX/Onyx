#include "frame_stage_static.h"

#include "components/mesh.h"
#include "components/camera.h"
#include "components/light_omni.h"

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
    
    lightsOmni = root->FindAllOf<LightOmni>();
    lightsDirect = root->FindAllOf<LightDirect>();
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
    
    for(unsigned i = 0; i < lightsDirect.size(); ++i)
    {
        LightDirect* l = lightsDirect[i];
        Au::Math::Vec3f col = l->Color();
        Au::Math::Vec3f dir = l->Direction();
        glUniform3f(
            glGetUniformLocation(shaderProgram, (std::string("LightDirectRGB[") + std::to_string(i) + "]").c_str()),
            col.x, col.y, col.z
        );
        glUniform3f(
            glGetUniformLocation(shaderProgram, ("LightDirect[" + std::to_string(i) + "]").c_str()),
            dir.x, dir.y, dir.z
        );
    }
    
    for(unsigned i = 0; i < lightsOmni.size(); ++i)
    {
        LightOmni* l = lightsOmni[i];
        Au::Math::Vec3f col = l->Color();
        Au::Math::Vec3f pos = l->GetComponent<Transform>()->Position();
        glUniform3f(
            glGetUniformLocation(shaderProgram, ("LightOmniRGB[" + std::to_string(i) + "]").c_str()),
            col.x, col.y, col.z
        );
        glUniform3f(
            glGetUniformLocation(shaderProgram, ("LightOmniPos[" + std::to_string(i) + "]").c_str()),
            pos.x, pos.y, pos.z
        );
    }
    
    glUniform3f(
        glGetUniformLocation(shaderProgram, "AmbientColor"),
        0.4f, 0.3f, 0.2f
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
        unit.vao.DrawElements(GL_TRIANGLES);
    }
}