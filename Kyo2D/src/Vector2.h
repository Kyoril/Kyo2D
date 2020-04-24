#pragma once

namespace Kyo2D
{
	/// A 2-dimensional floating point vector class.
	class Vector2
	{
	public:
		/// X component of the vector.
		float X;
		/// Y component of the vector.
		float Y;

	public:
		/// Default constructor initializes an empty vector.
		Vector2()
			: X(0.0f)
			, Y(0.0f)
		{
		}
		/// Initializes a vector and sets both coordinates to the given parameter.
		/// @param a The new value which will be applied to both coordinates.
		explicit Vector2(float a)
			: X(a)
			, Y(a)
		{
		}
		/// Initializes a vector with custom coordinates.
		/// @param x The new x coordinate of the vector.
		/// @param y The new y coordinate of the vector.
		explicit Vector2(float x, float y)
			: X(x)
			, Y(y)
		{
		}

	public:
		// Assignment, Copy and Move methods
		explicit Vector2(const Vector2& Other) = default;
		Vector2& operator=(const Vector2& Other) = default;
		explicit Vector2(Vector2 &&Other) = default;
		Vector2& operator=(Vector2&& Other) = default;
	};
}
