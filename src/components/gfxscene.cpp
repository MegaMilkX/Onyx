#include "gfxscene.h"

#include "mesh.h"

void GFXScene::OnCreate()
{
    
}

void GFXScene::AddMesh(Mesh* mesh)
{
    meshes.push_back(mesh);
}

void GFXScene::Render(const Au::Math::Mat4f& perspective,
        const Au::Math::Mat4f& transform)
{
    
}