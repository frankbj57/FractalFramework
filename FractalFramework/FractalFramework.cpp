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

#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include <ppl.h>

#include "CircularBufferTemplate.h"
#include "ErikssonColorizer.h"
#include "StripedColorizer.h"

constexpr int nMaxThreads = 32;

struct IComputeState
{
	double cr, ci;
	double zr, zi;
	double zr2, zi2;

	virtual void Advance() = 0;
	virtual IComputeState* Clone() = 0;
};

struct IComputePoint
{
	std::unique_ptr<IComputeState> z;
	int maxIterations;

	virtual int ComputePointCount(double x, double y) = 0;
	virtual IComputePoint* Clone() = 0;
	virtual ~IComputePoint() {}
};

class FractalFramework : public olc::PixelGameEngine
{
public:
	FractalFramework() : stripedColorizer(&colorizer)
	{
		sAppName = "Fractal Framework";
	}

	int* pFractal = nullptr;
	int nMode = 1;
	int nIterations = 256;
	bool striped = false;

public:
	bool OnUserCreate() override
	{
		//pFractal = new int[ScreenWidth() * ScreenHeight()]{ 0 };

		// Using Vector extensions, align memory (not as necessary as it used to be)
		// MS Specific - see std::aligned_alloc for others
		pFractal = (int*)_aligned_malloc(size_t(ScreenWidth()) * size_t(ScreenHeight()) * sizeof(int), 64);

		colorizer.scale = nIterations;

		effectiveColorizer = &colorizer;

		// Original viewport should be x: 0 to 2, y: 0 to 1. Screencoordinates y are opposite
		float xscale = ScreenWidth() / (2.0 + 1.0);
		float yscale = ScreenHeight() / (2.0);
		float scale = std::min(xscale, yscale);

		tv.Initialise(
			{ ScreenWidth(), ScreenHeight() },
			{ scale, -scale});
		tv.SetWorldOffset({-2, 1});

		m_pCurrentStateAlgorithm = new BurningShipComputeState;

		m_ComputePoint.z.reset(m_pCurrentStateAlgorithm->Clone());
		m_ComputePoint.maxIterations = nIterations;

		m_ComputePointWithLoop.z.reset(m_pCurrentStateAlgorithm->Clone());
		m_ComputePointWithLoop.maxIterations = nIterations;

		m_pCurrentPointAlgorithm = &m_ComputePoint;
		loopCheck = false;
		return true;
	}

	bool OnUserDestroy() override
	{
		// Clean up memory
		_aligned_free(pFractal);
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

	struct ComputePoint : public IComputePoint
	{
		inline int ComputePointCount(double x, double y) override
		{
			int n = 0;
			z->ci = y;
			z->cr = x;
			z->zr = 0;
			z->zi = 0;
			z->zr2 = 0;
			z->zi2 = 0;

			while ((z->zr2 + z->zi2) < 4.0 && n < maxIterations)
			{
				z->Advance();
				n++;
			}

			return n;
		}

		inline IComputePoint* Clone() override
		{
			ComputePoint* pR = new ComputePoint;
			pR->z.reset(z->Clone());
			pR->maxIterations = maxIterations;

			return pR;
		}

		~ComputePoint() override
		{
		}
	};

	struct ComputePointWithLoop : public IComputePoint
	{
		inline int ComputePointCount(double x, double y)
		{
			z->ci = y;
			z->cr = x;
			z->zr = 0;
			z->zi = 0;
			z->zr2 = 0;
			z->zi2 = 0;

			std::unique_ptr<IComputeState> ztail(z->Clone());

			int n = 0;
			bool loops = false;
			while ((z->zr2 + z->zi2) < 4.0 && n < maxIterations && !loops)
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
			pR->z.reset(z->Clone());
			pR->maxIterations = maxIterations;

			return pR;
		}

		~ComputePointWithLoop() override
		{
		}
	};

	IComputeState *m_pCurrentStateAlgorithm;
	IComputePoint* m_pCurrentPointAlgorithm;
	ComputePoint m_ComputePoint;
	ComputePointWithLoop m_ComputePointWithLoop;
	bool loopCheck;

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

			for (x = pix_tl.x; x < pix_br.x; x++)
			{
				n = comPoint->ComputePointCount(x_pos, y_pos);

				pFractal[y_offset + x] = n;
				x_pos += x_scale;
			}
		}
	}

	// Using built in parallelisation
	void CreateFractalParallelization(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations)
	{
		const double x_scale = (frac_br.x - frac_tl.x) / (double(pix_br.x) - double(pix_tl.x));
		const double y_scale = (frac_br.y - frac_tl.y) / (double(pix_br.y) - double(pix_tl.y));

		const int row_size = ScreenWidth();

		concurrency::parallel_for(int(pix_tl.y), int(pix_br.y), [&](int y)
			{
				double x_pos = frac_tl.x;
				const double y_pos = frac_tl.y+y*y_scale;

				const int y_offset = y * row_size;

				int x, n;

				// We need a copy for each parallel task, possibly down to each y coordinate
				std::unique_ptr<IComputePoint> comPoint(m_pCurrentPointAlgorithm->Clone());

				for (x = pix_tl.x; x < pix_br.x; x++)
				{
					n = comPoint->ComputePointCount(x_pos, y_pos);

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

		for (y = pix_tl.y; y < pix_br.y; y++)
		{
			double x_pos = frac_tl.x;
			const double y_pos = frac_tl.y + y * y_scale;

			const int y_offset = y * row_size;

			int x, n;

			for (x = pix_tl.x; x < pix_br.x; x++)
			{
				n = comPoint->ComputePointCount(x_pos, y_pos);

				pFractal[y_offset + x] = n;
				x_pos += x_scale;
			}
		}
	}

	CircularBuffer<std::chrono::duration<double>> elapseBuffer = CircularBuffer<std::chrono::duration<double>>(40);

	using CreateFractalFunction = void(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations);

	struct method_s {
		CreateFractalFunction FractalFramework::* pCreateMethod;
		string description;
	};

	static const method_s Methods[];

	vector<olc::vf2d> track;
	olc::vf2d prevMousPos;
	int loopLength = 0;

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Handle transform control
		tv.HandlePanAndZoom();

		olc::vi2d pix_tl = { 0,0 };
		olc::vi2d pix_br = { ScreenWidth(), ScreenHeight() };
		olc::vd2d frac_tl = { -2.0, -1.0 };
		olc::vd2d frac_br = { 1.0, 1.0 };

		frac_tl = tv.ScreenToWorld(pix_tl);
		frac_br = tv.ScreenToWorld(pix_br);

		// Handle User Input
		if (GetKey(olc::K1).bPressed) nMode = 0;
		if (GetKey(olc::K2).bPressed) nMode = 1;
		if (GetKey(olc::K3).bPressed) nMode = 2;

		if (GetKey(olc::UP).bPressed)
		{
			nIterations += 64;
			colorizer.scale = nIterations;
		}
		if (GetKey(olc::DOWN).bPressed)
		{
			nIterations -= 64;
			colorizer.scale = nIterations;
		}

		if (nIterations < 64)
		{
			nIterations = 64;
			colorizer.scale = nIterations;
		}

		if (GetKey(olc::L).bPressed)
		{
			// Toggle loopcheck
			loopCheck = !loopCheck;
			if (loopCheck)
				m_pCurrentPointAlgorithm = &m_ComputePointWithLoop;
			else
				m_pCurrentPointAlgorithm = &m_ComputePoint;
		}

		if (GetKey(olc::S).bPressed)
		{
			// Toggle striped
			striped = !striped;
			if (striped)
				effectiveColorizer = &stripedColorizer;
			else
				effectiveColorizer = &colorizer;
		}

		if (GetKey(olc::M).bPressed || GetKey(olc::B).bPressed)
		{
			if (GetKey(olc::M).bPressed)
				m_pCurrentStateAlgorithm = new MandelComputeState;
			else
				m_pCurrentStateAlgorithm = new BurningShipComputeState;

			m_ComputePoint.z.reset(m_pCurrentStateAlgorithm->Clone());
			m_ComputePoint.maxIterations = nIterations;

			m_ComputePointWithLoop.z.reset(m_pCurrentStateAlgorithm->Clone());
			m_ComputePointWithLoop.maxIterations = nIterations;

			if (loopCheck)
				m_pCurrentPointAlgorithm = &m_ComputePointWithLoop;
			else
				m_pCurrentPointAlgorithm = &m_ComputePoint;

		}

		olc::vf2d pos = GetMousePos();
		if (GetMouse(0).bPressed || (GetMouse(0).bHeld && pos != prevMousPos))
		{
			// Calculate orbit for the selected point at the mouse
			prevMousPos = pos;
			track.clear();
			pos = tv.ScreenToWorld(pos);
			std::unique_ptr<IComputeState> pz(m_pCurrentStateAlgorithm->Clone());
			pz->cr = pos.x;
			pz->ci = pos.y;
			pz->zr = 0;
			pz->zi = 0;
			pz->zr2 = 0;
			pz->zi2 = 0;

			std::unique_ptr<IComputeState> pztail(pz->Clone());
			pz->Advance();
			int i = 1;
			bool loops = false;
			while ((pz->zr2 + pz->zi2) < 4.0 && i < nIterations && !loops)
			{
				track.push_back({ (float)pz->zr, (float)pz->zi });
				pz->Advance();
				loops = pz->zr == pztail->zr && pz->zi == pztail->zi;
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
				while (pz->zr != pztail->zr && pz->zi != pztail->zi)
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

		m_pCurrentPointAlgorithm->z.reset(m_pCurrentStateAlgorithm->Clone());
		m_pCurrentPointAlgorithm->maxIterations = nIterations;

		// START TIMING
		auto tp1 = std::chrono::high_resolution_clock::now();

		// Do the computation
		// Select the right method from the Create Methods table
		(this->*Methods[nMode].pCreateMethod)(pix_tl, pix_br, frac_tl, frac_br, nIterations);

		// STOP TIMING
		auto tp2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsedTime = tp2 - tp1;
		elapseBuffer.insert(elapsedTime);

		// Render result to screen
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
				// Warning - it takes a long time to draw a line which is outside the window!
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
		DrawString(0, 0, std::to_string(nMode+1) + ") " + Methods[nMode].description, olc::WHITE, 3);

		// DrawString(0, 30, "Time Taken: " + std::to_string(elapsedTime.count()) + "s", olc::WHITE, 3);
		DrawString(0, 30, "Time Taken: " + std::to_string(elapseBuffer.meanValue().count()) + "s", olc::WHITE, 3);
		DrawString(0, 60, "Iterations: " + std::to_string(m_pCurrentPointAlgorithm->maxIterations), olc::WHITE, 3);
		DrawString(0, 90, "Track length: " + std::to_string(track.size()), olc::WHITE, 3);
		DrawString(0, 120, "Loop length: " + std::to_string(loopLength), olc::WHITE, 3);
		return !(GetKey(olc::Key::ESCAPE).bPressed);
	}


	olc::TransformedView tv;

	IColorizer* effectiveColorizer;
	ErikssonColorizer colorizer;
	StripedColorizer stripedColorizer;
};

const FractalFramework::method_s FractalFramework::Methods[]
=
{
	{
		&FractalFramework::CreateFractalOpenMP,
		"OpenMP parallel for"
	},
	{
		&FractalFramework::CreateFractalParallelization,
		"parallel_for Method"
	},
	{
		&FractalFramework::CreateFractalSingleThread,
		"Single Thread Method"
	},
};


int main()
{
	FractalFramework demo;
	if (demo.Construct(1024, 1024, 1, 1, false, false))
		demo.Start();
	return 0;
}