#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

struct Renderer;
struct RenderTarget;
struct Cmd;

class IMiddleware
{
public:
	// Our init function should only be called once
	// The middleware has to keep these pointers
	virtual bool Init(Renderer* renderer) = 0;
	virtual void Exit() = 0;

	// When app is loaded, app is provided of the render targets to load
	// App is responsible to keep track of these render targets until load is called again
	virtual bool Load(RenderTarget** rts, uint32_t count = 1) = 0;
	virtual void Unload() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual void Draw(Cmd* cmd) = 0;
};

SE_NAMESPACE_END
//=============================================================================