#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <iostream>

#include <stack>
#include <map>

#include <aurora/window.h>
#include <aurora/gfx.h>
#include <aurora/input.h>

#include <aurora/timer.h>

#include "lib/audio/audio_mixer_3d.h"

class GameState
{
public:
    class MouseHandler : public Au::Input::MouseHandler
    {
    public:
        void KeyUp(Au::Input::KEYCODE key) { PostMouseKeyUp(key); }
        void KeyDown(Au::Input::KEYCODE key) { PostMouseKeyDown(key); }
        void Move(int x, int y) { PostMouseMove(x, y); }
        void Wheel(short value) { PostMouseWheel(value); }
    };

    class KeyboardHandler : public Au::Input::KeyboardHandler
    {
    public:
        void KeyUp(Au::Input::KEYCODE key) { PostKeyUp(key); }
        void KeyDown(Au::Input::KEYCODE key) { PostKeyDown(key); }
        void OnChar(int charCode) { PostOnChar(charCode); }
    };
    
    virtual ~GameState(){}

    virtual void OnInit() {};
    virtual void OnSwitch() {};
    virtual void OnCleanup() {};
    virtual void OnUpdate() {};
    virtual void OnRender(Au::GFX::Device* device) {};
    
    virtual void MouseKeyUp(Au::Input::KEYCODE key) {}
    virtual void MouseKeyDown(Au::Input::KEYCODE key) {}
    virtual void MouseMove(int x, int y) {}
    virtual void MouseWheel(short value) {}
    virtual void KeyUp(Au::Input::KEYCODE key) {}
    virtual void KeyDown(Au::Input::KEYCODE key) {}
    virtual void OnChar(int charCode) {}
    
    template<typename T>
    static void Push()
    {
        GameState* state = new T();
        state->OnInit();
        stateStack.push(state);
    }
    
    static void Pop()
    {
        if(stateStack.empty())
            return;
        delete stateStack.top();
        stateStack.pop();
    }
    
    static void Init()
    {
        window = Au::Window::Create("Onyx", 1920, 1080);
        window->Show();
        gfxDevice.Init(*window);
        audioMixer.Init(48000, 16);
        DWORD threadId;
        HANDLE thread = CreateThread(NULL, 0, AudioThread, (void*)&audioMixer, 0, &threadId);
        mouseHandler.Init(window);
        keyboardHandler.Init(window);
    }
    
    static DWORD WINAPI AudioThread(LPVOID lpParam)
    {
        AudioMixer3D* mix = (AudioMixer3D*)lpParam;
        while(1)
        {
            Sleep(5);
            mix->Update();
        }
        return 0;
    }
    
    static bool Update()
    {
        timer.Start();
        bool result = !window->Destroyed();
        if(result)
        {
            Au::Window::PollMessages();
            
            stateStack.top()->OnUpdate();
            
            gfxDevice.Clear();
            stateStack.top()->OnRender(&gfxDevice);
            gfxDevice.SwapBuffers();
        }
        deltaTime = timer.End() / 1000000.0f;
        
        // Maintaining 60fps, TODO: remove hack, add fps limiting system
        while(deltaTime < 1.0f/60.0f)
        {
            deltaTime = timer.End() / 1000000.0f;
        }
        
        return result;
    }
    
    static void Cleanup()
    {
        audioMixer.Cleanup();
        gfxDevice.Cleanup();
        Au::Window::Destroy(window);
    }
    
    float DeltaTime() { return deltaTime; }
    
    static Au::GFX::Device* GFXDevice() { return &gfxDevice; }
    static AudioMixer3D* GetAudioMixer() { return &audioMixer; }
    
    static void PostMouseKeyUp(Au::Input::KEYCODE key) { stateStack.top()->MouseKeyUp(key); }
    static void PostMouseKeyDown(Au::Input::KEYCODE key) { stateStack.top()->MouseKeyDown(key); }
    static void PostMouseMove(int x, int y) { stateStack.top()->MouseMove(x, y); }
    static void PostMouseWheel(short value) { stateStack.top()->MouseWheel(value); }
    static void PostKeyUp(Au::Input::KEYCODE key) { stateStack.top()->KeyUp(key); }
    static void PostKeyDown(Au::Input::KEYCODE key) { stateStack.top()->KeyDown(key); }
    static void PostOnChar(int charCode) { stateStack.top()->OnChar(charCode); }
private:
    static float deltaTime;
    static Au::Timer timer;

    static std::stack<GameState*> stateStack;

    static Au::Window* window;
    static Au::GFX::Device gfxDevice;
    static AudioMixer3D audioMixer;
    static MouseHandler mouseHandler;
    static KeyboardHandler keyboardHandler;
};

#endif
