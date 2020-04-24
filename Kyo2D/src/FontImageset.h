#pragma once

#include "FontImage.h"
#include <list>

namespace Kyo2D
{
	/// This class contains image data and splits it into multiple images.
	/// For example, each glyph has a FontImage object reference, which specifies
	/// which area of the imageset texture is used to render this specific image.
	class FontImageset
	{
	public:

		FontImageset();
		~FontImageset();

		FontImageset(FontImageset&&) = default;
		FontImageset& operator=(FontImageset&&) = default;
		FontImageset(const FontImageset&) = default;
		FontImageset& operator=(const FontImageset&) = default;

	public:

		void setTexture(std::uint32_t textureId);
		FontImage& defineImage(const Vector2& position, const Vector2& size, const Vector2& renderOffset);
		FontImage& defineImage(const RectF& imageRect, const Vector2& renderOffset);

		void draw(const RectF& srcRect, const RectF& dstRect) const;

	private:

		/// Gets the texture id of this font.
		std::uint32_t m_textureId;
		/// Contains a list of all images.
		std::list<FontImage> m_images;
	};
}