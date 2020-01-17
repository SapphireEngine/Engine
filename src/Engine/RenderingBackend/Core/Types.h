#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

enum class MapType
{
	READ = 1,
	WRITE = 2,
	READ_WRITE = 3,
	WRITE_DISCARD = 4,
	WRITE_NO_OVERWRITE = 5
};

struct ClearFlag final
{
	enum Enum
	{
		COLOR = 1 << 0,				// Clear color buffer
		DEPTH = 1 << 1,				// Clear depth buffer
		STENCIL = 1 << 2,			// Clear stencil buffer
		COLOR_DEPTH = COLOR | DEPTH	// Clear color and depth buffer
	};
};

enum class ComparisonFunc
{
	NEVER = 1,	// Never pass the comparison
	LESS = 2,	// If the source data is less than the destination data, the comparison passes
	EQUAL = 3,	// If the source data is equal to the destination data, the comparison passes
	LESS_EQUAL = 4,	// If the source data is less than or equal to the destination data, the comparison passes
	GREATER = 5,	// If the source data is greater than the destination data, the comparison passes
	NOT_EQUAL = 6,	// If the source data is not equal to the destination data, the comparison passes
	GREATER_EQUAL = 7,	// If the source data is greater than or equal to the destination data, the comparison passes
	ALWAYS = 8	// Always pass the comparison
};

struct ColorWriteEnableFlag final
{
	enum Enum
	{
		RED = 1,
		GREEN = 2,
		BLUE = 4,
		ALPHA = 8,
		ALL = (((RED | GREEN) | BLUE) | ALPHA)
	};
};

struct MappedSubresource final
{
	void *data;
	uint32_t rowPitch;
	uint32_t depthPitch;
};

struct Viewport final
{
	float topLeftX;	// Top left x start position
	float topLeftY;	// Top left y start position
	float width;	// Viewport width
	float height;	// Viewport height
	float minDepth;	// Minimum depth value, usually 0.0f, between [0, 1]
	float maxDepth;	// Maximum depth value, usually 1.0f, between [0, 1]
};

struct ScissorRectangle final
{
	long topLeftX;		// Top left x-coordinate of the scissor rectangle
	long topLeftY;		// Top left y-coordinate of the scissor rectangle
	long bottomRightX;	// Bottom right x-coordinate of the scissor rectangle
	long bottomRightY;	// Bottom right y-coordinate of the scissor rectangle
};

SE_NAMESPACE_END
//=============================================================================