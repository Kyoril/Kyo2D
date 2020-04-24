#pragma once

#include "FontImage.h"

namespace Kyo2D
{
	/// Represents a single glyph of a font.
	class FontGlyph
	{
	public:

		/// Initializes a new font glyph.
		explicit FontGlyph(float advance = 0.0f, FontImage* image = nullptr);

	public:

		/// Gets the horizontal advance value for the glyph.
		inline float getAdvance(float scale) const { return m_advance * scale; }
		/// Gets the rendered advance value for this glyph.
		inline float getRenderedAdvance(float scale = 1.0f) const { return (m_image->getWidth() + m_image->getOffsetX()) * scale; }
		/// Gets the image object which is rendered when this glyph is rendered.
		inline const FontImage* getImage() const { return m_image; }
		/// Sets the image object which is rendered when this glyph is rendered.
		/// @param image The new image or nullptr to not use an image at all.
		inline void setImage(const FontImage* image) { m_image = image; }

	private:

		/// The amount to advance the cursor after rendering this glyph.
		float m_advance;
		/// The image which will be rendered when this glyph is drawn.
		const FontImage* m_image;
	};

}