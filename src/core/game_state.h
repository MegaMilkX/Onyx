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

#include "lib/event.h"

#include <util/imgui_wrapper.h>
#include <util/imgui_console.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#include <asset_storage.h>

#include <common.h>

struct eKeyDown{
    Au::Input::KEYCODE key;
};
struct eKeyUp{
    Au::Input::KEYCODE key;
};
struct eChar{
    int code;
};
struct eMouseMove{
    int dx, dy;
    int x, y;
};
struct eMouseWheel{
    short value;
};
struct eMouseDown{
    Au::Input::KEYCODE key;
    int x, y;
};
struct eMouseUp{
    Au::Input::KEYCODE key;
};

inline void FramebufferResizeCallback(GLFWwindow* win, int width, int height)
{
    Common.frameSize.x = width;
    Common.frameSize.y = height;
}

class GameState
{
public:
    class MouseHandler : public Au::Input::MouseHandler
    {
    public:
        void KeyUp(Au::Input::KEYCODE key) { 
            ImGuiIO& io = ImGui::GetIO();            
            if(key == Au::Input::KEY_LBUTTON)
            {
                io.MouseDown[0] = false;
            }
            if(key == Au::Input::KEY_RBUTTON)
            {
                io.MouseDown[1] = false;
            }

            event_post(eMouseUp{key});
            PostMouseKeyUp(key); 
        }
        void KeyDown(Au::Input::KEYCODE key) { 
            ImGuiIO& io = ImGui::GetIO();            
            if(key == Au::Input::KEY_LBUTTON)
            {
                io.MouseDown[0] = true;
            }
            if(key == Au::Input::KEY_RBUTTON)
            {
                io.MouseDown[1] = true;
            }

            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            event_post(eMouseDown{key, pt.x, pt.y});
            PostMouseKeyDown(key); 
        }
        void Move(int x, int y) { 
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);

            ImGuiIO& io = ImGui::GetIO();
            io.MousePos = ImVec2((float)pt.x, (float)pt.y);

            event_post(eMouseMove{x, y, pt.x, pt.y});
            PostMouseMove(x, y); 
        }
        void Wheel(short value) { 
            event_post(eMouseWheel{value});
            PostMouseWheel(value); 
        }
    };

    class KeyboardHandler : public Au::Input::KeyboardHandler
    {
    public:
        void KeyUp(Au::Input::KEYCODE key) { 
            ImGuiIO& io = ImGui::GetIO();
            if(key < 512) io.KeysDown[key] = false;
            if(io.WantCaptureKeyboard)
                return;

            event_post(eKeyUp{key});
            PostKeyUp(key); 
        }
        void KeyDown(Au::Input::KEYCODE key) { 
            ImGuiIO& io = ImGui::GetIO();
            if(key < 512) io.KeysDown[key] = true;
            if(io.WantCaptureKeyboard)
                return;

            event_post(eKeyDown{key});
            PostKeyDown(key); 
        }
        void OnChar(int charCode) {
            ImGuiIO& io = ImGui::GetIO();
            io.AddInputCharacter(charCode);
            if(io.WantCaptureKeyboard)
                return;
             
            event_post(eChar{charCode});
            PostOnChar(charCode);
        }
    };
    
    virtual ~GameState(){}

    virtual void OnInit() {};
    virtual void OnSwitch() {};
    virtual void OnCleanup() {};
    virtual void OnUpdate() {};
    virtual void OnRender() {};
    
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
        if(!glfwInit())
        {
            std::cout << "glfwInit() failed" << std::endl;
            return;
        }
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(1280, 720, "qwe", NULL, NULL);
        if(!window)
        {
            glfwTerminate();
            std::cout << "failed to create a window" << std::endl;
            return;
        }
        glfwSetFramebufferSizeCallback(window, &FramebufferResizeCallback);
        Common.frameSize.x = 1280;
        Common.frameSize.y = 720;
        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);

        WGLEXTLoadFunctions();
        GLEXTLoadFunctions();
        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        audioMixer.Init(48000, 16);
        DWORD threadId;
        HANDLE thread = CreateThread(NULL, 0, AudioThread, (void*)&audioMixer, 0, &threadId);
        
        mouseHandler.Init(glfwGetWin32Window(window));
        keyboardHandler.Init(glfwGetWin32Window(window));
        deltaTime = 0.0f;

        ImGuiInit();

        Update();
    }
    static ImGuiDbgConsole dbgConsole;
    
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
        assets::sync();

        ImGuiIO& io = ImGui::GetIO();
        ImGuiUpdate(DeltaTime());

        bool result = glfwWindowShouldClose(window) == 0;
        if(result)
        {
            if(!stateStack.empty())
            {
                stateStack.top()->OnUpdate();
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //gfxDevice.Clear();
            if(!stateStack.empty())
            {
                stateStack.top()->OnRender();
            }
            bool consoleOpen = true;
            dbgConsole.Draw("Dev console", &consoleOpen);
            ShowProfOverlay(&consoleOpen, (int)(1.0f / deltaTime), 1);
            //ShowFpsPlot((int)(1.0f / deltaTime));
            ImGuiDraw();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        deltaTime = timer.End() / 1000000.0f;
        
        // Maintaining 60fps, TODO: remove hack, add fps limiting system
        /*
        while(deltaTime < 1.0f/30.0f)
        {
            deltaTime = timer.End() / 1000000.0f;
        }
        */
        frameCount++;

        return result;
    }
    
    static void Cleanup()
    {
        ImGuiCleanup();

        audioMixer.Cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    static float DeltaTime() { return deltaTime; }
    static uint64_t FrameCount() { return frameCount; }
    
    static AudioMixer3D* GetAudioMixer() { return &audioMixer; }
    static MouseHandler* GetMouseHandler() { return &mouseHandler; }
    
    static void PostMouseKeyUp(Au::Input::KEYCODE key) { if(!stateStack.empty()) stateStack.top()->MouseKeyUp(key); }
    static void PostMouseKeyDown(Au::Input::KEYCODE key) { if(!stateStack.empty()) stateStack.top()->MouseKeyDown(key); }
    static void PostMouseMove(int x, int y) { if(!stateStack.empty()) stateStack.top()->MouseMove(x, y); }
    static void PostMouseWheel(short value) { if(!stateStack.empty()) stateStack.top()->MouseWheel(value); }
    static void PostKeyUp(Au::Input::KEYCODE key) { if(!stateStack.empty()) stateStack.top()->KeyUp(key); }
    static void PostKeyDown(Au::Input::KEYCODE key) { if(!stateStack.empty()) stateStack.top()->KeyDown(key); }
    static void PostOnChar(int charCode) { if(!stateStack.empty()) stateStack.top()->OnChar(charCode); }
private:
    static uint64_t frameCount;
    static float deltaTime;
    static Au::Timer timer;

    static std::stack<GameState*> stateStack;

    static GLFWwindow* window;
    //static Au::Window* window;
    static AudioMixer3D audioMixer;
    static MouseHandler mouseHandler;
    static KeyboardHandler keyboardHandler;
};

#endif
