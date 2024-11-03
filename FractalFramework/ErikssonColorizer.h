#pragma once
#include "IColorizer.h"
#include <cmath>


class ErikssonColorizer : public IColorizer
{
	const float pi = 3.141593f;
	const float pithird = pi / 3;

public:
	ErikssonColorizer() : pi(acosf(-1.0)), pithird(pi/3)
	{
		scale = 256.0;
	}

	olc::Pixel ColorizePixel(int value)
	{
		return ColorizePixel((float)value);
	}

	olc::Pixel ColorizePixel(float value)
	{
		// Thank you @Eriksonn - Wonderful Magic Fractal Oddball Man
		float scaledAngle = 2 * pi * value / scale;

		return olc::PixelF(0.5f * std::sinf(scaledAngle) + 0.5f, 0.5f * sinf(scaledAngle + 2*pithird) + 0.5f, 0.5f * sinf(scaledAngle + 4*pithird) + 0.5f);
	}

private:
};
