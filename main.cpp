#include <aurora/window.h>

#include "game_state.h"

class StateTest : public GameState
{
public:
    virtual void OnInit()
    {
        
    }
    virtual void OnSwitch()
    {
        
    }
    virtual void OnCleanup()
    {
        
    }
    virtual void OnUpdate()
    {
        
    }
    virtual void OnRender()
    {
        
    }
    
    
private:
};

int main()
{
    GameState::Init();
    GameState::Push<StateTest>();
    while(GameState::Update());
    GameState::Cleanup();
    return 0;
}