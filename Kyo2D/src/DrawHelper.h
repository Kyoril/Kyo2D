
#pragma once

#include <cstdint>
#include "DirectXMath.h"
using namespace DirectX;

namespace Kyo2D
{
	/// Basic class for drawing 2D geometry.
	class DrawHelper
	{
	public:

		/// Default constructor.
		DrawHelper();
		/// Destructor.
		virtual ~DrawHelper();

		/// Initializes the DrawHelper class.
		virtual bool Initialize() = 0;
		/// Prepares the DrawHelper before drawing.
		virtual void Prepare() = 0;
		/// 
		virtual void DrawPoint(float X, float Y, std::int32_t Color = -1) = 0;
		/// 
		virtual void DrawLine(float X1, float Y1, float X2, float Y2, std::int32_t Color = -1) = 0;
		/// 
		virtual void DrawRect(float X, float Y, float W, float H, std::int32_t Color = -1) = 0;
		/// 
		virtual void FillRect(float X, float Y, float W, float H, std::int32_t Color = -1) = 0;

		virtual void UpdateViewMatrix(const XMMATRIX &ViewMatrix) = 0;
	};
}
