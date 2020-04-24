#include "FontImage.h"
#include "FontImageset.h"


namespace Kyo2D
{
	//=============================================================================
	FontImage::FontImage()
		: m_owner(nullptr)
	{
	}

	#define PixelAligned(x)	( (float)(int)(( x ) + (( x ) > 0.0f ? 0.5f : -0.5f)) )

	//=============================================================================
	FontImage::FontImage(const FontImageset & owner, const RectF & area, const Vector2 & renderOffset, float horzScaling, float vertScaling)
		: m_owner(&owner)
		, m_area(area)
		, m_offset(renderOffset)
	{
		m_scaledSize = Vector2(PixelAligned(m_area.Width * horzScaling), PixelAligned(m_area.Height * vertScaling));
		m_scaledOffset = Vector2(PixelAligned(m_offset.X * horzScaling), PixelAligned(m_offset.Y * vertScaling));
	}

	//=============================================================================
	const RectF & FontImage::getSourceTextureArea() const
	{
		return m_area;
	}

	//=============================================================================
	void FontImage::draw(const Vector2 & position, const Vector2 & size) const
	{
		if (!m_owner)
			return;

		m_owner->draw(m_area, RectF(Vector2(position.X + m_scaledOffset.X, position.Y + m_scaledOffset.Y), size));
	}
}
