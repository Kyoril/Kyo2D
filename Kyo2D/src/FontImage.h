#pragma once

#include "RectF.h"

namespace Kyo2D
{
	class FontImageset;

	/// Class that represents a single glyph image in a glyph image set.
	class FontImage
	{
	public:

		FontImage();
		explicit FontImage(const FontImageset& owner, const RectF& area, const Vector2& renderOffset, float horzScaling, float vertScaling);

		FontImage(FontImage&&) = default;
		FontImage& operator=(FontImage&&) = default;
		FontImage(const FontImage&) = default;
		FontImage& operator=(const FontImage&) = default;

	public:

		/// Gets the size of this image in pixels.
		inline const Vector2& getSize() const { return m_scaledSize; }
		/// Gets the width of this image in pixels.
		inline float getWidth() const { return m_scaledSize.X; }
		/// Gets the height of this image in pixels.
		inline float getHeight() const { return m_scaledSize.Y; }
		/// Gets the start offset of this image in pixels.
		inline const Vector2& getOffset() const { return m_scaledOffset; }
		/// Gets the x coordinate of the start offset of this image in pixels.
		inline float getOffsetX() const { return m_scaledOffset.X; }
		/// Gets the y coordinate of the start offset of this image in pixels.
		inline float getOffsetY() const { return m_scaledOffset.Y; }

	public:

		/// A rectangle which describes the source texture area used by this image.
		const RectF& getSourceTextureArea() const;
		/// Queues the image to be drawn.
		/// @param position The position of the image.
		/// @param size The size with which the image will be drawn.
		void draw(const Vector2& position, const Vector2& size) const;

	private:

		const FontImageset* m_owner;
		RectF m_area;
		Vector2 m_offset;
		Vector2 m_scaledSize;
		Vector2 m_scaledOffset;
	};

}