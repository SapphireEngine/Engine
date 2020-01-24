﻿#include "stdafx.h"
#pragma comment(lib, "Engine.lib")

using namespace se;

#include "Platform/Window/Window.h"
#include "Platform/Window/EventQueue.h"
//#include "RenderingBackend/Renderer/Renderer.h"
#include "RenderingBackend/RenderSystem.h"
#include "Core/Utility/MakeID.h"
#include "Core/Debug/Log.h"
#include "Core/Debug/Assert.h"
#include "Core/Memory/Memory.h"

using namespace window;

int main()
{
    Log::Create();
    Assert::Create();
    Memory::Create();

    WindowConfig windowDesc;
    windowDesc.name = L"Test";
    windowDesc.title = L"My Title";
    windowDesc.visible = true;
    windowDesc.width = 1280;
    windowDesc.height = 720;
    
    Window::Create();

    auto &window = GetSubsystem<Window>();


    window::EventQueue eventQueue;
    if( !window.Init(windowDesc, eventQueue) )
    {
        return 0;
    }

    //Renderer renderer(window);
    RenderConfig rconfig;
    RenderSystem render(rconfig, window);

    bool isRunning = true;
    while( isRunning )
    {
        bool shouldRender = true;

        eventQueue.Update();

        while( !eventQueue.Empty() )
        {
            const window::Event &event = eventQueue.Front();

            if( event.type == window::EventType::Resize )
            {
                const window::ResizeData data = event.data.resize;
                //renderer.resize(data.width, data.height);
                render.Resize(data.width, data.height);
                shouldRender = false;
            }
            if( event.type == window::EventType::Close )
            {
                window.Close();
                isRunning = false;
                shouldRender = false;
            }

            eventQueue.Pop();
        }

        if( shouldRender )
        {
            // renderer.render();
            render.BeginFrame();
            render.EndFrame();
        }
    }

    Window::Destroy();
    Memory::Destroy();
    Assert::Destroy();
    Log::Destroy();

    return 0;
}