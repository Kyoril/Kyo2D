
#pragma once

#include "../DrawHelper.h"
#include <Windows.h>
#include <comptr.h>
#include <d3d9.h>
using namespace Microsoft::WRL;

extern ComPtr<IDirect3D9> g_D3D9;
extern ComPtr<IDirect3DDevice9> g_D3DDevice9;

namespace Kyo2D
{
	class DrawHelperD3D9 : public DrawHelper
	{
	public:

		/// Default constructor.
		DrawHelperD3D9();
		/// Destructor.
		virtual ~DrawHelperD3D9();

		/// @copydoc DrawHelper::Initialize()
		virtual bool Initialize() override;
		/// @copydoc DrawHelper::Prepare()
		virtual void Prepare() override;
		/// @copydoc DrawHelper::DrawPoint()
		virtual void DrawPoint(float X, float Y, std::int32_t Color = -1) override;
		/// @copydoc DrawHelper::DrawLine()
		virtual void DrawLine(float X1, float Y1, float X2, float Y2, std::int32_t Color = -1) override;
		/// @copydoc DrawHelper::DrawRect()
		virtual void DrawRect(float X, float Y, float W, float H, std::int32_t Color = -1) override;
		/// @copydoc DrawHelper::FillRect()
		virtual void FillRect(float X, float Y, float W, float H, std::int32_t Color = -1) override;
		/// @copydoc DrawHelper::UpdateViewMatrix()
		virtual void UpdateViewMatrix(const XMMATRIX &ViewMatrix) override;

	private:

		/// 
		void Dispatch(D3DPRIMITIVETYPE Topology, UINT Count);

	private:

		ComPtr<IDirect3DVertexShader9> m_VertShader2D;
		ComPtr<IDirect3DPixelShader9> m_PixShader2D;
		ComPtr<IDirect3DVertexBuffer9> m_2DGeomBuffer;
		ComPtr<IDirect3DVertexDeclaration9> m_VertexDecl;
		XMMATRIX m_ViewMatrix;
	};
}
