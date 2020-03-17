#pragma once

/** 1D, 2D, 3D and 4D float Perlin simplex noise
*/
#ifdef __cplusplus
extern "C"
{
#endif

	float snoise1(float x);
	float snoise2(float x, float y);
	float snoise3(float x, float y, float z);
	float snoise4(float x, float y, float z, float w);

#ifdef __cplusplus
}
#endif