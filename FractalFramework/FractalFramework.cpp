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

#include <condition_variable>
#include <atomic>
#include <complex>
#include <cstdlib>
#include <immintrin.h>
#include <ppl.h>

#include "CircularBufferTemplate.h"

constexpr int nMaxThreads = 32;

class FractalFramework : public olc::PixelGameEngine
{
public:
	FractalFramework()
	{
		sAppName = "Fractal Framework";
	}

	int* pFractal = nullptr;
	int nMode = 1;
	int nIterations = 256;

public:
	bool OnUserCreate() override
	{
		//pFractal = new int[ScreenWidth() * ScreenHeight()]{ 0 };

		// Using Vector extensions, align memory (not as necessary as it used to be)
		// MS Specific - see std::aligned_alloc for others
		pFractal = (int*)_aligned_malloc(size_t(ScreenWidth()) * size_t(ScreenHeight()) * sizeof(int), 64);

		return true;
	}

	bool OnUserDestroy() override
	{
		// Clean up memory
		_aligned_free(pFractal);
		return true;
	}

	// New parallel method, using OpenMP
	void CreateFractalOpenMP(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations)
	{
		const double x_scale = (frac_br.x - frac_tl.x) / (double(pix_br.x) - double(pix_tl.x));
		const double y_scale = (frac_br.y - frac_tl.y) / (double(pix_br.y) - double(pix_tl.y));

		const int row_size = ScreenWidth();

		int y;


#pragma omp parallel
		#pragma omp for schedule(dynamic, 1)
		for (y = pix_tl.y; y < pix_br.y; y++)
		{
			double x_pos = frac_tl.x;
			const double y_pos = frac_tl.y + y * y_scale;

			const int y_offset = y * row_size;

			int x, n;

			double cr;
			double ci;
			double zr;
			double zi;
			double re = 0;
			double im = 0;
			double zr2;
			double zi2;

			ci = y_pos;

			for (x = pix_tl.x; x < pix_br.x; x++)
			{
				cr = x_pos;
				zr = 0;
				zi = 0;
				zr2 = 0;
				zi2 = 0;

				n = 0;
				while ((zr2 + zi2) < 4.0 && n < iterations)
				{
					re = zr2 - zi2 + cr;
					im = zr * zi * 2.0 + ci;
					zr = re;
					zi = im;
					zr2 = zr * zr;
					zi2 = zi * zi;
					n++;
				}

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

				double cr = 0;
				double ci = 0;
				double zr = 0;
				double zi = 0;
				double re = 0;
				double im = 0;
				double zr2 = 0;
				double zi2 = 0;

				ci = y_pos;
				for (x = pix_tl.x; x < pix_br.x; x++)
				{
					cr = x_pos;
					zr = 0;
					zi = 0;
					zr2 = 0;
					zi2 = 0;

					n = 0;
					while ((zr2 + zi2) < 4.0 && n < iterations)
					{
						re = zr2 - zi2 + cr;
						im = zr * zi * 2.0 + ci;
						zr = re;
						zi = im;
						zr2 = zr * zr;
						zi2 = zi * zi;
						n++;
					}

					pFractal[y_offset + x] = n;
					x_pos += x_scale;
				}
			});
	}

	CircularBuffer<std::chrono::duration<double>> elapseBuffer = CircularBuffer<std::chrono::duration<double>>(40);

	using CreateFractalFunction = void(const olc::vi2d& pix_tl, const olc::vi2d& pix_br, const olc::vd2d& frac_tl, const olc::vd2d& frac_br, const int iterations);

	struct method_s {
		CreateFractalFunction FractalFramework::* pCreateMethod;
		string description;
	};

	static const method_s Methods[];

	bool OnUserUpdate(float fElapsedTime) override
	{

		// Get mouse location this frame
		olc::vd2d vMouse = { (double)GetMouseX(), (double)GetMouseY() };

		// Handle Pan & Zoom
		if (GetMouse(2).bPressed)
		{
			vStartPan = vMouse;
		}

		if (GetMouse(2).bHeld)
		{
			vOffset -= (vMouse - vStartPan) / vScale;
			vStartPan = vMouse;
		}

		olc::vd2d vMouseBeforeZoom;
		ScreenToWorld(vMouse, vMouseBeforeZoom);

		if (GetKey(olc::Key::Q).bHeld || GetMouseWheel() > 0) vScale *= 1.1;
		if (GetKey(olc::Key::A).bHeld || GetMouseWheel() < 0) vScale *= 0.9;
		
		olc::vd2d vMouseAfterZoom;
		ScreenToWorld(vMouse, vMouseAfterZoom);
		vOffset += (vMouseBeforeZoom - vMouseAfterZoom);
		
		olc::vi2d pix_tl = { 0,0 };
		olc::vi2d pix_br = { ScreenWidth(), ScreenHeight() };
		olc::vd2d frac_tl = { -2.0, -1.0 };
		olc::vd2d frac_br = { 1.0, 1.0 };

		ScreenToWorld(pix_tl, frac_tl);
		ScreenToWorld(pix_br, frac_br);

		// Handle User Input
		if (GetKey(olc::K1).bPressed) nMode = 0;
		if (GetKey(olc::K2).bPressed) nMode = 1;

		if (GetKey(olc::UP).bPressed) nIterations += 64;
		if (GetKey(olc::DOWN).bPressed) nIterations -= 64;
		if (nIterations < 64) nIterations = 64;


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
					float n = (float)i;
					float a = 0.1f;
					// Thank you @Eriksonn - Wonderful Magic Fractal Oddball Man
					Draw(x, y, olc::PixelF(0.5f * sin(a * n) + 0.5f, 0.5f * sin(a * n + 2.094f) + 0.5f, 0.5f * sin(a * n + 4.188f) + 0.5f));
				}
			}
		}

		// Render UI
		DrawString(0, 0, std::to_string(nMode+1) + ") " + Methods[nMode].description, olc::WHITE, 3);

		// DrawString(0, 30, "Time Taken: " + std::to_string(elapsedTime.count()) + "s", olc::WHITE, 3);
		DrawString(0, 30, "Time Taken: " + std::to_string(elapseBuffer.meanValue().count()) + "s", olc::WHITE, 3);
		DrawString(0, 60, "Iterations: " + std::to_string(nIterations), olc::WHITE, 3);
		return !(GetKey(olc::Key::ESCAPE).bPressed);
	}

	// Pan & Zoom variables
	olc::vd2d vOffset = { 0.0, 0.0 };
	olc::vd2d vStartPan = { 0.0, 0.0 };
	olc::vd2d vScale = { 1280.0 / 2.0, 720.0 };
	

	// Convert coordinates from World Space --> Screen Space
	void WorldToScreen(const olc::vd2d& v, olc::vi2d &n)
	{
		n.x = (int)((v.x - vOffset.x) * vScale.x);
		n.y = (int)((v.y - vOffset.y) * vScale.y);
	}

	// Convert coordinates from Screen Space --> World Space
	void ScreenToWorld(const olc::vi2d& n, olc::vd2d& v)
	{
		v.x = (double)(n.x) / vScale.x + vOffset.x;
		v.y = (double)(n.y) / vScale.y + vOffset.y;
	}
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
};


int main()
{
	FractalFramework demo;
	if (demo.Construct(1280, 720, 1, 1, false, false))
		demo.Start();
	return 0;
}