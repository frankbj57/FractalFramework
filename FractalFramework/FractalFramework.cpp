/*
	Fractal Viewing framework,
	based on Javidx9's:
	Brute Force Processing a Mandelbrot Renderer
	"Dammit Moros & Saladin, you guys keep making tools, I'll have nothing left to video..." - javidx9

	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018-2020 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Relevant Video: https://youtu.be/PBvLs88hvJ8

	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
				https://www.youtube.com/javidx9extra
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Patreon:	https://www.patreon.com/javidx9
	Homepage:	https://www.onelonecoder.com

	Community Blog: https://community.onelonecoder.com

	Author
	~~~~~~
	David Barr, aka javidx9, ©OneLoneCoder 2018, 2019, 2020

	Further development by
	Frank B. Jakobsen, © 2023
	This is a refactoring of the original application
	to use and demonstrate refactoring of the separate steps
	to calculate and view iteration function based objects, sometimes showing
	up as fractals.

*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_TRANSFORMEDVIEW_IMPLEMENTATION
#include "olcPGEX_TransformedViewTemplate.h"


#if defined(_MSC_VER)
#include <ppl.h>
#endif

#include <algorithm>
#include <execution>
#include <numeric>

#include <cassert>

#include "CircularBufferTemplate.h"
#include "ErikssonColorizer.h"
#include "OptimizedEriksson.h"
#include "StripedColorizer.h"
#include "InterpolatingColorizer.h"
#include "ShiftingColorizer.h"

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
	virtual ~IComputeState() {}
};

struct IComputePoint
{
	std::unique_ptr<IComputeState> z;
	int maxIterations = 256;
	double bailOutSquare = 4.0;

	virtual int ComputePointCount(double x, double y, double initr = 0.0, double initi = 0.0) = 0;
	virtual IComputePoint* Clone() = 0;
	virtual ~IComputePoint() {}
};

class FractalFramework : public olc::PixelGameEngine
{
public:
	FractalFramework() 
		: 
		stripedColorizer(&eColorizer),
		shiftColorizer(&eColorizer)
	{
		sAppName = "Fractal Framework";
	}

	int* pFractal = nullptr;  // Result buffer - matches screen size
	int nMode = 2;
	int nIterations = 256;  // Classic fractal maximum interation
	double bailoutSquared = 4.0;  // Classic fractal bailout value, squared for easier calculations
	olc::vd2d juliaSeed{ -1, 0 }; // Current constant for julia set calculations
	olc::vd2d z0Value{ 0,0 };   // Current startvalue if not julia calculation

	bool julia = false;
	bool striped = false;  // Use striped colorization
	bool loopCheck = false;
	bool shifting = false;
	int shiftValue = 0;
	float shiftSpeed = 8; // Values per second
	float shiftTime = 0;

	bool recalculate = true;

	std::unique_ptr<IComputeState> m_pCurrentStateAlgorithm;
	std::unique_ptr<IComputePoint> m_pCurrentPointAlgorithm;




public:
	bool OnUserCreate() override
	{
		pFractal = new int[ScreenWidth() * ScreenHeight()] { 0 };

		// Using Vector extensions, align memory (not as necessary as it used to be)
		// MS Specific - see std::aligned_alloc for others
		// pFractal = (int*)_aligned_malloc(size_t(ScreenWidth()) * size_t(ScreenHeight()) * sizeof(int), 64);

		eColorizer.setScale(nIterations);
		oeColorizer.setScale(nIterations);

		// effectiveColorizer = &eColorizer;
		colorUpColorizer.fromColor = olc::RED;
		colorUpColorizer.toColor = olc::GREEN;
		colorUpColorizer.fromValue = 1;
		colorUpColorizer.toValue = 256;

		basicColorizer = &oeColorizer;

		// Original viewport should be x: 0 to 2, y: 0 to 1. Screencoordinates y are opposite
		float xscale = ScreenWidth() / (4);
		float yscale = ScreenHeight() / (2);
		float scale = std::min(xscale, yscale);

		tv.Initialise(
			{ ScreenWidth(), ScreenHeight() },
			{ scale, -scale });
		

		tv.SetWorldOffset(-tv.ScreenToWorld({ (float) ScreenWidth()/2, (float) ScreenHeight()/2 }));

		m_pCurrentStateAlgorithm.reset(new MandelComputeState);

		loopCheck = false;

		// List commands
		std::cout << "Available commands:" << std::endl;

		for (const auto &c : KeyCommands)
		{
			std::cout << c.keyName << ":\t" << c.commandDescription << endl;
		}

		Colorizers.push_back(colorizer_s{ "Optimized Eriksson", &oeColorizer });
		Colorizers.push_back(colorizer_s{ "Eriksson", &eColorizer });
		Colorizers.push_back(colorizer_s{ "Color Up", &colorUpColorizer });

		recalculate = true;

		return true;
	}

	bool OnUserDestroy() override
	{
		// Clean up memory
		// _aligned_free(pFractal);
		if (currentHelperThread)
		{
			// Stop the current calculation
			stopCalculation = true;

			currentHelperThread.get()->join();
		}

		currentHelperThread.release();

		delete pFractal;
		return true;
	}

	struct MandelComputeState : public IComputeState
	{
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
				loops = z->zr == ztail->zr && z->zi == ztail->zi;
				if (n & 0x1)
					ztail->Advance();
				n++;
			}

			if (loops)
			{
				// We are looping, calculate loop length
				*ztail = *z;
				z->Advance();
				int loop = 1;
				while (z->zr != ztail->zr || z->zi != ztail->zi)
				{
					z->Advance();
					loop++;
				}
				return loop;
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

	// New parallel method, using OpenMP
	void CreateFractalOpenMP(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations)
	{
		const double x_scale = (frac_br.x - frac_tl.x) / (double(pix_br.x) - double(pix_tl.x));
		const double y_scale = (frac_br.y - frac_tl.y) / (double(pix_br.y) - double(pix_tl.y));

		const int row_size = ScreenWidth();

		int y;


#pragma omp parallel
#pragma omp for schedule(dynamic, 1) nowait
		for (y = pix_tl.y; y < pix_br.y; y++)
		{
			double x_pos = frac_tl.x;
			const double y_pos = frac_tl.y + y * y_scale;

			const int y_offset = y * row_size;

			int x, n;

			// We need a copy for each parallel task, possibly down to each y coordinate
			std::unique_ptr<IComputePoint> comPoint(m_pCurrentPointAlgorithm->Clone());

			for (x = pix_tl.x; x < pix_br.x && !stopCalculation; x++)
			{
				if (julia)
				{
					n = comPoint->ComputePointCount(juliaSeed.x, juliaSeed.y, x_pos, y_pos);
				}
				else
				{
					n = comPoint->ComputePointCount(x_pos, y_pos, z0Value.x, z0Value.y);
				}

				pFractal[y_offset + x] = n;
				x_pos += x_scale;
			}
		}
	}
#if defined(_MSC_VER)
	// _MSC_VER is also defined for clang under VS (clang-cl)
	// Using concurrency library parallelization
	void CreateFractalParallelization(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations)
	{
		const double x_scale = (frac_br.x - frac_tl.x) / (double(pix_br.x) - double(pix_tl.x));
		const double y_scale = (frac_br.y - frac_tl.y) / (double(pix_br.y) - double(pix_tl.y));

		const int row_size = ScreenWidth();

		concurrency::parallel_for(int(pix_tl.y), int(pix_br.y), [&](int y)
			{
				double x_pos = frac_tl.x;
				const double y_pos = frac_tl.y + y * y_scale;

				const int y_offset = y * row_size;

				int x, n;

				// We need a copy for each parallel task, possibly down to each y coordinate
				std::unique_ptr<IComputePoint> comPoint(m_pCurrentPointAlgorithm->Clone());

				for (x = pix_tl.x; x < pix_br.x && !stopCalculation; x++)
				{
					if (julia)
					{
						n = comPoint->ComputePointCount(juliaSeed.x, juliaSeed.y, x_pos, y_pos);
					}
					else
					{
						n = comPoint->ComputePointCount(x_pos, y_pos, z0Value.x, z0Value.y);
					}

					pFractal[y_offset + x] = n;
					x_pos += x_scale;
				}
			});
	}
#endif

	// Using built C++17 parallelization
	void CreateFractalCppForEachAlgorithm(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations)
	{
		const double x_scale = (frac_br.x - frac_tl.x) / (double(pix_br.x) - double(pix_tl.x));
		const double y_scale = (frac_br.y - frac_tl.y) / (double(pix_br.y) - double(pix_tl.y));

		const int row_size = ScreenWidth();

		std::vector<int> indexes(int(pix_br.y - pix_tl.y));
		std::iota(indexes.begin(), indexes.end(), 0);

		std::for_each_n(std::execution::par, indexes.begin(), int(pix_br.y - pix_tl.y), [&](int y)
			{
				double x_pos = frac_tl.x;
				const double y_pos = frac_tl.y + y * y_scale;

				const int y_offset = y * row_size;

				int x, n;

				// We need a copy for each parallel task, possibly down to each y coordinate
				std::unique_ptr<IComputePoint> comPoint(m_pCurrentPointAlgorithm->Clone());

				for (x = pix_tl.x; x < pix_br.x &&!stopCalculation; x++)
				{
					if (julia)
					{
						n = comPoint->ComputePointCount(juliaSeed.x, juliaSeed.y, x_pos, y_pos);
					}
					else
					{
						n = comPoint->ComputePointCount(x_pos, y_pos, z0Value.x, z0Value.y);
					}

					pFractal[y_offset + x] = n;
					x_pos += x_scale;
				}
			});
	}

	// Using single thread
	void CreateFractalSingleThread(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations)
	{
		const double x_scale = (frac_br.x - frac_tl.x) / (double(pix_br.x) - double(pix_tl.x));
		const double y_scale = (frac_br.y - frac_tl.y) / (double(pix_br.y) - double(pix_tl.y));

		const int row_size = ScreenWidth();

		int y;
		// We only need one for the whole picture
		std::unique_ptr<IComputePoint> comPoint(m_pCurrentPointAlgorithm->Clone());

		for (y = pix_tl.y; y < pix_br.y && !stopCalculation; y++)
		{
			double x_pos = frac_tl.x;
			const double y_pos = frac_tl.y + y * y_scale;

			const int y_offset = y * row_size;

			int x, n;

			for (x = pix_tl.x; x < pix_br.x && !stopCalculation; x++)
			{
				if (julia)
				{
					n = comPoint->ComputePointCount(juliaSeed.x, juliaSeed.y, x_pos, y_pos);
				}
				else
				{
					n = comPoint->ComputePointCount(x_pos, y_pos, z0Value.x, z0Value.y);
				}

				pFractal[y_offset + x] = n;
				x_pos += x_scale;
			}
		}
	}

	std::atomic<bool> stopCalculation;
	std::atomic<bool> calculationCompleted;

	std::unique_ptr<std::thread> currentHelperThread;

	void ThreadFunction(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations)
	{

		// START TIMING
		auto tp1 = std::chrono::high_resolution_clock::now();

		// Do the computation
		// Select the right method from the Create Methods table
		(this->*Methods[nMode].pCreateMethod)(pix_tl, pix_br, frac_tl, frac_br, nIterations);

		// STOP TIMING
		auto tp2 = std::chrono::high_resolution_clock::now();
		elapsedTime = tp2 - tp1;

		calculationCompleted = true;
	}

	std::chrono::duration<double> elapsedTime;

	using CreateFractalFunction = void(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations);

	struct method_s {
		olc::Key key;
		CreateFractalFunction FractalFramework::* pCreateMethod;
		string description;
	};

	static const std::vector<method_s> Methods;

	using KeyCommandFunction = bool(olc::Key);

	struct key_command_s {
		olc::Key key;
		std::string keyName;
		std::string commandDescription;
		KeyCommandFunction FractalFramework::* pKeyCommandFunction;
	};

	static const std::vector<FractalFramework::key_command_s> KeyCommands;

	struct colorizer_s
	{
		std::string description;
		IColorizer * pColorizer;
	};

	std::vector<colorizer_s> Colorizers;

	vector<olc::vf2d> track;
	olc::vf2d prevMousPos;
	int loopLength = 0;
	int currentColorizer = 0;

	bool ToggleStripes(olc::Key)
	{
		// Toggle striped
		striped = !striped;

		return true;
	}

	bool ToggleLoopDetection(olc::Key)
	{
		// Toggle loopcheck
		loopCheck = !loopCheck;
		if (loopCheck)
			m_pCurrentPointAlgorithm.reset(new ComputePointWithLoop);
		else
			m_pCurrentPointAlgorithm.reset(new ComputePoint);

		recalculate |= true;

		return true;
	}

	bool ToggleAnimateColors(olc::Key)
	{
		// Toggle shifting of colors
		shifting = !shifting;
		if (shifting)
		{
			shiftColorizer.setShift(0);
			shiftTime = 0;
		}
		else
		{
			// Do nothing
		}

		return true;
	}

	bool ToggleJulia(olc::Key)
	{
		// Toggle julia state
		julia = !julia;

		recalculate |= true;

		return true;
	}

	bool CycleColorizer(olc::Key)
	{
		if (Colorizers.size())
		{
			currentColorizer++;
			if (currentColorizer >= Colorizers.size())
			{
				currentColorizer = 0;
			}

			basicColorizer = Colorizers[currentColorizer].pColorizer;
		}

		return true;
	}

	bool AdjustIterations(olc::Key key)
	{
		switch (key)
		{
		case olc::Key::UP:
			{
				nIterations += 64;
			}
			break;

		case olc::Key::DOWN:
			{
				nIterations -= 64;
			}
			break;

			default:
				break;
		}

		if (nIterations < 64)
		{
			nIterations = 64;
		}

		recalculate |= true;

		return true;
	}

	bool ExitProgram(olc::Key)
	{
		return false;
	}

	bool SelectFunction(olc::Key key)
	{
		switch (key)
		{
		case olc::Key::M:
			{
				m_pCurrentStateAlgorithm.reset(new MandelComputeState);
				z0Value = { 0,0 };
				bailoutSquared = 4;
				recalculate |= true;
		}
			break;

		case olc::Key::B:
			{
				m_pCurrentStateAlgorithm.reset(new BurningShipComputeState);
				z0Value = { 0,0 };
				bailoutSquared = 4;
				recalculate |= true;
			}
			break;

		case olc::Key::G:
			{
				m_pCurrentStateAlgorithm.reset(new LogisticComputeState);
				z0Value = { 0.5,0 };
				bailoutSquared = 16;
				recalculate |= true;
			}
			break;

		default:
			break;
		}

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		auto oldOffSet = tv.GetWorldOffset();
		auto oldScale = tv.GetWorldScale();

		// Handle transform control
		tv.HandlePanAndZoom(olc::Mouse::MIDDLE, 0.2, true, true);

		if (oldOffSet != tv.GetWorldOffset() || oldScale != tv.GetWorldScale())
			recalculate |= true;

		olc::vi2d pix_tl = { 0,0 };
		olc::vi2d pix_br = { ScreenWidth(), ScreenHeight() };
		olc::vd2d frac_tl = { -2.0, -1.0 };
		olc::vd2d frac_br = { 1.0, 1.0 };

		frac_tl = tv.ScreenToWorld(pix_tl);
		frac_br = tv.ScreenToWorld(pix_br);

		// Handle User Input
		// Determine overall algorithm
		for (size_t i = 0; i < Methods.size(); i++)
		{
			if (GetKey(Methods[i].key).bPressed)
			{
				if (nMode != i)
				{
					nMode = i;
					recalculate |= true;
				}
				break;
			}
		}

		for (const auto & c : KeyCommands)
		{
			if (GetKey(c.key).bPressed)
			{
				if (!(this->*c.pKeyCommandFunction)(c.key))
					return false;
			}
		}

		olc::vf2d pos = GetMousePos();
		if (GetMouse(olc::Mouse::RIGHT).bPressed && !julia)
		{
			juliaSeed = tv.ScreenToWorld(pos);
		}

		if (GetMouse(olc::Mouse::LEFT).bPressed || (GetMouse(olc::Mouse::LEFT).bHeld && pos != prevMousPos))
		{
			// Calculate orbit for the selected point at the mouse
			prevMousPos = pos;
			track.clear();
			pos = tv.ScreenToWorld(pos);
			std::unique_ptr<IComputeState> pz(m_pCurrentStateAlgorithm->Clone());

			pz->Initialize(pos.x, pos.y, z0Value.x, z0Value.y);

			std::unique_ptr<IComputeState> pztail(pz->Clone());
			pz->Advance();
			int i = 1;
			bool loops = false;
			while ((pz->zr2 + pz->zi2) < bailoutSquared && i < nIterations && !loops)
			{
				track.push_back({ (float)pz->zr, (float)pz->zi });
				pz->Advance();
				// loops = pz->zr == pztail->zr && pz->zi == pztail->zi;
				loops = std::abs(pz->zr - pztail->zr) < 1e-6 && std::abs(pz->zi- pztail->zi) < 1e-6;
				if (i & 0x1)
					pztail->Advance();
				i++;
			}
			loopLength = 0;
			if (loops)
			{
				// Keep z fixed
				*pztail = *pz;
				pztail->Advance();
				loopLength = 1;
				//while (pz->zr != pztail->zr && pz->zi != pztail->zi)
				while (!(std::abs(pz->zr - pztail->zr) < 1e-6 && std::abs(pz->zi - pztail->zi) < 1e-6))
				{
					loopLength++;
					pztail->Advance();
				}
			}
		}
		else if (GetMouse(0).bReleased)
		{
			track.clear();
			loopLength = 0;
		}

		if (recalculate)
		{
			if (currentHelperThread)
			{
				// Stop the current calculation
				stopCalculation = true;

				currentHelperThread.get()->join();
			}

			// Safe area, where globals can be changed
			stopCalculation = false;
			calculationCompleted = false;
			if (loopCheck)
				m_pCurrentPointAlgorithm.reset(new ComputePointWithLoop);
			else
				m_pCurrentPointAlgorithm.reset(new ComputePoint);

			m_pCurrentPointAlgorithm->z.reset(m_pCurrentStateAlgorithm->Clone());
			m_pCurrentPointAlgorithm->maxIterations = nIterations;
			m_pCurrentPointAlgorithm->bailOutSquare = bailoutSquared;


			currentHelperThread.reset(new std::thread { &FractalFramework::ThreadFunction, this, pix_tl, pix_br, frac_tl, frac_br, nIterations });
			
			recalculate = false;
		}

		// Render result to screen
		// effectiveColorizer->scale = nIterations;
		effectiveColorizer = basicColorizer;
		if (striped)
		{
			stripedColorizer.setCore(effectiveColorizer);
			effectiveColorizer = &stripedColorizer;
		}

		if (shifting)
		{
			shiftTime += fElapsedTime;
			if (shiftTime > (1.0 / shiftSpeed))
			{
				shiftColorizer.setShift(shiftColorizer.getShift()+1);
				shiftTime = fmodf(shiftTime, (1.0 / shiftSpeed));
			}

			shiftColorizer.setCore(effectiveColorizer);
			effectiveColorizer = &shiftColorizer;
		}

		for (int y = 0; y < ScreenHeight(); y++)
		{
			for (int x = 0; x < ScreenWidth(); x++)
			{
				int i = pFractal[y * ScreenWidth() + x];
				if (i == nIterations)
				{
					Draw(x, y, olc::BLACK);
				}
				else
				{
					Draw(x, y,
						effectiveColorizer->ColorizePixel(i));
				}
			}
		}

		if (track.size() > 1)
		{
			for (int i = 0; i < track.size() - 1; i++)
			{
				// Warning - it takes a long time to draw a line which is wholly or partially outside the window!
				olc::vf2d screen1 = tv.WorldToScreen(track[i]);
				olc::vf2d screen2 = tv.WorldToScreen(track[i + 1]);
				// If both points are outside screen, don't draw the line
				if ((screen1.x < 0 || screen1.x >= ScreenWidth() || screen1.y < 0 || screen1.y > ScreenHeight())
					&&
					(screen2.x < 0 || screen2.x >= ScreenWidth() || screen2.y < 0 || screen2.y > ScreenHeight()))
				{
					// Don't do anything
				}
				else
					DrawLine(screen1, screen2);
			}
		}

		// Render UI
		uint32_t scale = 1;
		int32_t lineDistance = 10;

		DrawString(0, 0, std::to_string(nMode + 1) + ") " + Methods[nMode].description, olc::WHITE, scale);

		DrawString(0, 1 * scale * lineDistance, "Time Taken: " + std::to_string(elapsedTime.count()) + "s", olc::WHITE, scale);
		// DrawString(0, 1 * scale * lineDistance, "Time Taken: " + std::to_string(elapseBuffer.meanValue().count()) + "s", olc::WHITE, scale);
		DrawString(0, 2 * scale * lineDistance, "Iterations: " + std::to_string(m_pCurrentPointAlgorithm->maxIterations), olc::WHITE, scale);
		
		if (track.size() > 1)
		{
			DrawString(0, 3 * scale * lineDistance, "Track length: " + std::to_string(track.size()), olc::WHITE, scale);
			DrawString(0, 4 * scale * lineDistance, "Loop length: " + std::to_string(loopLength), olc::WHITE, scale);
		}

		// Exit program when returning false
		return !(GetKey(olc::Key::ESCAPE).bPressed);
	}


	olc::TransformedView tv;

	IColorizer* effectiveColorizer;
	IColorizer* basicColorizer;
	ErikssonColorizer eColorizer;
	OptimizedErikssonColorizer oeColorizer;
	StripedColorizer stripedColorizer;
	ColorUp colorUpColorizer;
	ShiftingColorizer shiftColorizer;
};

const std::vector<FractalFramework::method_s> FractalFramework::Methods
=
{
	{
		olc::Key::K1,
		&FractalFramework::CreateFractalOpenMP,
		"OpenMP parallel for"
	},
	{
		olc::Key::K2,
		&FractalFramework::CreateFractalSingleThread,
		"Single Thread Method"
	},
	{
		olc::Key::K3,
		&FractalFramework::CreateFractalCppForEachAlgorithm,
		"C++17 for_each_n parallel implementation"
	},
#if defined(_MSC_VER)
	{
		olc::Key::K4,
		&FractalFramework::CreateFractalParallelization,
#ifdef __clang_version__
		"parallel_for Method (clang "  __clang_version__ ")"
#else
		"parallel_for Method (MSC)"  
#endif

	},
#endif
};

#define keyData(k) olc::Key::k, #k

const std::vector<FractalFramework::key_command_s> FractalFramework::KeyCommands
=
{
	{
		keyData(S),
		"Toggle striped eColorizer",
		&FractalFramework::ToggleStripes
	},
	{
		keyData(L),
		"Toggle loop detection",
		&FractalFramework::ToggleLoopDetection
	},
	{
		keyData(UP),
		"Increase maximum interation",
		&FractalFramework::AdjustIterations
	},
	{
		keyData(DOWN),
		"Increase maximum interation",
		&FractalFramework::AdjustIterations
	},
	{
		keyData(Q),
		"Exit program",
		&FractalFramework::ExitProgram
	},
	{
		keyData(ESCAPE),
		"Exit program",
		&FractalFramework::ExitProgram
	},
	{
		keyData(M),
		"Mandelbrot function",
		&FractalFramework::SelectFunction
	},
	{
		keyData(B),
		"Burning Ship function",
		&FractalFramework::SelectFunction
	},
	{
		keyData(G),
		"Logistic function",
		&FractalFramework::SelectFunction
	},
	{
		keyData(J),
		"Toggle Julia mode",
		&FractalFramework::ToggleJulia
	},
	{
		keyData(C),
		"Cycle colorizers",
		&FractalFramework::CycleColorizer
	},
	{
		keyData(A),
		"Toggle Animate colors",
		&FractalFramework::ToggleAnimateColors
	},
};

int main()
{
	FractalFramework demo;
	if (demo.Construct(1280, 960, 1, 1, false, false))
		demo.Start();
	return 0;
}

