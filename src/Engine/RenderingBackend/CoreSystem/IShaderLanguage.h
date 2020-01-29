#pragma once

#include "Core/Template/Ref.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IRendererBackend;
class IVertexShader;
class IFragmentShader;
class IGeometryShader;
class ITessellationControlShader;
class ITessellationEvaluationShader;

class IShaderLanguage : public RefCount<IShaderLanguage>
{
public:
	enum class OptimizationLevel
	{
		DEBUG = 0,	// No optimization and debug features enabled, usually only used for debugging
		NONE,		// No optimization, usually only used for debugging
		LOW,		// Low optimization
		MEDIUM,		// Medium optimization
		HIGH,		// High optimization
		ULTRA		// Ultra optimization
	};

	inline virtual ~IShaderLanguage() override {}

	[[nodiscard]] inline IRendererBackend& GetRender() const
	{
		return *m_render;
	}

	[[nodiscard]] inline OptimizationLevel GetOptimizationLevel() const
	{
		return m_optimizationLevel;
	}

	inline void SetOptimizationLevel(OptimizationLevel optimizationLevel)
	{
		m_optimizationLevel = optimizationLevel;
	}

	// Create a graphics program and assigns a vertex and fragment shader to it
	[[nodiscard]] inline IGraphicsProgram* CreateGraphicsProgram(const IRootSignature &rootSignature, const VertexAttributes &vertexAttributes, IVertexShader *vertexShader, IFragmentShader *fragmentShader SE_DEBUG_NAME_PARAMETER)
	{
		return CreateGraphicsProgram(rootSignature, vertexAttributes, vertexShader, nullptr, nullptr, nullptr, fragmentShader SE_DEBUG_PASS_PARAMETER);
	}

	// Create a graphics program and assigns a vertex, geometry and fragment shader to it	
	[[nodiscard]] inline IGraphicsProgram* CreateGraphicsProgram(const IRootSignature& rootSignature, const VertexAttributes& vertexAttributes, IVertexShader* vertexShader, IGeometryShader* geometryShader, IFragmentShader* fragmentShader SE_DEBUG_NAME_PARAMETER)
	{
		return CreateGraphicsProgram(rootSignature, vertexAttributes, vertexShader, nullptr, nullptr, geometryShader, fragmentShader SE_DEBUG_PASS_PARAMETER);
	}
	
	// Create a graphics program and assigns a vertex, tessellation control, tessellation evaluation and fragment shader to it
	[[nodiscard]] inline IGraphicsProgram* CreateGraphicsProgram(const IRootSignature& rootSignature, const VertexAttributes& vertexAttributes, IVertexShader* vertexShader, ITessellationControlShader* tessellationControlShader, ITessellationEvaluationShader* tessellationEvaluationShader, IFragmentShader* fragmentShader SE_DEBUG_NAME_PARAMETER)
	{
		return CreateGraphicsProgram(rootSignature, vertexAttributes, vertexShader, tessellationControlShader, tessellationEvaluationShader, nullptr, fragmentShader SE_DEBUG_PASS_PARAMETER);
	}

	[[nodiscard]] virtual const char* GetShaderLanguageName() const = 0;

	// Create a vertex shader from shader bytecode
	[[nodiscard]] virtual IVertexShader* CreateVertexShaderFromBytecode(const VertexAttributes& vertexAttributes, const ShaderBytecode& shaderBytecode SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a vertex shader from shader source code
	[[nodiscard]] virtual IVertexShader* CreateVertexShaderFromSourceCode(const VertexAttributes& vertexAttributes, const ShaderSourceCode& shaderSourceCode, ShaderBytecode* shaderBytecode = nullptr SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a tessellation control shader ("hull shader" in Direct3D terminology) from shader bytecode
	[[nodiscard]] virtual ITessellationControlShader* CreateTessellationControlShaderFromBytecode(const ShaderBytecode& shaderBytecode SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a tessellation control shader ("hull shader" in Direct3D terminology) from shader source code	
	[[nodiscard]] virtual ITessellationControlShader* CreateTessellationControlShaderFromSourceCode(const ShaderSourceCode& shaderSourceCode, ShaderBytecode* shaderBytecode = nullptr SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a tessellation evaluation shader ("domain shader" in Direct3D terminology) from shader bytecode	
	[[nodiscard]] virtual ITessellationEvaluationShader* CreateTessellationEvaluationShaderFromBytecode(const ShaderBytecode& shaderBytecode SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a tessellation evaluation shader ("domain shader" in Direct3D terminology) from shader source code	
	[[nodiscard]] virtual ITessellationEvaluationShader* CreateTessellationEvaluationShaderFromSourceCode(const ShaderSourceCode& shaderSourceCode, ShaderBytecode* shaderBytecode = nullptr SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a geometry shader from shader bytecode	
	[[nodiscard]] virtual IGeometryShader* CreateGeometryShaderFromBytecode(const ShaderBytecode& shaderBytecode, GsInputPrimitiveTopology gsInputPrimitiveTopology, GsOutputPrimitiveTopology gsOutputPrimitiveTopology, uint32_t numberOfOutputVertices SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a geometry shader from shader source code	
	[[nodiscard]] virtual IGeometryShader* CreateGeometryShaderFromSourceCode(const ShaderSourceCode& shaderSourceCode, GsInputPrimitiveTopology gsInputPrimitiveTopology, GsOutputPrimitiveTopology gsOutputPrimitiveTopology, uint32_t numberOfOutputVertices, ShaderBytecode* shaderBytecode = nullptr SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a fragment shader from shader bytecode
	[[nodiscard]] virtual IFragmentShader* CreateFragmentShaderFromBytecode(const ShaderBytecode& shaderBytecode SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a fragment shader from shader source code	
	[[nodiscard]] virtual IFragmentShader* CreateFragmentShaderFromSourceCode(const ShaderSourceCode& shaderSourceCode, ShaderBytecode* shaderBytecode = nullptr SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a compute shader from shader bytecode
	[[nodiscard]] virtual IComputeShader* CreateComputeShaderFromBytecode(const ShaderBytecode& shaderBytecode SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a compute shader from shader source code	
	[[nodiscard]] virtual IComputeShader* CreateComputeShaderFromSourceCode(const ShaderSourceCode& shaderSourceCode, ShaderBytecode* shaderBytecode = nullptr SE_DEBUG_NAME_PARAMETER) = 0;

	// Create a graphics program and assigns a vertex, tessellation control, tessellation evaluation, geometry and fragment shader to it	
	[[nodiscard]] virtual IGraphicsProgram* CreateGraphicsProgram(const IRootSignature& rootSignature, const VertexAttributes& vertexAttributes, IVertexShader* vertexShader, ITessellationControlShader* tessellationControlShader, ITessellationEvaluationShader* tessellationEvaluationShader, IGeometryShader* geometryShader, IFragmentShader* fragmentShader SE_DEBUG_NAME_PARAMETER) = 0;

protected:
	inline explicit IShaderLanguage(IRendererBackend &render)
		: m_render(&render)
		, m_optimizationLevel(OptimizationLevel::ULTRA)
	{
	}

	explicit IShaderLanguage(const IShaderLanguage&) = delete;
	IShaderLanguage& operator=(const IShaderLanguage&) = delete;

private:
	IRendererBackend *m_render;	
	OptimizationLevel m_optimizationLevel;
};

typedef SmartRefCount<IShaderLanguage> IShaderLanguagePtr;

SE_NAMESPACE_END
//=============================================================================