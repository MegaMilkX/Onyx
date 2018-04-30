#include "state_gameplay.h"

#define REG_COMP(NAME) SceneObject::RegisterComponent<NAME>(#NAME)

int main()
{
    asset<Mesh>::add_search_path("data\\model");
    asset<Mesh>::add_reader("fbx", &LoadAsset<Mesh, FBX>);
    asset<ScriptData>::add_search_path("data\\script");
    asset<ScriptData>::add_reader("lua", &LoadAsset<ScriptData, LUA>);
    asset<Material>::add_search_path("data\\material");
    asset<Material>::add_reader("json", &LoadAsset<Material, JSON>);
    asset<Animation>::add_search_path("data\\anim");
    asset<Animation>::add_search_path("data\\model");
    asset<Animation>::add_reader("fbx", &LoadAsset<Animation, FBX>);
    asset<SkeletonData>::add_search_path("data\\model");
    asset<SkeletonData>::add_reader("fbx", &LoadAsset<SkeletonData, FBX>);
    asset<Texture2D>::add_search_path("data\\texture");
    asset<Texture2D>::add_reader("png", &LoadAsset<Texture2D, PNG>);
    asset<Texture2D>::add_reader("jpg", &LoadAsset<Texture2D, JPG>);
    asset<SoundClip>::add_search_path("data\\audio");
    asset<SoundClip>::add_reader("ogg", &LoadAsset<SoundClip, OGG>);
    asset<FontData>::add_search_path("data\\fonts");
    asset<FontData>::add_reader("ttf", &LoadAsset<FontData, TTF>);
    asset<FontData>::add_reader("otf", &LoadAsset<FontData, TTF>);
    
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