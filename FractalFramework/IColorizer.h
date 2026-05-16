#pragma once
#include "olcPixelGameEngine.h"

// Define an interface for the colorizing funtions
class IColorizer
{
public:
	virtual olc::Pixel ColorizePixelScaled(double value) const = 0;
	virtual olc::Pixel ColorizePixelNormalized(double value) const = 0;
	virtual void setScale(double scale) = 0;
	virtual double getScale() const = 0;

private:

};

// Define a base class, it's abstract
class Colorizer : public IColorizer
{
public:
	virtual void setScale(double scale_) { this->scale = scale_; }
	virtual double getScale() const { return scale; }

protected:
	double scale = 255;
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

	virtual olc::Pixel ColorizePixelScaled(double value) const override { return pCore->ColorizePixelScaled(value); }
	virtual olc::Pixel ColorizePixelNormalized(double value) const override { return pCore->ColorizePixelNormalized(value); }

	void setScale(double scale) override { pCore->setScale(scale); }
	double getScale() const override { return pCore->getScale(); }

	virtual IColorizer* getCore() const override { return pCore; }
	virtual void setCore(IColorizer* pCore_) override { this->pCore = pCore_; }

protected:
	IColorizer* pCore;
};

IColorizer & operator|(IColorizerDecorator &left, IColorizer &right)
{
	left.setCore(&right);
	return left;
}
