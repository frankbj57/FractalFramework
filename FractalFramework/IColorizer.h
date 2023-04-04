#pragma once
#include "olcPixelGameEngine.h"

// Define an interface for the colorizing funtions
class IColorizer
{
public:
	virtual olc::Pixel ColorizePixel(int value) = 0;
	virtual olc::Pixel ColorizePixel(float value) = 0;

	float scale;

private:
};