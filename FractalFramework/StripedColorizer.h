#pragma once
#include "IColorizer.h"

class StripedColorizer : public IColorizerDecorator
{
public:
	StripedColorizer(IColorizer* core) : IColorizerDecorator(core) {}
	olc::Pixel ColorizePixel(int value) override 
	{ 
		if (value % 2 == 1)
		{
			return olc::WHITE;
		}
		else
		{
			return pCore->ColorizePixel(value / 2);
		}
	}


private:

};