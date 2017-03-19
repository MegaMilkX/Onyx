#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <iostream>

#include <stack>
#include <map>

#include <aurora/window.h>
#include <aurora/gfx.h>
#include <aurora/input.h>

class MouseHandler : public Au::Input::MouseHandler
{
public:
    void KeyUp(Au::Input::KEYCODE key)
    {
    }
    
    void KeyDown(Au::Input::KEYCODE key)
    {
    }
    
    void Move(int x, int y)
    {        

    }
    
    void Wheel(short value)
    {
    }
};

class KeyboardHandler : public Au::Input::KeyboardHandler
{
public:
    void KeyUp(Au::Input::KEYCODE key)
    {
        std::cout << "Keyboard UP" << key <<std::endl;
    }
    
    void KeyDown(Au::Input::KEYCODE key)
    {
        std::cout << "Keyboard DOWN" << key <<std::endl;
    }
};

class GameState
{
public:
    virtual ~GameState(){}

    virtual void OnInit() = 0;
    virtual void OnSwitch() = 0;
    virtual void OnCleanup() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    
    virtual void MouseKeyUp(Au::Input::KEYCODE key) {}
    virtual void MouseKeyDown(Au::Input::KEYCODE key) {}
    virtual void MouseMove(int x, int y) {}
    virtual void MouseWheel(short value) {}
    virtual void KeyUp(Au::Input::KEYCODE key) {}
    virtual void KeyDown(Au::Input::KEYCODE key) {}
    
    template<typename T>
    static void Push()
    {
        GameState* state = new T();
        state->OnInit();
        stateStack.push(state);
    }
    
    static void Init()
    {
        window = Au::Window::Create("Onyx", 1280, 720);
        window->Show();
        gfxDevice.Init(*window);
        mouseHandler.Init(window);
        keyboardHandler.Init(window);
    }
    
    static bool Update()
    {
        bool result = !window->Destroyed();
        if(result)
        {
            Au::Window::PollMessages();
            
            stateStack.top()->OnUpdate();
            
            gfxDevice.Clear();
            stateStack.top()->OnRender();
            gfxDevice.SwapBuffers();
        }
        return result;
    }
    
    static void Cleanup()
    {
        gfxDevice.Cleanup();
        Au::Window::Destroy(window);
    }
    
    static void PostMouseKeyUp(Au::Input::KEYCODE key) { stateStack.top()->MouseKeyUp(key); }
    static void PostMouseKeyDown(Au::Input::KEYCODE key) { stateStack.top()->MouseKeyDown(key); }
    static void PostMouseMove(int x, int y) { stateStack.top()->MouseMove(x, y); }
    static void PostMouseWheel(short value) { stateStack.top()->MouseWheel(value); }
    static void PostKeyUp(Au::Input::KEYCODE key) { stateStack.top()->KeyUp(key); }
    static void PostKeyDown(Au::Input::KEYCODE key) { stateStack.top()->KeyDown(key); }
private:
    static std::stack<GameState*> stateStack;

    static Au::Window* window;
    static Au::GFX::Device gfxDevice;
    static MouseHandler mouseHandler;
    static KeyboardHandler keyboardHandler;
};

#endif
