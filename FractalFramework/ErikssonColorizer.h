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
	}

	olc::Pixel ColorizePixelScaled(double value) const override
	{
		return ColorizePixelNormalized(value/getScale());
	}

	olc::Pixel ColorizePixelNormalized(double value) const override
	{
		// Thank you @Eriksonn - Wonderful Magic Fractal Oddball Man
		float scaledAngle = (float) (2 * pi * value);

		return olc::PixelF(0.5f * std::sin(scaledAngle) + 0.5f, 0.5f * sin(scaledAngle + 2*pithird) + 0.5f, 0.5f * sin(scaledAngle + 4*pithird) + 0.5f);
	}

private:
};

// const float ErikssonColorizer::pi = 3.141593f;
// const float ErikssonColorizer::pithird = pi / 3;
const float ErikssonColorizer::pi = acos(-1.0f);
const float ErikssonColorizer::pithird = pi / 3;

