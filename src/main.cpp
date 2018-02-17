#include "state_test.h"
#include "state_gameplay.h"

typedef Au::Math::Vec4f vec4;
typedef Au::Math::Vec3f vec3;
typedef Au::Math::Vec2f vec2;
typedef Au::Math::Mat4f mat4;
typedef Au::Math::Mat3f mat3;

#define REG_COMP(NAME) SceneObject::RegisterComponent<NAME>(#NAME)

int main()
{
    Resource<MeshData>::AddSearchPath("data\\model");
    Resource<MeshData>::AddReader<MeshReaderFBX>("fbx");
    
    Resource<ScriptData>::AddSearchPath("data\\script");
    Resource<ScriptData>::AddReader<ScriptReaderLUA>("lua");
    
    Resource<Material>::AddSearchPath("data\\material");
    Resource<Material>::AddReader<MaterialReaderJSON>("json");
    
    Resource<AnimData>::AddSearchPath("data\\anim");
    Resource<AnimData>::AddReader<AnimDataReaderFBX>("fbx");
    
    Resource<SkeletonData>::AddSearchPath("data\\model");
    Resource<SkeletonData>::AddReader<SkeletonDataReaderFBX>("fbx");
    
    Resource<Texture2D>::AddSearchPath("data\\texture");
    Resource<Texture2D>::AddReader<Texture2DReaderPNG>("png");
    Resource<Texture2D>::AddReader<Texture2DReaderJPG>("jpg");
    
    Resource<SoundClip>::AddSearchPath("data\\audio");
    Resource<SoundClip>::AddReader<SoundClipReaderOGG>("ogg");
    
    Resource<FontData>::AddSearchPath("data\\fonts");
    Resource<FontData>::AddReader<FontDataReader>("ttf");
    Resource<FontData>::AddReader<FontDataReader>("otf");
    
    REG_COMP(Transform);
    REG_COMP(Mesh);
    REG_COMP(Skeleton);
    REG_COMP(Renderer);
    REG_COMP(LuaScript);
    REG_COMP(LightOmni);
    REG_COMP(Camera);
    REG_COMP(Animation);
    REG_COMP(SoundEmitter);
    
    GameState::Init();
    GameState::Push<Gameplay>();
    while(GameState::Update());
    GameState::Cleanup();
    return 0;
}