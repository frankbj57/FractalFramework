#pragma once
#include "IColorizer.h"
#include <cmath>

struct ShiftingColorizer : public ColorizerDecorator
{
	int shift;
	void setShift(int shift)
	{
		float mod = fmodf(shift, pCore->getScale());
		this->shift = (int) mod;
	}

	int getShift() const
	{
		return shift;
	}

	ShiftingColorizer(IColorizer* pCore) : ColorizerDecorator(pCore), shift(0) {}

	olc::Pixel ColorizePixel(int value) const override
	{
		int effValue = value + shift;
		effValue = fmodf(effValue, pCore->getScale());

		return pCore->ColorizePixel(effValue);
	}


};