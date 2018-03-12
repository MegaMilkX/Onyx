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
    asset<MeshData>::add_search_path("data\\model");
    asset<MeshData>::add_reader<MeshReaderFBX>("fbx");
    asset<ScriptData>::add_search_path("data\\script");
    asset<ScriptData>::add_reader<ScriptReaderLUA>("lua");
    asset<Material>::add_search_path("data\\material");
    asset<Material>::add_reader<MaterialReaderJSON>("json");
    asset<AnimData>::add_search_path("data\\anim");
    asset<AnimData>::add_reader<AnimDataReaderFBX>("fbx");
    asset<SkeletonData>::add_search_path("data\\model");
    asset<SkeletonData>::add_reader<SkeletonDataReaderFBX>("fbx");
    asset<Texture2D>::add_search_path("data\\texture");
    asset<Texture2D>::add_reader<Texture2DReaderPNG>("png");
    asset<Texture2D>::add_reader<Texture2DReaderJPG>("jpg");
    asset<SoundClip>::add_search_path("data\\audio");
    asset<SoundClip>::add_reader<SoundClipReaderOGG>("ogg");
    asset<FontData>::add_search_path("data\\fonts");
    asset<FontData>::add_reader<FontDataReader>("ttf");
    asset<FontData>::add_reader<FontDataReader>("otf");
    
    REG_COMP(Transform);
    REG_COMP(Model);
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