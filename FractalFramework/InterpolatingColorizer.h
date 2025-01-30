#pragma once
#include "IColorizer.h"

class ColorUp : public Colorizer
{
public:
	olc::Pixel fromColor = olc::BLACK;
	olc::Pixel toColor = olc::WHITE;
	float fromValue = 0;
	float toValue = getScale();

	virtual olc::Pixel ColorizePixel(int value) const override
	{
		return ColorizePixel(float(value));
	}

	virtual olc::Pixel ColorizePixel(float value) const override
	{
		if (value <= fromValue)
			return fromColor;
		else if (value >= toValue)
			return toColor;
		else
		{
			float s = (value - fromValue) / (toValue - fromValue);
			float t = s;
			return fromColor * (1 - t) + toColor * t;
		}
	}

private:

};