#include "stdafx.h"
#pragma comment(lib, "Engine.lib")

using namespace se;

#include "Platform/Window/Window.h"
#include "Platform/Window/EventQueue.h"
//#include "RenderingBackend/Renderer/Renderer.h"

/*
- генератор данжей как ассет
*/

int main()
{
    window::WindowConfig windowDesc;
    windowDesc.name = L"Test";
    windowDesc.title = L"My Title";
    windowDesc.visible = true;
    windowDesc.width = 1280;
    windowDesc.height = 720;

    window::Window window;
    window::EventQueue eventQueue;
    if( !window.Create(windowDesc, eventQueue) )
    {
        return 0;
    }

    //Renderer renderer(window);

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
        }
    }

    return 0;
}