#pragma once

// For producing random numbers used in the asteroids test
class MyRandom
{
private:
public:
	MyRandom(unsigned seed = 1)
	{
		SetSeed(seed);
	}

	void SetSeed(unsigned seed)
	{
		srand(seed);
	}

	float GetUniformDistribution(float min, float max)
	{
		return min + float(rand()) / (float(RAND_MAX / (max - min)));
	}

	int GetUniformDistribution(int min, int max)
	{
		return (rand() % (max - min)) + min;
	}

	// Using polar method
	float GetNormalDistribution(float mean, float stdDev)
	{
		static bool  hasSpare = false;    // not thread-safe
		static float spare;
		float        u, v, s;

		if ( hasSpare )
		{
			hasSpare = false;
			return mean + stdDev * spare;
		}

		hasSpare = true;

		do
		{
			u = (rand() / ((float)RAND_MAX)) * 2.0f - 1.0f;
			v = (rand() / ((float)RAND_MAX)) * 2.0f - 1.0f;
			s = u * u + v * v;
		} while ( s >= 1 || s == 0 );

		s = sqrt((-2.0f * log(s)) / s);
		spare = v * s;
		return (mean + stdDev * u * s);
	}
};