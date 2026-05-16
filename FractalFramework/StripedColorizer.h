#pragma once
#include "IColorizer.h"

class StripedColorizer : public ColorizerDecorator
{
public:
	StripedColorizer(IColorizer* pCore) : ColorizerDecorator(pCore) {}
	olc::Pixel ColorizePixelScaled(double value) const override 
	{ 
		if (static_cast<int>(value) % 2 == 1)
		{
			return olc::WHITE;
		}
		else
		{
			return pCore->ColorizePixelScaled(value);
		}
	}


private:

};