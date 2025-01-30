#pragma once
#include "IColorizer.h"

class StripedColorizer : public ColorizerDecorator
{
public:
	StripedColorizer(IColorizer* pCore) : ColorizerDecorator(pCore) {}
	olc::Pixel ColorizePixel(int value) const override 
	{ 
		if (value % 2 == 1)
		{
			return olc::WHITE;
		}
		else
		{
			return pCore->ColorizePixel(value);
		}
	}


private:

};