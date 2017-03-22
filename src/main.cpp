#include "state_test.h"
#include "state_gameplay.h"

typedef Au::Math::Vec4f vec4;
typedef Au::Math::Vec3f vec3;
typedef Au::Math::Vec2f vec2;
typedef Au::Math::Mat4f mat4;
typedef Au::Math::Mat3f mat3;

int main()
{
    GameState::Init();
    GameState::Push<Gameplay>();
    while(GameState::Update());
    GameState::Cleanup();
    return 0;
}