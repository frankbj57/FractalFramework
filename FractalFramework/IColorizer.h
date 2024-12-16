#pragma once
#include "olcPixelGameEngine.h"

// Define an interface for the colorizing funtions
class IColorizer
{
public:
	virtual olc::Pixel ColorizePixel(int value) = 0;
	virtual olc::Pixel ColorizePixel(float value) = 0;
	virtual void setScale(float scale) { this->scale = scale; }
	virtual float getScale() const { return scale; }

private:
	float scale;
};

class IColorizerDecorator : public IColorizer
{
public:
	IColorizerDecorator(IColorizer* pCore) : pCore(pCore) {}
	olc::Pixel ColorizePixel(int value) override { return pCore->ColorizePixel(value); }
	olc::Pixel ColorizePixel(float value) override { return pCore->ColorizePixel(value); }

	void setScale(float scale) override { pCore->setScale(scale); }
	float getScale() const override { return pCore->getScale(); }

	IColorizer* pCore;

protected:

};