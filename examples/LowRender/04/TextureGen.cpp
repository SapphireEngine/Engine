#include "TextureGen.h"
#include "NoiseOctaves.h"
#include "Random.h"

void genTextures(uint32_t texture_count, se::Texture** pOutTexture)
{
	static const int textureDim = 256;

	uint32_t array_count = 3;

	uint32_t* seeds = (uint32_t*)alloca(texture_count * sizeof(uint32_t));
	{
		for ( uint32_t i = 0; i < texture_count; ++i )
			seeds[i] = rand();
	}

	se::RawImageData rawData = {};

	uint32_t sliceSize = sizeof(unsigned char) * textureDim * textureDim * 4;
	rawData.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
	rawData.mWidth = textureDim;
	rawData.mHeight = textureDim;
	rawData.mDepth = 1;
	rawData.mMipLevels = 1;
	rawData.mArraySize = texture_count * array_count;

	se::TextureDesc desc = {};
	desc.mArraySize = rawData.mArraySize;
	desc.mFormat = rawData.mFormat;
	desc.mDepth = rawData.mDepth;
	desc.mWidth = rawData.mWidth;
	desc.mHeight = rawData.mHeight;
	desc.mMipLevels = rawData.mMipLevels;
	desc.mSampleCount = se::SAMPLE_COUNT_1;
	desc.mDescriptors = se::DESCRIPTOR_TYPE_TEXTURE;
	se::TextureLoadDesc textureDesc = {};
	textureDesc.pDesc = &desc;
	textureDesc.ppTexture = pOutTexture;
	addResource(&textureDesc, NULL, se::LOAD_PRIORITY_NORMAL);

	se::TextureUpdateDesc updateDesc = {};
	updateDesc.pTexture = *pOutTexture;
	updateDesc.pRawImageData = &rawData;
	beginUpdateResource(&updateDesc);

	for ( uint32_t t = 0; t < texture_count; ++t )
	{
		MyRandom rng(seeds[t]);

		for ( uint32_t a = 0; a < array_count; ++a )
		{
			float randomNoise = rng.GetUniformDistribution(0.0f, 10000.0f);
			float randomNoiseScale = rng.GetUniformDistribution(100.0f, 150.0f);
			float randomPersistence = rng.GetNormalDistribution(0.9f, 0.2f);

			// Use same parameters for each of the tri-planar projection planes/cube map faces/etc.
			float noiseScale = randomNoiseScale / float(textureDim);
			float persistence = randomPersistence;
			float seed = randomNoise;
			float strength = 1.5f;

			NoiseOctaves<4> textureNoise(persistence);

			uint32_t    slice = t * array_count + a;
			uint32_t	nextSliceInMem = sliceSize * slice;
			uint32_t*	scanline = (uint32_t*)((uint8_t*)updateDesc.pMappedData + nextSliceInMem);
			for ( size_t y = 0; y < textureDim; ++y )
			{
				for ( size_t x = 0; x < textureDim; ++x )
				{
					float c = textureNoise((float)x * noiseScale, (float)y * noiseScale, seed);
					c = max(0.0f, min(1.0f, (c - 0.5f) * strength + 0.5f));

					int32_t cr = (int32_t)(c * 255.0f);
					int32_t cg = (int32_t)(c * 255.0f);
					int32_t cb = (int32_t)(c * 255.0f);
					scanline[x] = (cr) << 16 | (cg) << 8 | (cb) << 0;
				}
				scanline += textureDim;
			}
		}
	}

	endUpdateResource(&updateDesc, NULL);
}