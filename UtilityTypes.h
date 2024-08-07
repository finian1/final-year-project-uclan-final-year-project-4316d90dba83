#pragma once
namespace utility
{
	struct Vector2i
	{
		Vector2i(){};
		Vector2i(int x, int y)
		{
			X = x;
			Y = y;
		}

		int X = 0;
		int Y = 0;
	};

	struct Colour
	{
		Colour(float r, float g, float b)
		{
			R = r;
			G = g;
			B = b;
		}

		float R = 0.0f;
		float G = 0.0f;
		float B = 0.0f;
	};
}