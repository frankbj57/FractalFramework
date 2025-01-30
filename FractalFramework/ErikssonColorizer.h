#pragma once
#include "IColorizer.h"
#include <cmath>


class ErikssonColorizer : public Colorizer
{
	const float pi = 3.141593f;
	const float pithird = pi / 3;

public:
	ErikssonColorizer() : pi(acosf(-1.0)), pithird(pi/3)
	{
		setScale(256.0);
	}

	olc::Pixel ColorizePixel(int value) const override
	{
		return ColorizePixel((float)value);
	}

	olc::Pixel ColorizePixel(float value) const override
	{
		// Thank you @Eriksonn - Wonderful Magic Fractal Oddball Man
		float scaledAngle = 2 * pi * value / getScale();

		return olc::PixelF(0.5f * std::sin(scaledAngle) + 0.5f, 0.5f * sin(scaledAngle + 2*pithird) + 0.5f, 0.5f * sin(scaledAngle + 4*pithird) + 0.5f);
	}

private:
};
