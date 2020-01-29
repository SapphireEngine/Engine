#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

enum class DepthWriteMask
{
	ZERO = 0,
	ALL = 1
};

enum class StencilOp
{
	KEEP = 1,
	ZERO = 2,
	REPLACE = 3,
	INCR_SAT = 4,
	DECR_SAT = 5,
	INVERT = 6,
	INCREASE = 7,
	DECREASE = 8
};

struct DepthStencilOpDesc final
{
	StencilOp		stencilFailOp;		// Default: "StencilOp::KEEP"
	StencilOp		stencilDepthFailOp;	// Default: "StencilOp::KEEP"
	StencilOp		stencilPassOp;		// Default: "StencilOp::KEEP"
	ComparisonFunc	stencilFunc;		// Default: "ComparisonFunc::ALWAYS"
};

struct DepthStencilState final
{
	int					depthEnable;		// Boolean value. Default: "true"
	DepthWriteMask		depthWriteMask;		// Default: "DepthWriteMask::ALL"
	ComparisonFunc		depthFunc;			// Default: "ComparisonFunc::GREATER" instead of "ComparisonFunc::LESS" due to usage of Reversed-Z (see e.g. https://developer.nvidia.com/content/depth-precision-visualized and https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/)
	int					stencilEnable;		// Boolean value. Default: "false"
	uint8_t				stencilReadMask;	// Default: "0xff"
	uint8_t				stencilWriteMask;	// Default: "0xff"
	DepthStencilOpDesc	frontFace;			// Default: See "DepthStencilOpDesc"
	DepthStencilOpDesc	backFace;			// Default: See "DepthStencilOpDesc"
};

struct DepthStencilStateBuilder final
{
	[[nodiscard]] static inline const DepthStencilState& GetDefaultDepthStencilState()
	{
		// As default values, the one of Direct3D 11 and Direct 10 were chosen in order to make it easier for those RHI implementations
		// (choosing OpenGL default values would bring no benefit due to the design of the OpenGL API)
		// - Direct3D 11 "D3D11_DEPTH_STENCIL_DESC structure"-documentation at MSDN: http://msdn.microsoft.com/en-us/library/windows/desktop/ff476110%28v=vs.85%29.aspx		
		// - OpenGL & OpenGL ES 3: The official specifications (unlike Direct3D, OpenGL versions are more compatible to each other)

		// Return default values
		static constexpr DepthStencilState DEPTH_STENCIL_STATE =
		{																		//	Direct3D 11		Direct3D 10	Direct3D 9				OpenGL
			true,						// depthEnable (int)						"true"			"true"		"true"					TODO(co)
			DepthWriteMask::ALL,		// depthWriteMask (DepthWriteMask)		"ALL"			"ALL"		"ALL"					TODO(co)
			ComparisonFunc::GREATER,	// depthFunc (ComparisonFunc)			"LESS"			"LESS"		"LESS_EQUAL"			TODO(co)	- Default: "ComparisonFunc::GREATER" instead of "ComparisonFunc::LESS" due to usage of Reversed-Z (see e.g. https://developer.nvidia.com/content/depth-precision-visualized and https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/)
			false,						// stencilEnable (int)						"false"			"false"		"false"					TODO(co)
			0xff,						// stencilReadMask (uint8_t)				"0xff"			"0xff"		"0xffffffff"			TODO(co)
			0xff,						// stencilWriteMask (uint8_t)				"0xff"			"0xff"		"0xffffffff"			TODO(co)
			{ // sFrontFace (DepthStencilOpDesc)
				StencilOp::KEEP,		// stencilFailOp (StencilOp			"KEEP"			"KEEP"		"KEEP" (both sides)		TODO(co)
				StencilOp::KEEP,		// stencilDepthFailOp (StencilOp)		"KEEP"			"KEEP"		"KEEP" (both sides)		TODO(co)
				StencilOp::KEEP,		// stencilPassOp (StencilOp)			"KEEP"			"KEEP"		"KEEP" (both sides)		TODO(co)
				ComparisonFunc::ALWAYS	// stencilFunc (ComparisonFunc)		"ALWAYS"		"ALWAYS"	"ALWAYS" (both sides)
			},
			{ // sBackFace (DepthStencilOpDesc)
				StencilOp::KEEP,		// stencilFailOp (StencilOp)			"KEEP"			"KEEP"		"KEEP" (both sides)		TODO(co)
				StencilOp::KEEP,		// stencilDepthFailOp (StencilOp)		"KEEP"			"KEEP"		"KEEP" (both sides)		TODO(co)
				StencilOp::KEEP,		// stencilPassOp (StencilOp)			"KEEP"			"KEEP"		"KEEP" (both sides)		TODO(co)
				ComparisonFunc::ALWAYS	// stencilFunc (ComparisonFunc)		"ALWAYS"		"ALWAYS"	"ALWAYS" (both sides)	TODO(co)
			}
		};
		return DEPTH_STENCIL_STATE;
	}
};

SE_NAMESPACE_END
//=============================================================================