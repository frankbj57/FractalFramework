/*
	olcPGEX_TransformedView.h

	+-------------------------------------------------------------+
	|         OneLoneCoder Pixel Game Engine Extension            |
	|                 Transformed View v1.10                      |
	+-------------------------------------------------------------+

	NOTE: UNDER ACTIVE DEVELOPMENT - THERE ARE BUGS/GLITCHES

	What is this?
	~~~~~~~~~~~~~
	This extension provides drawing routines that are compatible with
	changeable world and screen spaces. For example you can pan and
	zoom, and all PGE drawing routines will automatically adopt the current
	world scales and offsets.

	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018 - 2025 OneLoneCoder.com

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

	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Homepage:	https://www.onelonecoder.com

	Author
	~~~~~~
	David Barr, aka javidx9, ©OneLoneCoder 2019, 2020, 2021, 2022, 2023, 2024, 2025

	Revisions:
	1.00:	Initial Release
	1.01:	Fix for rounding error when scaling to screen
	1.02:	Added DrawLineDecal for convenience
	1.03:	Removed std::floor from WorldToScreen()
			Added HandlePanAndZoom(...) convenience function
			Removed unused "range" facility in TileTransformView
	1.04:	Added DrawPolygonDecal() for arbitrary polygons
	1.05:	Clipped DrawSprite() to visible area, massive performance increase
	1.06:	Fixed error in DrawLine() - Thanks CraisyDaisyRecords (& Fern)!
	1.07:   +DrawRectDecal()
			+GetPGE()
	1.08:  +DrawPolygonDecal() with tint overload, akin to PGE
	1.09:  +SetScaleExtents() - Sets range that world scale can exist within
		   +EnableScaleClamp() - Applies a range that scaling is clamped to
				These are both useful for having zoom clamped between a min and max
				without weird panning artefacts occuring
	1.10:	Hitched in some "shader" PGEX things
*/

#pragma once
#ifndef OLC_PGEX_TRANSFORMEDVIEW_TEMPLATE_H
#define OLC_PGEX_TRANSFORMEDVIEW_TEMPLATE_H

#include "olcPixelGameEngine.h"



namespace olc
{
	template <class BASE>
	class TransformedViewTemplate : public olc::PGEX
	{
	public:
		TransformedViewTemplate() = default;
		virtual void Initialise(const olc::vi2d& vViewArea, const olc::v_2d<BASE>& vPixelScale = { 1.0f, 1.0f });

		olc::PixelGameEngine* GetPGE();

	public:
		void SetWorldOffset(const olc::v_2d<BASE>& vOffset);
		void MoveWorldOffset(const olc::v_2d<BASE>& vDeltaOffset);
		void SetWorldScale(const olc::v_2d<BASE>& vScale);
		void SetViewArea(const olc::vi2d& vViewArea);
		olc::v_2d<BASE> GetWorldTL() const;
		olc::v_2d<BASE> GetWorldBR() const;
		olc::v_2d<BASE> GetWorldVisibleArea() const;
		void ZoomAtScreenPos(const BASE fDeltaZoom, const olc::vi2d& vPos);
		void SetZoom(const BASE fZoom, const olc::v_2d<BASE>& vPos);
		void StartPan(const olc::vi2d& vPos);
		void UpdatePan(const olc::vi2d& vPos);
		void EndPan(const olc::vi2d& vPos);
		const olc::v_2d<BASE>& GetWorldOffset() const;
		const olc::v_2d<BASE>& GetWorldScale() const;
		virtual olc::v_2d<BASE> WorldToScreen(const olc::v_2d<BASE>& vWorldPos) const;
		virtual olc::v_2d<BASE> ScreenToWorld(const olc::v_2d<BASE>& vScreenPos) const;
		virtual olc::v_2d<BASE> ScaleToWorld(const olc::v_2d<BASE>& vScreenSize) const;
		virtual olc::v_2d<BASE> ScaleToScreen(const olc::v_2d<BASE>& vWorldSize) const;
		virtual bool IsPointVisible(const olc::v_2d<BASE>& vPos) const;
		virtual bool IsRectVisible(const olc::v_2d<BASE>& vPos, const olc::v_2d<BASE>& vSize) const;
		virtual void HandlePanAndZoom(const int nMouseButton = 2, const BASE fZoomRate = 0.1f, const bool bPan = true, const bool bZoom = true);
		void SetScaleExtents(const olc::v_2d<BASE>& vScaleMin, const olc::v_2d<BASE>& vScaleMax);
		void EnableScaleClamp(const bool bEnable);

	protected:
		olc::v_2d<BASE> m_vWorldOffset = { 0.0f, 0.0f };
		olc::v_2d<BASE> m_vWorldScale = { 1.0f, 1.0f };
		olc::v_2d<BASE> m_vRecipPixel = { 1.0f, 1.0f };
		olc::v_2d<BASE> m_vPixelScale = { 1.0f, 1.0f };
		bool m_bPanning = false;
		olc::v_2d<BASE> m_vStartPan = { 0.0f, 0.0f };
		olc::vi2d m_vViewArea;
		bool m_bZoomClamp = false;
		olc::v_2d<BASE> m_vMaxScale = { 0.0f, 0.0f };
		olc::v_2d<BASE> m_vMinScale = { 0.0f, 0.0f };

	public: // Hopefully, these should look familiar!
		// Plots a single point
		virtual bool Draw(BASE x, BASE y, olc::Pixel p = olc::WHITE);
		bool Draw(const olc::v_2d<BASE>& pos, olc::Pixel p = olc::WHITE);
		// Draws a line from (x1,y1) to (x2,y2)
		void DrawLine(BASE x1, BASE y1, BASE x2, BASE y2, olc::Pixel p = olc::WHITE, uint32_t pattern = 0xFFFFFFFF);
		void DrawLine(const olc::v_2d<BASE>& pos1, const olc::v_2d<BASE>& pos2, olc::Pixel p = olc::WHITE, uint32_t pattern = 0xFFFFFFFF);
		// Draws a circle located at (x,y) with radius
		void DrawCircle(BASE x, BASE y, BASE radius, olc::Pixel p = olc::WHITE, uint8_t mask = 0xFF);
		void DrawCircle(const olc::v_2d<BASE>& pos, BASE radius, olc::Pixel p = olc::WHITE, uint8_t mask = 0xFF);
		// Fills a circle located at (x,y) with radius
		void FillCircle(BASE x, BASE y, BASE radius, olc::Pixel p = olc::WHITE);
		void FillCircle(const olc::v_2d<BASE>& pos, BASE radius, olc::Pixel p = olc::WHITE);
		// Draws a rectangle at (x,y) to (x+w,y+h)
		void DrawRect(BASE x, BASE y, BASE w, BASE h, olc::Pixel p = olc::WHITE);
		void DrawRect(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, olc::Pixel p = olc::WHITE);
		// Fills a rectangle at (x,y) to (x+w,y+h)
		void FillRect(BASE x, BASE y, BASE w, BASE h, olc::Pixel p = olc::WHITE);
		void FillRect(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, olc::Pixel p = olc::WHITE);
		// Draws a triangle between points (x1,y1), (x2,y2) and (x3,y3)
		void DrawTriangle(BASE x1, BASE y1, BASE x2, BASE y2, BASE x3, BASE y3, olc::Pixel p = olc::WHITE);
		void DrawTriangle(const olc::v_2d<BASE>& pos1, const olc::v_2d<BASE>& pos2, const olc::v_2d<BASE>& pos3, olc::Pixel p = olc::WHITE);
		// Flat fills a triangle between points (x1,y1), (x2,y2) and (x3,y3)
		void FillTriangle(BASE x1, BASE y1, BASE x2, BASE y2, BASE x3, BASE y3, olc::Pixel p = olc::WHITE);
		void FillTriangle(const olc::v_2d<BASE>& pos1, const olc::v_2d<BASE>& pos2, const olc::v_2d<BASE>& pos3, olc::Pixel p = olc::WHITE);
		// Draws an entire sprite at location (x,y)
		void DrawSprite(BASE x, BASE y, olc::Sprite* sprite, BASE scalex = 1, BASE scaley = 1, uint8_t flip = olc::Sprite::NONE);
		void DrawSprite(const olc::v_2d<BASE>& pos, olc::Sprite* sprite, const olc::v_2d<BASE>& scale = { 1.0f, 1.0f }, uint8_t flip = olc::Sprite::NONE);
		// Draws an area of a sprite at location (x,y), where the
		// selected area is (ox,oy) to (ox+w,oy+h)
		void DrawPartialSprite(BASE x, BASE y, Sprite* sprite, int32_t ox, int32_t oy, int32_t w, int32_t h, BASE scalex = 1, BASE scaley = 1, uint8_t flip = olc::Sprite::NONE);
		void DrawPartialSprite(const olc::v_2d<BASE>& pos, Sprite* sprite, const olc::vi2d& sourcepos, const olc::vi2d& size, const olc::v_2d<BASE>& scale = { 1.0f, 1.0f }, uint8_t flip = olc::Sprite::NONE);
		void DrawString(BASE x, BASE y, const std::string& sText, Pixel col, const olc::v_2d<BASE>& scale);
		void DrawString(const olc::v_2d<BASE>& pos, const std::string& sText, const Pixel col, const olc::v_2d<BASE>& scale);


		// Draws a whole decal, with optional scale and tinting
		void DrawDecal(const olc::v_2d<BASE>& pos, olc::Decal* decal, const olc::v_2d<BASE>& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
		// Draws a region of a decal, with optional scale and tinting
		void DrawPartialDecal(const olc::v_2d<BASE>& pos, olc::Decal* decal, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::v_2d<BASE>& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
		void DrawPartialDecal(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, olc::Decal* decal, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint = olc::WHITE);
		// Draws fully user controlled 4 vertices, pos(pixels), uv(pixels), colours
		void DrawExplicitDecal(olc::Decal* decal, const olc::v_2d<BASE>* pos, const olc::vf2d* uv, const olc::Pixel* col, uint32_t elements = 4);
		//// Draws a decal with 4 arbitrary points, warping the texture to look "correct"
		void DrawWarpedDecal(olc::Decal* decal, const olc::v_2d<BASE>(&pos)[4], const olc::Pixel& tint = olc::WHITE);
		void DrawWarpedDecal(olc::Decal* decal, const olc::v_2d<BASE>* pos, const olc::Pixel& tint = olc::WHITE);
		void DrawWarpedDecal(olc::Decal* decal, const std::array<olc::v_2d<BASE>, 4>& pos, const olc::Pixel& tint = olc::WHITE);
		//// As above, but you can specify a region of a decal source sprite
		void DrawPartialWarpedDecal(olc::Decal* decal, const olc::v_2d<BASE>(&pos)[4], const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint = olc::WHITE);
		void DrawPartialWarpedDecal(olc::Decal* decal, const olc::v_2d<BASE>* pos, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint = olc::WHITE);
		void DrawPartialWarpedDecal(olc::Decal* decal, const std::array<olc::v_2d<BASE>, 4>& pos, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint = olc::WHITE);
		//// Draws a decal rotated to specified angle, wit point of rotation offset
		void DrawRotatedDecal(const olc::v_2d<BASE>& pos, olc::Decal* decal, const float fAngle, const olc::v_2d<BASE>& center = { 0.0f, 0.0f }, const olc::v_2d<BASE>& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
		void DrawPartialRotatedDecal(const olc::v_2d<BASE>& pos, olc::Decal* decal, const float fAngle, const olc::v_2d<BASE>& center, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::v_2d<BASE>& scale = { 1.0f, 1.0f }, const olc::Pixel& tint = olc::WHITE);
		// Draws a multiline string as a decal, with tiniting and scaling
		void DrawStringDecal(const olc::v_2d<BASE>& pos, const std::string& sText, const olc::Pixel col = olc::WHITE, const olc::v_2d<BASE>& scale = { 1.0f, 1.0f });
		void DrawStringPropDecal(const olc::v_2d<BASE>& pos, const std::string& sText, const olc::Pixel col = olc::WHITE, const olc::v_2d<BASE>& scale = { 1.0f, 1.0f });
		// Draws a single shaded filled rectangle as a decal
		void FillRectDecal(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, const olc::Pixel col = olc::WHITE);
		void DrawRectDecal(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, const olc::Pixel col = olc::WHITE);

		// Draws a corner shaded rectangle as a decal
		void GradientFillRectDecal(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, const olc::Pixel colTL, const olc::Pixel colBL, const olc::Pixel colBR, const olc::Pixel colTR);
		// Draws an arbitrary convex textured polygon using GPU
		void DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::v_2d<BASE>>& pos, const std::vector<olc::vf2d>& uv, const olc::Pixel tint = olc::WHITE);
		void DrawLineDecal(const olc::v_2d<BASE>& pos1, const olc::v_2d<BASE>& pos2, Pixel p = olc::WHITE);
		void DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::v_2d<BASE>>& pos, const std::vector<olc::vf2d>& uv, const std::vector<olc::Pixel>& tint);
		void DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::v_2d<BASE>>& pos, const std::vector<olc::vf2d>& uv, const std::vector<olc::Pixel>& colours, const olc::Pixel tint);


#if defined(OLC_USING_PGEX_SHADER)
		// Shader Specific
		void DrawDecal(olc::Shade& shader, const olc::v_2d<BASE>& pos, olc::Decal* decal, const olc::v_2d<BASE>& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
		void DrawPartialDecal(olc::Shade& shader, const olc::v_2d<BASE>& pos, olc::Decal* decal, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::v_2d<BASE>& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
		void DrawPartialDecal(olc::Shade& shader, const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, olc::Decal* decal, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint = olc::WHITE);
#endif



	};

	using TransformedViewD = TransformedViewTemplate<double>;
	using TransformedView = TransformedViewTemplate<float>;

}

#ifdef OLC_PGEX_TRANSFORMEDVIEW_IMPLEMENTATION
#undef OLC_PGEX_TRANSFORMEDVIEW_IMPLEMENTATION


namespace olc
{
	template <class BASE>
	olc::PixelGameEngine* TransformedViewTemplate<BASE>::GetPGE()
	{
		return pge;
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::Initialise(const olc::vi2d& vViewArea, const olc::v_2d<BASE>& vPixelScale)
	{
		SetViewArea(vViewArea);
		SetWorldScale(vPixelScale);
		m_vPixelScale = vPixelScale;
		m_vRecipPixel = 1.0f / m_vPixelScale;
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::SetWorldOffset(const olc::v_2d<BASE>& vOffset)
	{
		m_vWorldOffset = vOffset;
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::MoveWorldOffset(const olc::v_2d<BASE>& vDeltaOffset)
	{
		m_vWorldOffset += vDeltaOffset;
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::SetWorldScale(const olc::v_2d<BASE>& vScale)
	{
		m_vWorldScale = vScale;
		if (m_bZoomClamp) m_vWorldScale = m_vWorldScale.clamp(m_vMinScale, m_vMaxScale);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::SetViewArea(const olc::vi2d& vViewArea)
	{
		m_vViewArea = vViewArea;
	}

	template <class BASE>
	olc::v_2d<BASE> TransformedViewTemplate<BASE>::GetWorldTL() const
	{
		return TransformedViewTemplate<BASE>::ScreenToWorld({ 0,0 });
	}

	template <class BASE>
	olc::v_2d<BASE> TransformedViewTemplate<BASE>::GetWorldBR() const
	{
		return TransformedViewTemplate<BASE>::ScreenToWorld(m_vViewArea);
	}

	template <class BASE>
	olc::v_2d<BASE> TransformedViewTemplate<BASE>::GetWorldVisibleArea() const
	{
		return GetWorldBR() - GetWorldTL();
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::SetScaleExtents(const olc::v_2d<BASE>& vScaleMin, const olc::v_2d<BASE>& vScaleMax)
	{
		m_vMaxScale = vScaleMax;
		m_vMinScale = vScaleMin;
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::EnableScaleClamp(const bool bEnable)
	{
		m_bZoomClamp = bEnable;
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::ZoomAtScreenPos(const BASE fDeltaZoom, const olc::vi2d& vPos)
	{
		olc::v_2d<BASE> vOffsetBeforeZoom = ScreenToWorld(vPos);
		m_vWorldScale *= fDeltaZoom;
		if (m_bZoomClamp) m_vWorldScale = m_vWorldScale.clamp(m_vMinScale, m_vMaxScale);
		olc::v_2d<BASE> vOffsetAfterZoom = ScreenToWorld(vPos);
		m_vWorldOffset += vOffsetBeforeZoom - vOffsetAfterZoom;
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::SetZoom(const BASE fZoom, const olc::v_2d<BASE>& vPos)
	{
		olc::v_2d<BASE> vOffsetBeforeZoom = ScreenToWorld(vPos);
		m_vWorldScale = { fZoom, fZoom };
		if (m_bZoomClamp) m_vWorldScale = m_vWorldScale.clamp(m_vMinScale, m_vMaxScale);
		olc::v_2d<BASE> vOffsetAfterZoom = ScreenToWorld(vPos);
		m_vWorldOffset += vOffsetBeforeZoom - vOffsetAfterZoom;
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::StartPan(const olc::vi2d& vPos)
	{
		m_bPanning = true;
		m_vStartPan = olc::v_2d<BASE>(vPos);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::UpdatePan(const olc::vi2d& vPos)
	{
		if (m_bPanning)
		{
			m_vWorldOffset -= (olc::v_2d<BASE>(vPos) - m_vStartPan) / m_vWorldScale;
			m_vStartPan = olc::v_2d<BASE>(vPos);
		}
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::EndPan(const olc::vi2d& vPos)
	{
		UpdatePan(vPos);
		m_bPanning = false;
	}

	template <class BASE>
	const olc::v_2d<BASE>& TransformedViewTemplate<BASE>::GetWorldOffset() const
	{
		return m_vWorldOffset;
	}

	template <class BASE>
	const olc::v_2d<BASE>& TransformedViewTemplate<BASE>::GetWorldScale() const
	{
		return m_vWorldScale;
	}

	template <class BASE>
	olc::v_2d<BASE> TransformedViewTemplate<BASE>::WorldToScreen(const olc::v_2d<BASE>& vWorldPos) const
	{
		olc::v_2d<BASE> vFloat = ((vWorldPos - m_vWorldOffset) * m_vWorldScale);
		//vFloat = { std::floor(vFloat.x + 0.5f), std::floor(vFloat.y + 0.5f) };
		return vFloat;
	}

	template <class BASE>
	olc::v_2d<BASE> TransformedViewTemplate<BASE>::ScreenToWorld(const olc::v_2d<BASE>& vScreenPos) const
	{
		return (olc::v_2d<BASE>(vScreenPos) / m_vWorldScale) + m_vWorldOffset;
	}

	template <class BASE>
	olc::v_2d<BASE> TransformedViewTemplate<BASE>::ScaleToWorld(const olc::v_2d<BASE>& vScreenSize) const
	{
		return (olc::v_2d<BASE>(vScreenSize) / m_vWorldScale);
	}

	template <class BASE>
	olc::v_2d<BASE> TransformedViewTemplate<BASE>::ScaleToScreen(const olc::v_2d<BASE>& vWorldSize) const
	{
		//olc::v_2d<BASE> vFloat = (vWorldSize * m_vWorldScale) + olc::v_2d<BASE>(0.5f, 0.5f);		
		//return vFloat.floor();
		return (vWorldSize * m_vWorldScale);
	}

	template <class BASE>
	bool TransformedViewTemplate<BASE>::IsPointVisible(const olc::v_2d<BASE>& vPos) const
	{
		olc::vi2d vScreen = WorldToScreen(vPos);
		return vScreen.x >= 0 && vScreen.x < m_vViewArea.x && vScreen.y >= 0 && vScreen.y < m_vViewArea.y;
	}

	template <class BASE>
	bool TransformedViewTemplate<BASE>::IsRectVisible(const olc::v_2d<BASE>& vPos, const olc::v_2d<BASE>& vSize) const
	{
		olc::vi2d vScreenPos = WorldToScreen(vPos);
		olc::vi2d vScreenSize = vSize * m_vWorldScale;
		return (vScreenPos.x < 0 + m_vViewArea.x && vScreenPos.x + vScreenSize.x > 0 && vScreenPos.y < m_vViewArea.y && vScreenPos.y + vScreenSize.y > 0);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::HandlePanAndZoom(const int nMouseButton, const BASE fZoomRate, const bool bPan, const bool bZoom)
	{
		const auto& vMousePos = pge->GetMousePos();
		if (bPan)
		{
			if (pge->GetMouse(nMouseButton).bPressed) StartPan(vMousePos);
			if (pge->GetMouse(nMouseButton).bHeld) UpdatePan(vMousePos);
			if (pge->GetMouse(nMouseButton).bReleased) EndPan(vMousePos);
		}

		if (bZoom)
		{
			if (pge->GetMouseWheel() > 0) ZoomAtScreenPos(1.0f + fZoomRate, vMousePos);
			if (pge->GetMouseWheel() < 0) ZoomAtScreenPos(1.0f - fZoomRate, vMousePos);
		}
	}

	template <class BASE>
	bool TransformedViewTemplate<BASE>::Draw(BASE x, BASE y, olc::Pixel p)
	{
		return Draw({ x, y }, p);
	}

	template <class BASE>
	bool TransformedViewTemplate<BASE>::Draw(const olc::v_2d<BASE>& pos, olc::Pixel p)
	{
		return pge->Draw(WorldToScreen(pos), p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawLine(BASE x1, BASE y1, BASE x2, BASE y2, olc::Pixel p, uint32_t pattern)
	{
		DrawLine({ x1, y1 }, { x2, y2 }, p, pattern);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawLine(const olc::v_2d<BASE>& pos1, const olc::v_2d<BASE>& pos2, olc::Pixel p, uint32_t pattern)
	{
		pge->DrawLine(WorldToScreen(pos1), WorldToScreen(pos2), p, pattern);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawCircle(BASE x, BASE y, BASE radius, olc::Pixel p, uint8_t mask)
	{
		DrawCircle({ x,y }, radius, p, mask);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawCircle(const olc::v_2d<BASE>& pos, BASE radius, olc::Pixel p, uint8_t mask)
	{
		pge->DrawCircle(WorldToScreen(pos), int32_t(radius * m_vWorldScale.x), p, mask);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::FillCircle(BASE x, BASE y, BASE radius, olc::Pixel p)
	{
		FillCircle({ x,y }, radius, p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::FillCircle(const olc::v_2d<BASE>& pos, BASE radius, olc::Pixel p)
	{
		pge->FillCircle(WorldToScreen(pos), int32_t(radius * m_vWorldScale.x), p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawRect(BASE x, BASE y, BASE w, BASE h, olc::Pixel p)
	{
		DrawRect({ x, y }, { w, h }, p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawRect(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, olc::Pixel p)
	{
		pge->DrawRect(WorldToScreen(pos), ((size * m_vWorldScale) + olc::v_2d<BASE>(0.5f, 0.5f)).floor(), p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::FillRect(BASE x, BASE y, BASE w, BASE h, olc::Pixel p)
	{
		FillRect({ x, y }, { w, h }, p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::FillRect(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, olc::Pixel p)
	{
		pge->FillRect(WorldToScreen(pos), size * m_vWorldScale, p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawTriangle(BASE x1, BASE y1, BASE x2, BASE y2, BASE x3, BASE y3, olc::Pixel p)
	{
		DrawTriangle({ x1, y1 }, { x2, y2 }, { x3, y3 }, p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawTriangle(const olc::v_2d<BASE>& pos1, const olc::v_2d<BASE>& pos2, const olc::v_2d<BASE>& pos3, olc::Pixel p)
	{
		pge->DrawTriangle(WorldToScreen(pos1), WorldToScreen(pos2), WorldToScreen(pos3), p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::FillTriangle(BASE x1, BASE y1, BASE x2, BASE y2, BASE x3, BASE y3, olc::Pixel p)
	{
		FillTriangle({ x1, y1 }, { x2, y2 }, { x3, y3 }, p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::FillTriangle(const olc::v_2d<BASE>& pos1, const olc::v_2d<BASE>& pos2, const olc::v_2d<BASE>& pos3, olc::Pixel p)
	{
		pge->FillTriangle(WorldToScreen(pos1), WorldToScreen(pos2), WorldToScreen(pos3), p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawSprite(BASE x, BASE y, olc::Sprite* sprite, BASE scalex, BASE scaley, uint8_t flip)
	{
		DrawSprite({ x, y }, sprite, { scalex, scaley }, flip);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawSprite(const olc::v_2d<BASE>& pos, olc::Sprite* sprite, const olc::v_2d<BASE>& scale, uint8_t flip)
	{
		olc::v_2d<BASE> vSpriteSize = olc::v_2d<BASE>(BASE(sprite->width), BASE(sprite->height));
		if (IsRectVisible(pos, vSpriteSize * scale))
		{
			olc::v_2d<BASE> vSpriteScaledSize = vSpriteSize * m_vRecipPixel * m_vWorldScale * scale;
			olc::vi2d vPixel;
			olc::vi2d vSpritePixelStart = WorldToScreen(pos);
			olc::vi2d vSpritePixelEnd = WorldToScreen((vSpriteSize * scale) + pos);

			olc::vi2d vScreenPixelStart = (vSpritePixelStart).max({ 0,0 });
			olc::vi2d vScreenPixelEnd = (vSpritePixelEnd).min({ pge->ScreenWidth(),pge->ScreenHeight() });

			olc::v_2d<BASE> vPixelStep = 1.0f / vSpriteScaledSize;

			for (vPixel.y = vScreenPixelStart.y; vPixel.y < vScreenPixelEnd.y; vPixel.y++)
			{
				for (vPixel.x = vScreenPixelStart.x; vPixel.x < vScreenPixelEnd.x; vPixel.x++)
				{
					olc::v_2d<BASE> vSample = olc::v_2d<BASE>(vPixel - vSpritePixelStart) * vPixelStep;
					pge->Draw(vPixel, sprite->Sample(vSample.x, vSample.y));
				}
			}
		}
	}


	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialSprite(BASE x, BASE y, Sprite* sprite, int32_t ox, int32_t oy, int32_t w, int32_t h, BASE scalex, BASE scaley, uint8_t flip)
	{
		DrawPartialSprite({ x,y }, sprite, { ox,oy }, { w, h }, { scalex, scaley }, flip);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialSprite(const olc::v_2d<BASE>& pos, Sprite* sprite, const olc::vi2d& sourcepos, const olc::vi2d& size, const olc::v_2d<BASE>& scale, uint8_t flip)
	{
		olc::v_2d<BASE> vSpriteSize = size;
		if (IsRectVisible(pos, size * scale))
		{
			olc::v_2d<BASE> vSpriteScaledSize = olc::v_2d<BASE>(size) * m_vRecipPixel * m_vWorldScale * scale;
			olc::v_2d<BASE> vSpritePixelStep = 1.0f / olc::v_2d<BASE>(BASE(sprite->width), BASE(sprite->height));
			olc::vi2d vPixel, vStart = WorldToScreen(pos), vEnd = vSpriteScaledSize + vStart;
			olc::v_2d<BASE> vScreenPixelStep = 1.0f / vSpriteScaledSize;

			for (vPixel.y = vStart.y; vPixel.y < vEnd.y; vPixel.y++)
			{
				for (vPixel.x = vStart.x; vPixel.x < vEnd.x; vPixel.x++)
				{
					olc::v_2d<BASE> vSample = ((olc::v_2d<BASE>(vPixel - vStart) * vScreenPixelStep) * size * vSpritePixelStep) + olc::v_2d<BASE>(sourcepos) * vSpritePixelStep;
					pge->Draw(vPixel, sprite->Sample(vSample.x, vSample.y));
				}
			}
		}
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawString(BASE x, BASE y, const std::string& sText, Pixel col, const olc::v_2d<BASE>& scale)
	{
		DrawString({ x, y }, sText, col, scale);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawString(const olc::v_2d<BASE>& pos, const std::string& sText, const Pixel col, const olc::v_2d<BASE>& scale)
	{
		olc::vf2d vOffset = { 0.0f, 0.0f };
		Pixel::Mode m = pge->GetPixelMode();

		auto StringPlot = [&col](const int x, const int y, const olc::Pixel& pSource, const olc::Pixel& pDest)
			{
				return pSource.r > 1 ? col : pDest;
			};

		pge->SetPixelMode(StringPlot);

		for (auto c : sText)
		{
			if (c == '\n')
			{
				vOffset.x = 0.0f; vOffset.y += 8.0f * m_vRecipPixel.y * scale.y;
			}
			else
			{
				int32_t ox = ((c - 32) % 16) * 8;
				int32_t oy = ((c - 32) / 16) * 8;
				DrawPartialSprite(pos + vOffset, pge->GetFontSprite(), { ox, oy }, { 8, 8 }, scale);
				vOffset.x += 8.0f * m_vRecipPixel.x * scale.x;
			}
		}
		pge->SetPixelMode(m);
	}


	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawDecal(const olc::v_2d<BASE>& pos, olc::Decal* decal, const olc::v_2d<BASE>& scale, const olc::Pixel& tint)
	{
		pge->DrawDecal(WorldToScreen(pos), decal, scale * m_vWorldScale * m_vRecipPixel, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialDecal(const olc::v_2d<BASE>& pos, olc::Decal* decal, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::v_2d<BASE>& scale, const olc::Pixel& tint)
	{
		pge->DrawPartialDecal(WorldToScreen(pos), decal, source_pos, source_size, scale * m_vWorldScale * m_vRecipPixel, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialDecal(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, olc::Decal* decal, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint)
	{
		pge->DrawPartialDecal(WorldToScreen(pos), size * m_vWorldScale * m_vRecipPixel, decal, source_pos, source_size, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawExplicitDecal(olc::Decal* decal, const olc::v_2d<BASE>* pos, const olc::vf2d* uv, const olc::Pixel* col, uint32_t elements)
	{
		std::vector<olc::vf2d> vTransformed(elements);
		for (uint32_t n = 0; n < elements; n++)
			vTransformed[n] = WorldToScreen(pos[n]);
		pge->DrawExplicitDecal(decal, vTransformed.data(), uv, col, elements);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawWarpedDecal(olc::Decal* decal, const olc::v_2d<BASE>* pos, const olc::Pixel& tint)
	{
		std::array<olc::vf2d, 4> vTransformed =
		{ {
			WorldToScreen(pos[0]), WorldToScreen(pos[1]),
			WorldToScreen(pos[2]), WorldToScreen(pos[3]),
		} };

		pge->DrawWarpedDecal(decal, vTransformed, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawWarpedDecal(olc::Decal* decal, const olc::v_2d<BASE>(&pos)[4], const olc::Pixel& tint)
	{
		DrawWarpedDecal(decal, &pos[0], tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawWarpedDecal(olc::Decal* decal, const std::array<olc::v_2d<BASE>, 4>& pos, const olc::Pixel& tint)
	{
		DrawWarpedDecal(decal, pos.data(), tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialWarpedDecal(olc::Decal* decal, const olc::v_2d<BASE>(&pos)[4], const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint)
	{
		DrawPartialWarpedDecal(decal, &pos[0], source_pos, source_size, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialWarpedDecal(olc::Decal* decal, const olc::v_2d<BASE>* pos, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint)
	{
		std::array<olc::vf2d, 4> vTransformed =
		{ {
			WorldToScreen(pos[0]), WorldToScreen(pos[1]),
			WorldToScreen(pos[2]), WorldToScreen(pos[3]),
		} };

		pge->DrawPartialWarpedDecal(decal, vTransformed, source_pos, source_size, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialWarpedDecal(olc::Decal* decal, const std::array<olc::v_2d<BASE>, 4>& pos, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint)
	{
		DrawPartialWarpedDecal(decal, pos.data(), source_pos, source_size, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawRotatedDecal(const olc::v_2d<BASE>& pos, olc::Decal* decal, const float fAngle, const olc::v_2d<BASE>& center, const olc::v_2d<BASE>& scale, const olc::Pixel& tint)
	{
		pge->DrawRotatedDecal(WorldToScreen(pos), decal, fAngle, center, scale * m_vWorldScale * m_vRecipPixel, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialRotatedDecal(const olc::v_2d<BASE>& pos, olc::Decal* decal, const float fAngle, const olc::v_2d<BASE>& center, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::v_2d<BASE>& scale, const olc::Pixel& tint)
	{
		pge->DrawPartialRotatedDecal(WorldToScreen(pos), decal, fAngle, center, source_pos, source_size, scale * m_vWorldScale * m_vRecipPixel, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawStringDecal(const olc::v_2d<BASE>& pos, const std::string& sText, const olc::Pixel col, const olc::v_2d<BASE>& scale)
	{
		pge->DrawStringDecal(WorldToScreen(pos), sText, col, scale * m_vWorldScale * m_vRecipPixel);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawStringPropDecal(const olc::v_2d<BASE>& pos, const std::string& sText, const olc::Pixel col, const olc::v_2d<BASE>& scale)
	{
		pge->DrawStringPropDecal(WorldToScreen(pos), sText, col, scale * m_vWorldScale * m_vRecipPixel);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::FillRectDecal(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, const olc::Pixel col)
	{
		pge->FillRectDecal(WorldToScreen(pos), (size * m_vWorldScale).ceil(), col);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawRectDecal(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, const olc::Pixel col)
	{
		pge->DrawRectDecal(WorldToScreen(pos), (size * m_vWorldScale).ceil(), col);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawLineDecal(const olc::v_2d<BASE>& pos1, const olc::v_2d<BASE>& pos2, Pixel p)
	{
		pge->DrawLineDecal(WorldToScreen(pos1), WorldToScreen(pos2), p);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::GradientFillRectDecal(const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, const olc::Pixel colTL, const olc::Pixel colBL, const olc::Pixel colBR, const olc::Pixel colTR)
	{
		pge->GradientFillRectDecal(WorldToScreen(pos), size * m_vWorldScale, colTL, colBL, colBR, colTR);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::v_2d<BASE>>& pos, const std::vector<olc::vf2d>& uv, const olc::Pixel tint)
	{
		std::vector<olc::vf2d> vTransformed(pos.size());
		for (uint32_t n = 0; n < pos.size(); n++)
			vTransformed[n] = WorldToScreen(pos[n]);
		pge->DrawPolygonDecal(decal, vTransformed, uv, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::v_2d<BASE>>& pos, const std::vector<olc::vf2d>& uv, const std::vector<olc::Pixel>& tint)
	{
		std::vector<olc::vf2d> vTransformed(pos.size());
		for (uint32_t n = 0; n < pos.size(); n++)
			vTransformed[n] = WorldToScreen(pos[n]);
		pge->DrawPolygonDecal(decal, vTransformed, uv, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::v_2d<BASE>>& pos, const std::vector<olc::vf2d>& uv, const std::vector<olc::Pixel>& colours, const olc::Pixel tint)
	{
		std::vector<olc::vf2d> vTransformed(pos.size());
		for (uint32_t n = 0; n < pos.size(); n++)
			vTransformed[n] = WorldToScreen(pos[n]);
		pge->DrawPolygonDecal(decal, vTransformed, uv, colours, tint);
	}



#if defined (OLC_USING_PGEX_SHADER)

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawDecal(olc::Shade& shade, const olc::v_2d<BASE>& pos, olc::Decal* decal, const olc::v_2d<BASE>& scale, const olc::Pixel& tint)
	{
		shade.DrawDecal(WorldToScreen(pos), decal, scale * m_vWorldScale * m_vRecipPixel, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialDecal(olc::Shade& shade, const olc::v_2d<BASE>& pos, olc::Decal* decal, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::v_2d<BASE>& scale, const olc::Pixel& tint)
	{
		shade.DrawPartialDecal(WorldToScreen(pos), decal, source_pos, source_size, scale * m_vWorldScale * m_vRecipPixel, tint);
	}

	template <class BASE>
	void TransformedViewTemplate<BASE>::DrawPartialDecal(olc::Shade& shade, const olc::v_2d<BASE>& pos, const olc::v_2d<BASE>& size, olc::Decal* decal, const olc::v_2d<BASE>& source_pos, const olc::v_2d<BASE>& source_size, const olc::Pixel& tint)
	{
		shade.DrawPartialDecal(WorldToScreen(pos), size * m_vWorldScale * m_vRecipPixel, decal, source_pos, source_size, tint);
	}

#endif

}





	/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////



#endif
#endif
