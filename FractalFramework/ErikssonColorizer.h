#pragma once
#include "IColorizer.h"
#include <cmath>

class ErikssonColorizer : public Colorizer
{
	static const float pi;
	static const float pithird;

public:
	ErikssonColorizer()
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

// const float ErikssonColorizer::pi = 3.141593f;
// const float ErikssonColorizer::pithird = pi / 3;
const float ErikssonColorizer::pi = acos(-1);
const float ErikssonColorizer::pithird = pi / 3;

