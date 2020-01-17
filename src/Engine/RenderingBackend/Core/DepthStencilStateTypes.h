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
	StencilOp		stencilFailOp;		// Default: "Rhi::StencilOp::KEEP"
	StencilOp		stencilDepthFailOp;	// Default: "Rhi::StencilOp::KEEP"
	StencilOp		stencilPassOp;		// Default: "Rhi::StencilOp::KEEP"
	ComparisonFunc	stencilFunc;		// Default: "Rhi::ComparisonFunc::ALWAYS"
};



SE_NAMESPACE_END
//=============================================================================