#include "state_gameplay.h"

#define REG_COMP(NAME) SceneObject::RegisterComponent<NAME>(#NAME)

int main()
{
    asset<Mesh>::add_search_path("data\\model");
    asset<Mesh>::add_reader<MeshReaderFBX>("fbx");
    asset<ScriptData>::add_search_path("data\\script");
    asset<ScriptData>::add_reader<ScriptReaderLUA>("lua");
    asset<Material>::add_search_path("data\\material");
    asset<Material>::add_reader<MaterialReaderJSON>("json");
    asset<Animation>::add_search_path("data\\anim");
    asset<Animation>::add_search_path("data\\model");
    asset<Animation>::add_reader<AnimationReaderFBX>("fbx");
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
    REG_COMP(Animator);
    REG_COMP(SoundEmitter);

    GameState::Init();
    GameState::Push<Gameplay>();
    while(GameState::Update());
    GameState::Cleanup();
    
    return 0;
}