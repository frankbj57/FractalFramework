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

};

// Define a base class, it's abstract
class Colorizer : public IColorizer
{
public:
	virtual void setScale(float scale_) { this->scale = scale_; }
	virtual float getScale() const { return scale; }

protected:
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

	virtual olc::Pixel ColorizePixel(int value) const override { return pCore->ColorizePixel(value); }
	virtual olc::Pixel ColorizePixel(float value) const override { return pCore->ColorizePixel(value); }

	void setScale(float scale) override { pCore->setScale(scale); }
	float getScale() const override { return pCore->getScale(); }

	virtual IColorizer* getCore() const override { return pCore; }
	virtual void setCore(IColorizer* pCore_) override { this->pCore = pCore_; }

protected:
	IColorizer* pCore;
};
