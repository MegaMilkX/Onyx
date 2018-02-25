#include "model.h"

Model::Model()
: dirty(true),
transform(0),
renderer(0)
{
}

Model::~Model()
{
    
}

void Model::OnCreate()
{
    static std::once_flag once_flag;
    std::call_once(
        once_flag,
        [](){
            std::cout << "Called once!" << std::endl;
        }
    );

    transform = GetObject()->GetComponent<Transform>();
    renderer = GetObject()->Root()->GetComponent<Renderer>();
    
    task_graph::graph& fg = renderer->GetFrameGraph();
    fg += SolidMeshDraw;
    fg += task_graph::once(ShaderSolidInit);
    fg += task_graph::once(ShaderSkinInit);
    fg += task_graph::once(SolidMeshDrawInitUnits);
    fg.reset_once_flag(SolidMeshDrawInitUnits);
}
std::string Model::Serialize() 
{
    using json = nlohmann::json;
    json j = json::object();
    j["Material"] = materialName;
    j["Mesh"] = meshName;
    j["SubMesh"] = subMeshName;
    return j.dump(); 
}
void Model::Deserialize(const std::string& data) 
{
    using json = nlohmann::json;
    json j = json::parse(data);
    if(j.is_null())
        return;
    if(j["Mesh"].is_string())
    {
        mesh.set(j["Mesh"].get<std::string>());
    }
    if(j["SubMesh"].is_string())
    {
        mesh.set(j["SubMesh"].get<std::string>());
    }
    if(j["Material"].is_string())
    {
        mesh.set(j["Material"].get<std::string>());
    }
}
