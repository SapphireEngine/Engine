#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

enum class Blend
{
	ZERO = 1,
	ONE = 2,
	SRC_COLOR = 3,
	INV_SRC_COLOR = 4,
	SRC_ALPHA = 5,
	INV_SRC_ALPHA = 6,
	DEST_ALPHA = 7,
	INV_DEST_ALPHA = 8,
	DEST_COLOR = 9,
	INV_DEST_COLOR = 10,
	SRC_ALPHA_SAT = 11,
	BLEND_FACTOR = 14,
	INV_BLEND_FACTOR = 15,
	SRC_1_COLOR = 16,
	INV_SRC_1_COLOR = 17,
	SRC_1_ALPHA = 18,
	INV_SRC_1_ALPHA = 19
};

enum class BlendOp
{
	ADD = 1,
	SUBTRACT = 2,
	REV_SUBTRACT = 3,
	MIN = 4,
	MAX = 5
};

struct RenderTargetBlendDesc final
{
	int		blendEnable;			// Boolean value. Default: "false"
	Blend	srcBlend;				// Default: "Blend::ONE"
	Blend	destBlend;				// Default: "Blend::ZERO"
	BlendOp	blendOp;				// Default: "BlendOp::ADD"
	Blend	srcBlendAlpha;			// Default: "Blend::ONE"
	Blend	destBlendAlpha;			// Default: "Blend::ZERO"
	BlendOp	blendOpAlpha;			// Default: "BlendOp::ADD"
	uint8_t	renderTargetWriteMask;	// Combination of "ColorWriteEnableFlag"-flags. Default: "ColorWriteEnableFlag::ALL"
};

struct BlendState final
{
	int					  alphaToCoverageEnable;	// Boolean value. Default: "false"
	int					  independentBlendEnable;	// Boolean value. Default: "false"
	RenderTargetBlendDesc renderTarget[8];			// Default: See "RenderTargetBlendDesc"
};
struct BlendStateBuilder final
{
	/**
	*  @brief
	*    Return the default blend state
	*
	*  @return
	*    The default blend state, see "BlendState" for the default values
	*/
	[[nodiscard]] static inline const BlendState& getDefaultBlendState()
	{
		// As default values, the one of Direct3D 11 and Direct 10 were chosen in order to make it easier for those RHI implementations
		// (choosing OpenGL default values would bring no benefit due to the design of the OpenGL API)
		// - Direct3D 11 "D3D11_BLEND_DESC structure"-documentation at MSDN: http://msdn.microsoft.com/en-us/library/windows/desktop/ff476087%28v=vs.85%29.aspx		
		// - OpenGL & OpenGL ES 3: The official specifications (unlike Direct3D, OpenGL versions are more compatible to each other)

		// Return default values
		// TODO(co) Finish default state comments
		static constexpr BlendState BLEND_STATE =
		{																															//	Direct3D 11	Direct3D 10	Direct3D 9			OpenGL
			false,								// alphaToCoverageEnable (int)															"false"			"false"
			false,								// independentBlendEnable (int)															"false"			"false"

			{ // renderTarget[8]
				// renderTarget[0]
				{
					false,						// blendEnable (int)																	"false"			"false"
					Blend::ONE,					// srcBlend (Blend)																"ONE"			"ONE"
					Blend::ZERO,				// destBlend (Blend)																"ZERO"			"ZERO"
					BlendOp::ADD,				// blendOp (BlendOp)																"ADD"			"ADD"
					Blend::ONE,					// srcBlendAlpha (Blend)															"ONE"			"ONE"
					Blend::ZERO,				// destBlendAlpha (Blend)															"ZERO"			"ZERO"
					BlendOp::ADD,				// blendOpAlpha (BlendOp)															"ADD"			"ADD"
					ColorWriteEnableFlag::ALL	// renderTargetWriteMask (uint8_t), combination of "ColorWriteEnableFlag"-flags	"ALL"			"ALL"
				},
			// renderTarget[1]
			{
				false,						// blendEnable (int)																	"false"			"false"
				Blend::ONE,					// srcBlend (Blend)																"ONE"			"ONE"
				Blend::ZERO,				// destBlend (Blend)																"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOp (BlendOp)																"ADD"			"ADD"
				Blend::ONE,					// srcBlendAlpha (Blend)															"ONE"			"ONE"
				Blend::ZERO,				// destBlendAlpha (Blend)															"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOpAlpha (BlendOp)															"ADD"			"ADD"
				ColorWriteEnableFlag::ALL	// renderTargetWriteMask (uint8_t), combination of "ColorWriteEnableFlag"-flags	"ALL"			"ALL"
			},
			// renderTarget[2]
			{
				false,						// blendEnable (int)																	"false"			"false"
				Blend::ONE,					// srcBlend (Blend)																"ONE"			"ONE"
				Blend::ZERO,				// destBlend (Blend)																"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOp (BlendOp)																"ADD"			"ADD"
				Blend::ONE,					// srcBlendAlpha (Blend)															"ONE"			"ONE"
				Blend::ZERO,				// destBlendAlpha (Blend)															"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOpAlpha (BlendOp)															"ADD"			"ADD"
				ColorWriteEnableFlag::ALL	// renderTargetWriteMask (uint8_t), combination of "ColorWriteEnableFlag"-flags	"ALL"			"ALL"
			},
			// renderTarget[3]
			{
				false,						// blendEnable (int)																	"false"			"false"
				Blend::ONE,					// srcBlend (Blend)																"ONE"			"ONE"
				Blend::ZERO,				// destBlend (Blend)																"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOp (BlendOp)																"ADD"			"ADD"
				Blend::ONE,					// srcBlendAlpha (Blend)															"ONE"			"ONE"
				Blend::ZERO,				// destBlendAlpha (Blend)															"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOpAlpha (BlendOp)															"ADD"			"ADD"
				ColorWriteEnableFlag::ALL	// renderTargetWriteMask (uint8_t), combination of "ColorWriteEnableFlag"-flags	"ALL"			"ALL"
			},
			// renderTarget[4]
			{
				false,						// blendEnable (int)																	"false"			"false"
				Blend::ONE,					// srcBlend (Blend)																"ONE"			"ONE"
				Blend::ZERO,				// destBlend (Blend)																"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOp (BlendOp)																"ADD"			"ADD"
				Blend::ONE,					// srcBlendAlpha (Blend)															"ONE"			"ONE"
				Blend::ZERO,				// destBlendAlpha (Blend)															"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOpAlpha (BlendOp)															"ADD"			"ADD"
				ColorWriteEnableFlag::ALL	// renderTargetWriteMask (uint8_t), combination of "ColorWriteEnableFlag"-flags	"ALL"			"ALL"
			},
			// renderTarget[5]
			{
				false,						// blendEnable (int)																	"false"			"false"
				Blend::ONE,					// srcBlend (Blend)																"ONE"			"ONE"
				Blend::ZERO,				// destBlend (Blend)																"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOp (BlendOp)																"ADD"			"ADD"
				Blend::ONE,					// srcBlendAlpha (Blend)															"ONE"			"ONE"
				Blend::ZERO,				// destBlendAlpha (Blend)															"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOpAlpha (BlendOp)															"ADD"			"ADD"
				ColorWriteEnableFlag::ALL	// renderTargetWriteMask (uint8_t), combination of "ColorWriteEnableFlag"-flags	"ALL"			"ALL"
			},
			// renderTarget[6]
			{
				false,						// blendEnable (int)																	"false"			"false"
				Blend::ONE,					// srcBlend (Blend)																"ONE"			"ONE"
				Blend::ZERO,				// destBlend (Blend)																"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOp (BlendOp)																"ADD"			"ADD"
				Blend::ONE,					// srcBlendAlpha (Blend)															"ONE"			"ONE"
				Blend::ZERO,				// destBlendAlpha (Blend)															"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOpAlpha (BlendOp)															"ADD"			"ADD"
				ColorWriteEnableFlag::ALL	// renderTargetWriteMask (uint8_t), combination of "ColorWriteEnableFlag"-flags	"ALL"			"ALL"
			},
			// renderTarget[7]
			{
				false,						// blendEnable (int)																	"false"			"false"
				Blend::ONE,					// srcBlend (Blend)																"ONE"			"ONE"
				Blend::ZERO,				// destBlend (Blend)																"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOp (BlendOp)																"ADD"			"ADD"
				Blend::ONE,					// srcBlendAlpha (Blend)															"ONE"			"ONE"
				Blend::ZERO,				// destBlendAlpha (Blend)															"ZERO"			"ZERO"
				BlendOp::ADD,				// blendOpAlpha (BlendOp)															"ADD"			"ADD"
				ColorWriteEnableFlag::ALL	// renderTargetWriteMask (uint8_t), combination of "ColorWriteEnableFlag"-flags	"ALL"			"ALL"
			}
		}
		};
		return BLEND_STATE;
	}
};

SE_NAMESPACE_END
//=============================================================================