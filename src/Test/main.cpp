#include "stdafx.h"
#pragma comment(lib, "Engine.lib")

using namespace se;

#include "Platform/Window/Window.h"
#include "RenderingBackend/Renderer/Renderer.h"

int main()
{
    window::Window window;
    window::EventQueue eventQueue;
    
    window::WindowDesc windowDesc;
    windowDesc.name = L"Test";
    windowDesc.title = L"My Title";
    windowDesc.visible = true;
    windowDesc.width = 1280;
    windowDesc.height = 720;

    if ( !window.create(windowDesc, eventQueue) )
    {
        return 0;
    }

    Renderer renderer(window);

    bool isRunning = true;
    while ( isRunning )
    {
        bool shouldRender = true;

        eventQueue.update();

        while ( !eventQueue.empty() )
        {            
            const window::Event &event = eventQueue.front();

            if ( event.type == window::EventType::Resize )
            {
                const window::ResizeData data = event.data.resize;
                renderer.resize(data.width, data.height);
                shouldRender = false;
            }
            if ( event.type == window::EventType::Close )
            {
                window.close();
                isRunning = false;
                shouldRender = false;
            }

            eventQueue.pop();
        }

        if ( shouldRender )
        {
            renderer.render();
        }
    }

	return 0;
}