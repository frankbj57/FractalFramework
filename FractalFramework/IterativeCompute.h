#pragma once

const double loopEpsilon = 1e-09;

struct IComputeState
{
	double cr, ci;
	double zr, zi;
	double zr2, zi2;

	virtual void Initialize(double constX, double constY, double initX, double initY)
	{
		cr = constX; ci = constY;
		zr = initX; zi = initY;
		zr2 = zr * zr; zi2 = zi * zi;
	}
	virtual void Advance() = 0;
	virtual IComputeState* Clone() = 0;
	virtual ~IComputeState() { }
};

struct IComputePoint
{
	std::unique_ptr<IComputeState> z;
	int maxIterations = 256;
	double bailOutSquare = 4.0;

	virtual int ComputePointCount(double x, double y, double initr = 0.0, double initi = 0.0) = 0;
	virtual IComputePoint* Clone() = 0;
	virtual ~IComputePoint() { }
};

struct MandelComputeState : public IComputeState
{
	// Logistic formula for complex numbers
	// z = z*z + c
	// Where z0 could be (0.0, 0.0), and c is the current point p
	// The formula has been optimized for faster computation
	// For Julia sets, z0 = p and c is the julia seed
	inline void Advance() override
	{
		zi = zr * zi * 2.0 + ci;
		zr = zr2 - zi2 + cr;

		zr2 = zr * zr;
		zi2 = zi * zi;
	}

	inline IComputeState* Clone() override
	{
		MandelComputeState* pR = new MandelComputeState(*this);

		return pR;
	}
};

struct BurningShipComputeState : public IComputeState
{
	// Logistic formula for complex numbers
	// z = (|zr| + i|zi|)^2 + c in mathematical short hand,
	// | | meaning the absolute value of the real and imaginary part of z respectively
	// Where z0 could be (0.0, 0.0), and c is the current point p
	// The formula has been optimized for faster computation
	// For Julia sets, z0 = p and c is the julia seed

	inline void Advance() override
	{
		zi = std::abs(zr * zi) * 2.0 + ci;
		zr = zr2 - zi2 + cr;

		zr2 = zr * zr;
		zi2 = zi * zi;
	}

	inline IComputeState* Clone() override
	{
		BurningShipComputeState* pR = new BurningShipComputeState(*this);

		return pR;
	}
};

struct LogisticComputeState : public IComputeState
{
	// Logistic formula for complex numbers
	// z = c*z*(1-z)
	// Where z0 could be (0.5, 0.0), and c is the current point p
	// The formula has been optimized for faster computation
	// For Julia sets, z0 = p and c is the julia seed
	inline void Advance() override
	{
		double fr = (zr - zr2 + zi2);
		double fi = (zi - 2 * zr * zi);

		zr = cr * fr - ci * fi;
		zi = ci * fr + cr * fi;

		zr2 = zr * zr;
		zi2 = zi * zi;
	}

	inline IComputeState* Clone() override
	{
		LogisticComputeState* pR = new LogisticComputeState(*this);

		return pR;
	}
};

struct ComputePoint : public IComputePoint
{
	inline int ComputePointCount(double x, double y, double initr = 0.0, double initi = 0.0) override
	{
		int n = 0;

		z->Initialize(x, y, initr, initi);

		while ((z->zr2 + z->zi2) < bailOutSquare && n < maxIterations)
		{
			z->Advance();
			n++;
		}

		return n;
	}

	inline IComputePoint* Clone() override
	{
		ComputePoint* pR = new ComputePoint;

		assert(z);

		pR->z.reset(z->Clone());
		pR->maxIterations = maxIterations;
		pR->bailOutSquare = bailOutSquare;

		return pR;
	}

	~ComputePoint() override
	{
	}
};

struct ComputePointWithLoop : public IComputePoint
{
	inline int ComputePointCount(double x, double y, double initr = 0.0, double initi = 0.0) override
	{
		int n = 0;

		z->Initialize(x, y, initr, initi);

		std::unique_ptr<IComputeState> ztail(z->Clone());

		bool loops = false;
		while ((z->zr2 + z->zi2) < bailOutSquare && n < maxIterations && !loops)
		{
			z->Advance();
			if (n & 0x1)
				ztail->Advance();
			// loops = z->zr == ztail->zr && z->zi == ztail->zi;
			loops = std::abs(z->zr - ztail->zr) < loopEpsilon && std::abs(z->zi - ztail->zi) < loopEpsilon;
			n++;
		}

		if (loops)
		{
			// We are looping, calculate loop length
			*ztail = *z;
			z->Advance();
			int loop = 1;
			// while (z->zr != ztail->zr || z->zi != ztail->zi)
			// When zooming very deep, calculations are not exact, so check again for maximum
			while (!(std::abs(z->zr - ztail->zr) < loopEpsilon && std::abs(z->zi - ztail->zi) < loopEpsilon) && loop < maxIterations)
			{
				z->Advance();
				loop++;
			}
			return maxIterations + loop;
		}
		else if (n >= maxIterations)
		{
			return maxIterations;
		}
		else
		{
			return n;
		}
	}

	inline IComputePoint* Clone() override
	{
		ComputePointWithLoop* pR = new ComputePointWithLoop;

		assert(z);

		pR->z.reset(z->Clone());
		pR->maxIterations = maxIterations;
		pR->bailOutSquare = bailOutSquare;

		return pR;
	}

	~ComputePointWithLoop() override
	{
	}
};


struct ComputePointWithConvergence : public IComputePoint
{
	inline int ComputePointCount(double x, double y, double initr = 0.0, double initi = 0.0) override
	{
		int n = 0;

		z->Initialize(x, y, initr, initi);

		std::unique_ptr<IComputeState> ztail(z->Clone());

		bool loops = false;
		while ((z->zr2 + z->zi2) < bailOutSquare && n < maxIterations && !loops)
		{
			z->Advance();
			if (n & 0x1)
				ztail->Advance();
			// loops = z->zr == ztail->zr && z->zi == ztail->zi;
			loops = std::abs(z->zr - ztail->zr) < loopEpsilon && std::abs(z->zi - ztail->zi) < loopEpsilon;
			n++;
		}

		if (loops)
		{
			// We are looping, calculate convergence time
			// Since zTail is moving half the speed of z, convergence time is half the count
			// For now, merge interation count and convergence count
			return maxIterations + n / 2;
		}
		else if (n >= maxIterations)
		{
			return maxIterations;
		}
		else
		{
			return n;
		}
	}

	inline IComputePoint* Clone() override
	{
		ComputePointWithConvergence* pR = new ComputePointWithConvergence();

		assert(z);

		pR->z.reset(z->Clone());
		pR->maxIterations = maxIterations;
		pR->bailOutSquare = bailOutSquare;

		return pR;
	}

	~ComputePointWithConvergence() override
	{
	}
};

