#include "game_state.h"

float GameState::deltaTime;
Au::Timer GameState::timer;

std::stack<GameState*> GameState::stateStack;

Au::Window* GameState::window;
Au::GFX::Device GameState::gfxDevice;
GameState::MouseHandler GameState::mouseHandler;
GameState::KeyboardHandler GameState::keyboardHandler;

