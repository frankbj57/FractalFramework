#pragma once
#include "IColorizer.h"

class ColorUp : public IColorizer
{
public:
	olc::Pixel fromColor = olc::BLACK;
	olc::Pixel toColor = olc::WHITE;
	double fromValue = 0;
	double toValue = scale;

	virtual olc::Pixel ColorizePixel(int value) override
	{
		return ColorizePixel(float(value));
	}

	virtual olc::Pixel ColorizePixel(float value) override
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