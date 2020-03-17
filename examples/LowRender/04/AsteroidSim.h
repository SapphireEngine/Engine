#pragma once

#include "stdafx.h"
#include "Math/MathTypes.h"

using namespace se;

struct AsteroidStatic
{
	float4 rotationAxis;
	float4 surfaceColor;
	float4 deepColor;

	float scale;
	float orbitSpeed;
	float rotationSpeed;

	uint32_t textureID;
	uint32_t vertexStart;
	uint32_t padding[3];
};

struct AsteroidDynamic
{
	mat4     transform;
	uint32_t indexStart;
	uint32_t indexCount;
	uint32_t padding[2];
};

struct AsteroidSimulation
{
public:
	void Init(uint32_t rngSeed, uint32_t numAsteroids, uint32_t numMeshes, uint32_t vertexCountPerMesh, uint32_t textureCount);
	void Exit();
	void Update(float deltaTime, unsigned startIdx, unsigned endIdx, const vec3& cameraPosition);

	eastl::vector<AsteroidStatic>  asteroidsStatic;
	eastl::vector<AsteroidDynamic> asteroidsDynamic;
	//eastl::vector<Asteroid> asteroids;
	int*     indexOffsets;
	unsigned numLODs;
};