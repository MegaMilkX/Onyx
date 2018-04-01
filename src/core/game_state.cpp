#include "game_state.h"

uint64_t GameState::frameCount = 0;
float GameState::deltaTime = 0.0f;
Au::Timer GameState::timer;

std::stack<GameState*> GameState::stateStack;

Au::Window* GameState::window;
Au::GFX::Device GameState::gfxDevice;
AudioMixer3D GameState::audioMixer;
GameState::MouseHandler GameState::mouseHandler;
GameState::KeyboardHandler GameState::keyboardHandler;

