#pragma once
#include "IColorizer.h"
#include <cmath>

struct ShiftingColorizer : public ColorizerDecorator
{
	double shift;
	void setShift(double shift_)
	{
		shift = shift_;
	}

	double getShift() const
	{
		return shift;
	}

	ShiftingColorizer(IColorizer* pCore) : ColorizerDecorator(pCore), shift(0) {}

	olc::Pixel ColorizePixelScaled(double value) const override
	{
		double effValue = value + shift;

		return pCore->ColorizePixelScaled(effValue);
	}


};