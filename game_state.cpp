#include "game_state.h"

std::stack<GameState*> GameState::stateStack;

Au::Window* GameState::window;
Au::GFX::Device GameState::gfxDevice;
MouseHandler GameState::mouseHandler;
KeyboardHandler GameState::keyboardHandler;