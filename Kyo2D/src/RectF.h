#pragma once

#include "Vector2.h"

namespace Kyo2D
{

	/// A floating point rectangle.
	class RectF
	{
	public:
		/// X coordinate
		float X;
		/// Y coordinate
		float Y;
		/// Width
		float Width;
		/// Height
		float Height;

	public:

		/// Default constructor initializes an empty rectangle.
		RectF()
			: X(0.0f)
			, Y(0.0f)
			, Width(0.0f)
			, Height(0.0f)
		{
		}
		/// Initializes a rectangle with the given parameters.
		/// @param x The new x coordinate of the rectangle.
		/// @param y The new y coordinate of the rectangle.
		/// @param width The new width of the rectangle.
		/// @param height The new height of the rectangle.
		explicit RectF(float x, float y, float width, float height)
			: X(x)
			, Y(y)
			, Width(width)
			, Height(height)
		{
		}
		/// Initializes a rectangle with the given parameters.
		/// @param offset The new coordinates of the rectangle offset.
		/// @param size Thew new size of the rectangle.
		explicit RectF(const Vector2& offset, const Vector2& size)
			: X(offset.X)
			, Y(offset.Y)
			, Width(size.X)
			, Height(size.Y)
		{
		}
	};
}
