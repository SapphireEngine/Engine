#pragma once

#include "RenderingBackend/Core/ResourceType.h"
#include "Core/Debug/Assert.h"
#include "RenderingBackend/Core/SamplerStateTypes.h"

//=============================================================================
SE_NAMESPACE_BEGIN

enum class DescriptorRangeType
{
	SRV = 0,			// Shader resource view (SRV), in HLSL: "t<index>"-register
	UAV = SRV + 1,	// Unordered access view (UAV), in HLSL: "u<index>"-register
	UBV = UAV + 1,	// Uniform buffer view (UBV), in HLSL: "b<index>"-register, "UBV" = "CBV"; we're using the OpenGL/Vulkan terminology of "uniform buffer" instead of "constant buffer" as DirectX does
	SAMPLER = UBV + 1,	// In HLSL: "s<index>"-register
	NUMBER_OF_RANGE_TYPES = SAMPLER + 1	// Number of range type, invalid descriptor range type
};

enum class ShaderVisibility
{
	ALL = 0,
	VERTEX = 1,
	TESSELLATION_CONTROL = 2,
	TESSELLATION_EVALUATION = 3,
	GEOMETRY = 4,
	FRAGMENT = 5,
	// The rest is not part of "D3D12_SHADER_VISIBILITY"
	COMPUTE = 6,
	ALL_GRAPHICS = 7
};

struct DescriptorRange
{
	DescriptorRangeType rangeType;
	uint32_t			numberOfDescriptors;
	uint32_t			baseShaderRegister;	// When using explicit binding locations
	uint32_t			registerSpace;
	uint32_t			offsetInDescriptorsFromTableStart;

	// The rest is not part of "D3D12_DESCRIPTOR_RANGE" and was added to support Vulkan, OpenGL and Direct3D 9 as well
	static constexpr uint32_t NAME_LENGTH = 31 + 1;	// +1 for the terminating zero
	char					  baseShaderRegisterName[NAME_LENGTH];	// When not using explicit binding locations (OpenGL ES 3, legacy GLSL profiles)
	ShaderVisibility		  shaderVisibility;
	ResourceType			  resourceType;
};
struct DescriptorRangeBuilder final : public DescriptorRange
{
	static constexpr uint32_t OFFSET_APPEND = 0xffffffff;
	static inline void Initialize(
		DescriptorRange& range,
		ResourceType _resourceType,
		uint32_t _baseShaderRegister,
		const char _baseShaderRegisterName[NAME_LENGTH],
		ShaderVisibility _shaderVisibility,
		DescriptorRangeType _rangeType = DescriptorRangeType::NUMBER_OF_RANGE_TYPES,	// Automatically determine the descriptor range type basing on the resource type
		uint32_t _numberOfDescriptors = 1,
		uint32_t _registerSpace = 0,
		uint32_t _offsetInDescriptorsFromTableStart = OFFSET_APPEND)
	{
		if ( DescriptorRangeType::NUMBER_OF_RANGE_TYPES == _rangeType )
		{
			// Automatically determine the descriptor range type basing on the resource type
			switch ( _resourceType )
			{
			case ResourceType::VERTEX_BUFFER:
			case ResourceType::INDEX_BUFFER:
			case ResourceType::TEXTURE_BUFFER:
			case ResourceType::STRUCTURED_BUFFER:
			case ResourceType::INDIRECT_BUFFER:
				range.rangeType = DescriptorRangeType::SRV;
				break;

			case ResourceType::UNIFORM_BUFFER:
				range.rangeType = DescriptorRangeType::UBV;
				break;

			case ResourceType::TEXTURE_1D:
			case ResourceType::TEXTURE_1D_ARRAY:
			case ResourceType::TEXTURE_2D:
			case ResourceType::TEXTURE_2D_ARRAY:
			case ResourceType::TEXTURE_3D:
			case ResourceType::TEXTURE_CUBE:
			case ResourceType::TEXTURE_CUBE_ARRAY:
				range.rangeType = DescriptorRangeType::SRV;
				break;

			case ResourceType::SAMPLER_STATE:
				range.rangeType = DescriptorRangeType::SAMPLER;
				break;

			case ResourceType::ROOT_SIGNATURE:
			case ResourceType::RESOURCE_GROUP:
			case ResourceType::GRAPHICS_PROGRAM:
			case ResourceType::VERTEX_ARRAY:
			case ResourceType::RENDER_PASS:
			case ResourceType::QUERY_POOL:
			case ResourceType::SWAP_CHAIN:
			case ResourceType::FRAMEBUFFER:
			case ResourceType::GRAPHICS_PIPELINE_STATE:
			case ResourceType::COMPUTE_PIPELINE_STATE:
			case ResourceType::VERTEX_SHADER:
			case ResourceType::TESSELLATION_CONTROL_SHADER:
			case ResourceType::TESSELLATION_EVALUATION_SHADER:
			case ResourceType::GEOMETRY_SHADER:
			case ResourceType::FRAGMENT_SHADER:
			case ResourceType::COMPUTE_SHADER:
				SE_ASSERT(false, "Invalid resource type")
				break;
			}
		}
		else
		{
			range.rangeType = _rangeType;
		}
		range.numberOfDescriptors = _numberOfDescriptors;
		range.baseShaderRegister = _baseShaderRegister;
		range.registerSpace = _registerSpace;
		range.offsetInDescriptorsFromTableStart = _offsetInDescriptorsFromTableStart;
		strcpy(range.baseShaderRegisterName, _baseShaderRegisterName);
		range.shaderVisibility = _shaderVisibility;
		range.resourceType = _resourceType;
		SE_ASSERT(ResourceType::UNIFORM_BUFFER != range.resourceType || DescriptorRangeType::UAV != range.rangeType, "Uniform buffer doesn't support UAV")
	}
	inline DescriptorRangeBuilder()
	{
	}
	inline explicit DescriptorRangeBuilder(const DescriptorRangeBuilder&)
	{
	}
	inline DescriptorRangeBuilder(
		ResourceType _resourceType,
		uint32_t _baseShaderRegister,
		const char _baseShaderRegisterName[NAME_LENGTH],
		ShaderVisibility _shaderVisibility,
		DescriptorRangeType _rangeType = DescriptorRangeType::NUMBER_OF_RANGE_TYPES,	// Automatically determine the descriptor range type basing on the resource type
		uint32_t _numberOfDescriptors = 1,
		uint32_t _registerSpace = 0,
		uint32_t _offsetInDescriptorsFromTableStart = OFFSET_APPEND)
	{
		Initialize(_resourceType, _baseShaderRegister, _baseShaderRegisterName, _shaderVisibility, _rangeType, _numberOfDescriptors, _registerSpace, _offsetInDescriptorsFromTableStart);
	}
	inline void InitializeSampler(
		uint32_t _baseShaderRegister,
		ShaderVisibility _shaderVisibility,
		uint32_t _numberOfDescriptors = 1,
		uint32_t _registerSpace = 0,
		uint32_t _offsetInDescriptorsFromTableStart = OFFSET_APPEND)
	{
		Initialize(*this, ResourceType::SAMPLER_STATE, _baseShaderRegister, "", _shaderVisibility, DescriptorRangeType::SAMPLER, _numberOfDescriptors, _registerSpace, _offsetInDescriptorsFromTableStart);
	}
	inline void Initialize(
		ResourceType _resourceType,
		uint32_t _baseShaderRegister,
		const char _baseShaderRegisterName[NAME_LENGTH],
		ShaderVisibility _shaderVisibility,
		DescriptorRangeType _rangeType = DescriptorRangeType::NUMBER_OF_RANGE_TYPES,	// Automatically determine the descriptor range type basing on the resource type
		uint32_t _numberOfDescriptors = 1,
		uint32_t _registerSpace = 0,
		uint32_t _offsetInDescriptorsFromTableStart = OFFSET_APPEND)
	{
		Initialize(*this, _resourceType, _baseShaderRegister, _baseShaderRegisterName, _shaderVisibility, _rangeType, _numberOfDescriptors, _registerSpace, _offsetInDescriptorsFromTableStart);
	}
};

struct RootDescriptorTable
{
	uint32_t numberOfDescriptorRanges;
	uint64_t descriptorRanges; // Can't use "const DescriptorRange*" because we need to have something platform neutral we can easily serialize without getting too fine granular
};
struct RootDescriptorTableBuilder final : public RootDescriptorTable
{
	static inline void Initialize(
		RootDescriptorTable& rootDescriptorTable,
		uint32_t _numberOfDescriptorRanges,
		const DescriptorRange* _descriptorRanges)
	{
		rootDescriptorTable.numberOfDescriptorRanges = _numberOfDescriptorRanges;
		rootDescriptorTable.descriptorRanges = reinterpret_cast<uintptr_t>(_descriptorRanges);
	}
	inline RootDescriptorTableBuilder()
	{
	}
	inline explicit RootDescriptorTableBuilder(const RootDescriptorTableBuilder&)
	{
	}
	inline RootDescriptorTableBuilder(
		uint32_t _numberOfDescriptorRanges,
		const DescriptorRange* _descriptorRanges)
	{
		Initialize(_numberOfDescriptorRanges, _descriptorRanges);
	}
	inline void Initialize(
		uint32_t _numberOfDescriptorRanges,
		const DescriptorRange* _descriptorRanges)
	{
		Initialize(*this, _numberOfDescriptorRanges, _descriptorRanges);
	}
};

enum class RootParameterType
{
	DESCRIPTOR_TABLE = 0,
	CONSTANTS_32BIT = DESCRIPTOR_TABLE + 1,
	UBV = CONSTANTS_32BIT + 1,		// Uniform buffer view (UBV), in HLSL: "b<index>"-register, "UBV" = "CBV"; we're using the OpenGL/Vulkan terminology of "uniform buffer" instead of "constant buffer" as DirectX does
	SRV = UBV + 1,					// Shader resource view (SRV), in HLSL: "t<index>"-register
	UAV = SRV + 1					// Unordered access view (UAV), in HLSL: "u<index>"-register
};

struct RootConstants
{
	uint32_t shaderRegister;
	uint32_t registerSpace;
	uint32_t numberOf32BitValues;
};
struct RootConstantsBuilder final : public RootConstants
{
	static inline void Initialize(
		RootConstants& rootConstants,
		uint32_t _numberOf32BitValues,
		uint32_t _shaderRegister,
		uint32_t _registerSpace = 0)
	{
		rootConstants.numberOf32BitValues = _numberOf32BitValues;
		rootConstants.shaderRegister = _shaderRegister;
		rootConstants.registerSpace = _registerSpace;
	}
	inline RootConstantsBuilder()
	{
	}
	inline explicit RootConstantsBuilder(const RootConstantsBuilder&)
	{
	}
	inline RootConstantsBuilder(
		uint32_t _numberOf32BitValues,
		uint32_t _shaderRegister,
		uint32_t _registerSpace = 0)
	{
		Initialize(_numberOf32BitValues, _shaderRegister, _registerSpace);
	}
	inline void Initialize(
		uint32_t _numberOf32BitValues,
		uint32_t _shaderRegister,
		uint32_t _registerSpace = 0)
	{
		Initialize(*this, _numberOf32BitValues, _shaderRegister, _registerSpace);
	}
};

struct RootDescriptor
{
	uint32_t shaderRegister;
	uint32_t registerSpace;
};
struct RootDescriptorBuilder final : public RootDescriptor
{
	static inline void Initialize(RootDescriptor& table, uint32_t _shaderRegister, uint32_t _registerSpace = 0)
	{
		table.shaderRegister = _shaderRegister;
		table.registerSpace = _registerSpace;
	}
	inline RootDescriptorBuilder()
	{
	}
	inline explicit RootDescriptorBuilder(const RootDescriptorBuilder&)
	{
	}
	inline RootDescriptorBuilder(
		uint32_t _shaderRegister,
		uint32_t _registerSpace = 0)
	{
		Initialize(_shaderRegister, _registerSpace);
	}
	inline void Initialize(
		uint32_t _shaderRegister,
		uint32_t _registerSpace = 0)
	{
		Initialize(*this, _shaderRegister, _registerSpace);
	}
};

struct RootParameter
{
	RootParameterType		parameterType;
	union
	{
		RootDescriptorTable	descriptorTable;
		RootConstants		constants;
		RootDescriptor		descriptor;
	};
};
struct RootParameterData final
{
	RootParameterType	parameterType;
	uint32_t			numberOfDescriptorRanges;
};
struct RootParameterBuilder final : public RootParameter
{
	static inline void InitializeAsDescriptorTable(
		RootParameter& rootParam,
		uint32_t numberOfDescriptorRanges,
		const DescriptorRange* descriptorRanges)
	{
		rootParam.parameterType = RootParameterType::DESCRIPTOR_TABLE;
		RootDescriptorTableBuilder::Initialize(rootParam.descriptorTable, numberOfDescriptorRanges, descriptorRanges);
	}
	static inline void InitializeAsConstants(
		RootParameter& rootParam,
		uint32_t numberOf32BitValues,
		uint32_t shaderRegister,
		uint32_t registerSpace = 0)
	{
		rootParam.parameterType = RootParameterType::CONSTANTS_32BIT;
		RootConstantsBuilder::Initialize(rootParam.constants, numberOf32BitValues, shaderRegister, registerSpace);
	}
	static inline void InitializeAsConstantBufferView(
		RootParameter& rootParam,
		uint32_t shaderRegister,
		uint32_t registerSpace = 0)
	{
		rootParam.parameterType = RootParameterType::UBV;
		RootDescriptorBuilder::Initialize(rootParam.descriptor, shaderRegister, registerSpace);
	}
	static inline void InitializeAsShaderResourceView(
		RootParameter& rootParam,
		uint32_t shaderRegister,
		uint32_t registerSpace = 0)
	{
		rootParam.parameterType = RootParameterType::SRV;
		RootDescriptorBuilder::Initialize(rootParam.descriptor, shaderRegister, registerSpace);
	}
	static inline void InitializeAsUnorderedAccessView(
		RootParameter& rootParam,
		uint32_t shaderRegister,
		uint32_t registerSpace = 0)
	{
		rootParam.parameterType = RootParameterType::UAV;
		RootDescriptorBuilder::Initialize(rootParam.descriptor, shaderRegister, registerSpace);
	}
	inline RootParameterBuilder()
	{
	}
	inline explicit RootParameterBuilder(const RootParameterBuilder&)
	{
	}
	inline void InitializeAsDescriptorTable(
		uint32_t numberOfDescriptorRanges,
		const DescriptorRange* descriptorRanges)
	{
		InitializeAsDescriptorTable(*this, numberOfDescriptorRanges, descriptorRanges);
	}
	inline void InitializeAsConstants(
		uint32_t numberOf32BitValues,
		uint32_t shaderRegister,
		uint32_t registerSpace = 0)
	{
		InitializeAsConstants(*this, numberOf32BitValues, shaderRegister, registerSpace);
	}
	inline void InitializeAsConstantBufferView(
		uint32_t shaderRegister,
		uint32_t registerSpace = 0)
	{
		InitializeAsConstantBufferView(*this, shaderRegister, registerSpace);
	}
	inline void InitializeAsShaderResourceView(
		uint32_t shaderRegister,
		uint32_t registerSpace = 0)
	{
		InitializeAsShaderResourceView(*this, shaderRegister, registerSpace);
	}
	inline void InitializeAsUnorderedAccessView(
		uint32_t shaderRegister,
		uint32_t registerSpace = 0)
	{
		InitializeAsUnorderedAccessView(*this, shaderRegister, registerSpace);
	}
};

struct RootSignatureFlags final
{
	enum Enum
	{
		NONE = 0,
		ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT = 0x1,
		DENY_VERTEX_SHADER_ROOT_ACCESS = 0x2,
		DENY_TESSELLATION_CONTROL_SHADER_ROOT_ACCESS = 0x4,
		DENY_TESSELLATION_EVALUATION_SHADER_ROOT_ACCESS = 0x8,
		DENY_GEOMETRY_SHADER_ROOT_ACCESS = 0x10,
		DENY_FRAGMENT_SHADER_ROOT_ACCESS = 0x20,
		ALLOW_STREAM_OUTPUT = 0x40
	};
};

enum class StaticBorderColor
{
	TRANSPARENT_BLACK = 0,
	OPAQUE_BLACK = TRANSPARENT_BLACK + 1,
	OPAQUE_WHITE = OPAQUE_BLACK + 1
};

struct StaticSampler final
{
	FilterMode			filter;
	TextureAddressMode	addressU;
	TextureAddressMode	addressV;
	TextureAddressMode	addressW;
	float				mipLodBias;
	uint32_t			maxAnisotropy;
	ComparisonFunc		comparisonFunc;
	StaticBorderColor	borderColor;
	float				minLod;
	float				maxLod;
	uint32_t			shaderRegister;
	uint32_t			registerSpace;
	ShaderVisibility	shaderVisibility;
};

struct RootSignature
{
	uint32_t				 numberOfParameters;
	const RootParameter*	 parameters;
	uint32_t				 numberOfStaticSamplers;
	const StaticSampler*	 staticSamplers;
	RootSignatureFlags::Enum flags;
};
struct RootSignatureBuilder final : public RootSignature
{
	static inline void Initialize(
		RootSignature& rootSignature,
		uint32_t _numberOfParameters,
		const RootParameter* _parameters,
		uint32_t _numberOfStaticSamplers = 0,
		const StaticSampler* _staticSamplers = nullptr,
		RootSignatureFlags::Enum _flags = RootSignatureFlags::NONE)
	{
		rootSignature.numberOfParameters = _numberOfParameters;
		rootSignature.parameters = _parameters;
		rootSignature.numberOfStaticSamplers = _numberOfStaticSamplers;
		rootSignature.staticSamplers = _staticSamplers;
		rootSignature.flags = _flags;
	}
	inline RootSignatureBuilder()
	{
	}
	inline explicit RootSignatureBuilder(const RootSignatureBuilder&)
	{
	}
	inline RootSignatureBuilder(
		uint32_t _numberOfParameters,
		const RootParameter* _parameters,
		uint32_t _numberOfStaticSamplers = 0,
		const StaticSampler* _staticSamplers = nullptr,
		RootSignatureFlags::Enum _flags = RootSignatureFlags::NONE)
	{
		Initialize(_numberOfParameters, _parameters, _numberOfStaticSamplers, _staticSamplers, _flags);
	}
	inline void Initialize(
		uint32_t _numberOfParameters,
		const RootParameter* _parameters,
		uint32_t _numberOfStaticSamplers = 0,
		const StaticSampler* _staticSamplers = nullptr,
		RootSignatureFlags::Enum _flags = RootSignatureFlags::NONE)
	{
		Initialize(*this, _numberOfParameters, _parameters, _numberOfStaticSamplers, _staticSamplers, _flags);
	}
};

SE_NAMESPACE_END
//=============================================================================