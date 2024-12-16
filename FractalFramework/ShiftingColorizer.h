#pragma once
#include "IColorizer.h"
#include <cmath>

struct ShiftingColorizer : public IColorizerDecorator
{
	int shift;
	void setShift(int shift)
	{
		float mod = fmodf(shift, pCore->getScale());
		this->shift = mod;
	}

	int getShift() const
	{
		return shift;
	}

	ShiftingColorizer(IColorizer* core) : IColorizerDecorator(core) {}
	olc::Pixel ColorizePixel(int value) override
	{
		int effValue = value + shift;
		effValue = fmodf(effValue, pCore->getScale());

		return pCore->ColorizePixel(effValue);
	}


};