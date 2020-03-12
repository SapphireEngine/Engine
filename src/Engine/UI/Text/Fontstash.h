#pragma once

#include "Platform/FileSystem/FileSystem.h"

//=============================================================================
SE_NAMESPACE_BEGIN

struct Renderer;
struct RenderTarget;

extern ResourceDirectory RD_MIDDLEWARE_TEXT;

typedef struct TextDrawDesc
{
	TextDrawDesc(uint font = 0, uint32_t color = 0xffffffff, float size = 15.0f, float spacing = 0.0f, float blur = 0.0f) :
		mFontID(font),
		mFontColor(color),
		mFontSize(size),
		mFontSpacing(spacing),
		mFontBlur(blur)
	{
	}

	uint32_t mFontID;
	uint32_t mFontColor;
	float    mFontSize;
	float    mFontSpacing;
	float    mFontBlur;
} TextDrawDesc;

class Fontstash
{
public:
	bool init(Renderer* pRenderer, uint32_t width, uint32_t height);
	void exit();

	bool load(RenderTarget** pRts, uint32_t count);
	void unload();

	//! Makes a font available to the font stash.
	//! - Fonts can not be undefined in a FontStash due to its dynamic nature (once packed into an atlas, they cannot be unpacked, unless it is fully rebuilt)
	//! - Defined fonts will automatically be unloaded when the Fontstash is destroyed.
	//! - When it is paramount to be able to unload individual fonts, use multiple fontstashes.
	int defineFont(const char* identification, const char* filename, ResourceDirectory root);

	void*       getFontBuffer(uint32_t index);
	uint32_t    getFontBufferSize(uint32_t index);

	//! Draw text.
	void drawText(
		struct Cmd* pCmd, const char* message, float x, float y, int fontID, unsigned int color = 0xffffffff, float size = 16.0f,
		float spacing = 0.0f, float blur = 0.0f);

	//! Draw text in worldSpace.
	void drawText(
		struct Cmd* pCmd, const char* message, const mat4& projView, const mat4& worldMat, int fontID, unsigned int color = 0xffffffff,
		float size = 16.0f, float spacing = 0.0f, float blur = 0.0f);

	//! Measure text boundaries. Results will be written to out_bounds (x,y,x2,y2).
	float measureText(
		float* out_bounds, const char* message, float x, float y, int fontID, unsigned int color = 0xffffffff, float size = 16.0f,
		float spacing = 0.0f, float blur = 0.0f);

protected:
	float                  m_fFontMaxSize;
	class _Impl_FontStash* impl;
};

SE_NAMESPACE_END
//=============================================================================