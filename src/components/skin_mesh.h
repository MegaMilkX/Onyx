#ifndef COMPONENT_SKIN_MESH_H
#define COMPONENT_SKIN_MESH_H

#include "../scene_object.h"
#include "transform.h"
#include "renderer.h"
#include <material.h>

#include <mesh_data.h>

#include "../frame_stage_skin.h"

#undef GetObject

class SkinMesh : public SceneObject::Component
{
public:
    SkinMesh()
    : meshData(0), material(0)
    {}
    void SetMeshData(const std::string& name)
    {
        SetMeshData(Resource<MeshData>::Get(name));
    }
    void SetMeshData(MeshData* md) { meshData = md; }
    void SetMaterial(const std::string& name)
    {
        SetMaterial(Resource<Material>::Get(name));
    }
    void SetMaterial(Material* mat) { material = mat; }
    
    MeshData* GetMeshData() { return meshData; }
    Material* GetMaterial() { return material; }
    
    void OnCreate()
    {
        Renderer* renderer = GetObject()->Root()->GetComponent<Renderer>();
        renderer->GetStage<FrameStageSkin>();
    }
private:
    MeshData* meshData;
    Material* material;
};

#endif
