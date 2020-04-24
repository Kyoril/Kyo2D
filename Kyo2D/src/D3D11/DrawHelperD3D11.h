
#pragma once

#include "../DrawHelper.h"
#include <Windows.h>
#include <comptr.h>
#include <d3d11.h>
using namespace Microsoft::WRL;

extern ComPtr<ID3D11Device> g_D3DDevice11;
extern ComPtr<ID3D11DeviceContext> g_D3DDeviceContext11;

namespace Kyo2D
{
	class DrawHelperD3D11 : public DrawHelper
	{
	public:

		/// Default constructor.
		DrawHelperD3D11();
		/// Destructor.
		virtual ~DrawHelperD3D11();

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
		void Dispatch(D3D11_PRIMITIVE_TOPOLOGY Topology, UINT Count);

	private:

		ComPtr<ID3D11VertexShader> m_VertShader2D;
		ComPtr<ID3D11PixelShader> m_PixShader2D;
		ComPtr<ID3D11Buffer> m_2DGeomBuffer;
		ComPtr<ID3D11Buffer> m_2DCBuffer;
		ComPtr<ID3D11InputLayout> m_2DInputLayout;
		ComPtr<ID3D11BlendState> m_2DBlendState;
	};
}
