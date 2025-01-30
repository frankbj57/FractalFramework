#pragma once
#include "olcPixelGameEngine.h"

// Define an interface for the colorizing funtions
class IColorizer
{
public:
	virtual olc::Pixel ColorizePixel(int value) const = 0;
	virtual olc::Pixel ColorizePixel(float value) const = 0;
	virtual void setScale(float scale) = 0;
	virtual float getScale() const = 0;

private:
	float scale;
};

// Define a base class, it's abstract
class Colorizer : public IColorizer
{
public:
	virtual void setScale(float scale) { this->scale = scale; }
	virtual float getScale() const { return scale; }

private:
	float scale;
};

// Declare an interface for colorizer decorators
class IColorizerDecorator : public IColorizer
{
public:
	virtual IColorizer* getCore() const = 0;
	virtual void setCore(IColorizer* core) = 0;

protected:
};

// Declare a base class colorizer decorators
class ColorizerDecorator : public IColorizerDecorator
{
public:
	ColorizerDecorator(IColorizer* pCore) : pCore(pCore) {}

	virtual olc::Pixel ColorizePixel(int value) const { return pCore->ColorizePixel(value); }
	virtual olc::Pixel ColorizePixel(float value) const { return pCore->ColorizePixel(value); }

	void setScale(float scale) override { pCore->setScale(scale); }
	float getScale() const override { return pCore->getScale(); }

	virtual IColorizer* getCore() const { return pCore; }
	virtual void setCore(IColorizer* pCore) { this->pCore = pCore; }

protected:
	IColorizer* pCore;
};
