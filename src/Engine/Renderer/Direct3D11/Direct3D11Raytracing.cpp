#include "stdafx.h"
#ifdef DIRECT3D11
#include "Renderer/IRay.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
bool isRaytracingSupported(Renderer*)
{
	return false;
}

bool initRaytracing(Renderer*, Raytracing**)
{
	return false;
}

void removeRaytracing(Renderer*, Raytracing*)
{
}

void addAccelerationStructure(Raytracing*, const AccelerationStructureDescTop*, AccelerationStructure**)
{
}
void removeAccelerationStructure(Raytracing*, AccelerationStructure*)
{
}

void addRaytracingRootSignature(Raytracing*, const ShaderResource*, uint32_t, bool, RootSignature**, const RootSignatureDesc*)
{
}

void addRaytracingShaderTable(Raytracing*, const RaytracingShaderTableDesc*, RaytracingShaderTable**)
{
}
void removeRaytracingShaderTable(Raytracing*, RaytracingShaderTable*)
{
}

void cmdBuildAccelerationStructure(Cmd*, Raytracing*, RaytracingBuildASDesc*)
{
}
void cmdDispatchRays(Cmd*, Raytracing*, const RaytracingDispatchDesc*)
{
}
SE_NAMESPACE_END
//=============================================================================

#endif