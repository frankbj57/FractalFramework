#pragma once
#include "IColorizer.h"

class ColorUp : public Colorizer
{
public:
	olc::Pixel fromColor = olc::BLACK;
	olc::Pixel toColor = olc::WHITE;
	float fromValue = 0;
	float toValue = getScale();

	virtual olc::Pixel ColorizePixelScaled(double value) const override
	{
		double s = (value - fromValue) / (toValue - fromValue);
		return ColorizePixelNormalized(s);
	}

	virtual olc::Pixel ColorizePixelNormalized(double value) const override
	{
		// Value range 0.0 .. 1.00, wrap around
		value = value - static_cast<int>(value);
		if (value < 0.0)
			value += 1.0;

		double t = value;
		return fromColor * (1 - t) + toColor * t;
	}

private:

};