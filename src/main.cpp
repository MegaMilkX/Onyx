#include "state_test.h"
#include "state_gameplay.h"

typedef Au::Math::Vec4f vec4;
typedef Au::Math::Vec3f vec3;
typedef Au::Math::Vec2f vec2;
typedef Au::Math::Mat4f mat4;
typedef Au::Math::Mat3f mat3;

int main()
{
    Resource<MeshData>::AddSearchPath("data");
    Resource<MeshData>::AddReader<MeshReaderFBX>("fbx");
    
    Resource<ScriptData>::AddSearchPath("data");
    Resource<ScriptData>::AddReader<ScriptReaderLUA>("lua");
    
    Resource<Material>::AddSearchPath("data");
    Resource<Material>::AddReader<MaterialReaderLUA>("lua");
    
    Resource<AnimData>::AddSearchPath("data");
    Resource<AnimData>::AddReader<AnimDataReaderFBX>("fbx");
    
    GameState::Init();
    GameState::Push<Gameplay>();
    while(GameState::Update());
    GameState::Cleanup();
    return 0;
}