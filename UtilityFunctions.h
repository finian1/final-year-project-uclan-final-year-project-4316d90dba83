#pragma once
#include "Globals.h"
namespace utility
{
	/// <summary>
	/// Returns random float between min (inclusive) and max (inclusive)
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	static float RandomNumber(float min, float max)
	{
		float shift = 0.0f;
		if (min < 0.0f)
		{
			shift = abs(min);
			max += abs(min);
			min = 0.0f;
		}
		int iMin = min * 10000;
		int iMax = max * 10000;
		/*float diff = max - min;
		float num = ((float)rand() / (RAND_MAX + 1));
		float posVal = diff * num;
		float shiftedVal = posVal + min;*/
		std::uniform_int_distribution dist(iMin, iMax);
		float val = (float)dist(gRandomNumberGen) / 10000.0f;
		return val - shift;
	}

	/// <summary>
	/// Returns random integer between min (inclusive) and max (inclusive)
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	static int RandomNumber(int min, int max)
	{
		if (min == max)
		{
			return min;
		}
		int shift = 0;
		if (min < 0)
		{
			shift = abs(min);
			max += abs(min);
			min = 0;
		}

		std::uniform_int_distribution dist(min, max);

		return dist(gRandomNumberGen) - shift;
	}

	static float Sigmoid(float val)
	{
		return 1 / (1 + exp(-val));
	}
}