#pragma comment(lib, "3rdPartyLib.lib")
#pragma comment(lib, "Engine.lib")
#pragma comment(lib, "winmm.lib")
#include "stdafx.h"
#include "Platform/Main.h"
#include "Platform/App.h"

using namespace se;

class Sample : public IApp
{
public:
	bool Init()
	{
		return true;
	}

	void Exit()
	{
	}

	bool Load()
	{
		return true;
	}

	void Unload()
	{
	}

	void Update(float deltaTime)
	{
	}

	void Draw()
	{
	}

	const char* GetName()
	{
		return "sample";
	}
};

int IApp::argc;
const char** IApp::argv; 
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	IApp::argc = argc;
	IApp::argv = (const char**)argv;
	Sample app;
	return WindowsMain(argc, argv, &app); 
}
//-----------------------------------------------------------------------------