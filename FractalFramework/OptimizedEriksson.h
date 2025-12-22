#pragma once
#include "IColorizer.h"
#include <cmath>

class OptimizedErikssonColorizer : public Colorizer
{
	static const float pi;
	static const float sqrt3half;

public:
	OptimizedErikssonColorizer()
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

		// Optimized with trigonometric formulas from originally using 3 calls to sine
		// to use one call, a sqrt call and some simple math

		float scaledAngle = value / getScale();
		scaledAngle = scaledAngle - static_cast<int>(scaledAngle);
		if (scaledAngle < 0)
			scaledAngle += 1;
		scaledAngle *= 2 * pi;
		float sinx = std::sin(scaledAngle);
		float cosx = std::sqrt(1 - sinx * sinx);
		if (scaledAngle > pi / 2 && scaledAngle < 3 * pi / 2)
			cosx = -cosx;

		float red = sinx;
		float sinpart = -0.5f * sinx;
		float cospart = sqrt3half * cosx;
		float green = sinpart + cospart;
		float blue =  sinpart - cospart;

		return olc::PixelF(0.5f * red + 0.5f, 0.5f * green + 0.5f, 0.5f * blue + 0.5f);
	}

private:
};

const float OptimizedErikssonColorizer::pi = acos(-1.0f);
const float OptimizedErikssonColorizer::sqrt3half = sqrt(3.0f)/2.0f;

