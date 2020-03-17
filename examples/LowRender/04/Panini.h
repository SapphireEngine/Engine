#pragma once

#include "Platform/FileSystem/FileSystem.h"
#include "Renderer/Renderer.h"

using namespace se;

extern ResourceDirectory RD_MIDDLEWARE_PANINI;

/************************************************************************/
/*					   HOW TO USE THIS MODULE
*************************************************************************/
//
// - Init()	 Compiles the panini shader, creates the vertex and index buffer for panini projection
//
// - Load()	 Links the shader compiled in Init to create the panini projection pipeline
//			  Uses the input render target to provide output format information to pipeline creation
//
// - Unload()   Destroys the pipeline
//
// - Update()   Empty update
//
// - Draw()	 Runs the panini projection shader on the active render pass. Call SetParams before calling this function to update the panini params
//
// - Exit()	 should be called when exiting the app to clean up Panini rendering resources.
//
// Panini Post Process takes a texture as input which contains the rendered scene, applies Panini distortion to it and outputs to currently bound render target.
// See UnitTests - 04_ExecuteIndirect project for example use case for this module.
//
/************************************************************************/
/*					   PANINI PROJECTION
*************************************************************************/
//
// The Pannini projection is a mathematical rule for constructing perspective images with very wide fields of view.
// source:  http://tksharpless.net/vedutismo/Pannini/
// paper:   http://tksharpless.net/vedutismo/Pannini/panini.pdf
//
struct PaniniParameters
{
	// horizontal field of view in degrees
	float FoVH = 90.0f;

	// D parameter: Distance of projection's center from the Panini frame's origin.
	//			  i.e. controls horizontal compression.
	//			  D = 0.0f	: regular rectilinear projection
	//			  D = 1.0f	: Panini projection
	//			  D->Infinity : cylindrical orthographic projection
	float D = 1.0f;

	// S parameter: A scalar that controls 'Hard Vertical Compression' of the projection.
	//			  Panini projection produces curved horizontal lines, which can feel
	//			  unnatural. Vertical compression attempts to straighten those curved lines.
	//			  S parameter works for FoVH < 180 degrees
	//			  S = 0.0f	: No compression
	//			  S = 1.0f	: Full straightening
	float S = 0.0f;

	// scale:	   Rendering scale to fit the distorted image to the screen. The bigger
	//			  the FoVH, the bigger the distortion is, hence the bigger the scale should
	//			  be in order to fit the image to screen.
	float scale = 1.0f;
};
/************************************************************************/
/*					   INTERFACE
*************************************************************************/
class Panini
{
public:
	// our init function should only be called once
	// the middleware has to keep these pointers
	bool Init(Renderer* renderer);
	void Exit();

	// when app is loaded, app is provided of the render targets to load
	// app is responsible to keep track of these render targets until load is called again
	// app will use the -first- rendertarget as texture to render to
	// make sure to always supply at least one render target with texture!
	bool Load(RenderTarget** rts, uint32_t count = 1);
	void Unload();

	// draws Panini Projection into first render target supplied at the Load call
	void Update(float deltaTime);
	void Draw(Cmd* cmd);

	// Allocates descriptor memory
	void SetMaxDraws(uint32_t maxDraws);

	// Set input texture to sample from
	void SetSourceTexture(se::Texture* pTex, uint32_t index);
	// Sets the parameters to be sent to the panini projection shader
	void SetParams(const PaniniParameters& params)
	{
		mParams = params;
	}

private:
	Renderer*         pRenderer;

	se::Shader*           pShader = NULL;
	se::RootSignature*    pRootSignature = NULL;
	se::DescriptorSet*    pDescriptorSet = NULL;
	se::Sampler*          pSamplerPointWrap = NULL;
	se::Pipeline*         pPipeline = NULL;

	se::Buffer*           pVertexBufferTessellatedQuad = NULL;
	se::Buffer*           pIndexBufferTessellatedQuad = NULL;

	PaniniParameters  mParams;
	uint32_t          mIndex;
	uint32_t          mMaxDraws;

	// Panini projection renders into a tessellated rectangle which imitates a curved cylinder surface
	const uint32_t    mPaniniDistortionTessellation[2] = { 64, 32 };
};