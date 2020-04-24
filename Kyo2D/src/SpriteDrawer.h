
#pragma once

#include "DirectXMath.h"
#include <cstdint>
using namespace DirectX;

namespace Kyo2D
{
	/// Base class for sprite rendering.
	class SpriteDrawer
	{
	public:

		/// Default constructor.
		SpriteDrawer();
		/// Destructor.
		virtual ~SpriteDrawer();

		/// 
		virtual bool Initialize() = 0;
		/// 
		virtual bool Prepare() = 0;
		///
		virtual void SetViewMatrix(const XMMATRIX &ViewMatrix) = 0;
		/// Enables or disables the Scale2X algorithm.
		virtual void SetScale2XEnabled(bool Enable) = 0;

	public:

		/// Draws a simple sprite at a given location.
		virtual void DrawSpriteAt(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey) = 0;
		/// Draws a subarea of a sprite at a given location.
		virtual void DrawSubspriteAt(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float srcX, float srcY, float srcW, float srcH, float Rotation, std::uint32_t color, std::uint32_t colorkey) = 0;
		/// Draws a sprite and stretches it to the given area.
		virtual void DrawSpriteScaled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float Rotation, std::uint32_t color, std::uint32_t colorkey) = 0;
		/// Draws a subarea of a sprite and stretches it to the given area.
		virtual void DrawSubspriteScaled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float srcX, float srcY, float srcW, float srcH, float Rotation, std::uint32_t color, std::uint32_t colorkey) = 0;
		/// Draws a sprite and tiles it in the given area.
		virtual void DrawSpriteTiled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float tX, float tY, float Rotation, std::uint32_t color, std::uint32_t colorkey) = 0;

	public:

		/// Determins whether Scale2X is enabled.
		virtual bool IsScale2XEnabled() const = 0;
	};
}
