#include "FontImageset.h"
#include "Kyo2D.h"

namespace Kyo2D
{
	FontImageset::FontImageset()
		: m_textureId(0)
	{
	}
	FontImageset::~FontImageset()
	{
		if (m_textureId != 0)
		{
			K2D_DestroyTexture(m_textureId);
			m_textureId = 0;
		}
	}
	void FontImageset::setTexture(std::uint32_t textureId)
	{
		if (m_textureId != 0)
		{
			K2D_DestroyTexture(m_textureId);
			m_textureId = 0;
		}

		m_textureId = textureId;
	}
	//=============================================================================
	FontImage & FontImageset::defineImage(const Vector2 & position, const Vector2 & size, const Vector2 & renderOffset)
	{
		return defineImage(RectF(position, size), renderOffset);
	}

	//=============================================================================
	FontImage & FontImageset::defineImage(const RectF & imageRect, const Vector2 & renderOffset)
	{
		m_images.emplace_back(FontImage(*this, imageRect, renderOffset, 1.0f, 1.0f));
		return m_images.back();
	}
	
	//=============================================================================
	void FontImageset::draw(const RectF & srcRect, const RectF & dstRect) const
	{
		if (m_textureId != 0)
		{
			// Draw the sprite!
			K2D_DrawSubspriteScaled(m_textureId,
				dstRect.X, dstRect.Y, dstRect.Width, dstRect.Height,
				srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height,
				999.0f,
				0.0f, 0xffffffff, 0);
		}
	}
}