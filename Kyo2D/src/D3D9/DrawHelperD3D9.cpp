
#include "DrawHelperD3D9.h"
#include "Kyo2D.h"
#include "shaders/d3d9/Draw2D9_VS.h"
#include "shaders/d3d9/Draw2D9_PS.h"

namespace Kyo2D
{
	DrawHelperD3D9::DrawHelperD3D9()
		: DrawHelper()
	{
	}

	DrawHelperD3D9::~DrawHelperD3D9()
	{
	}

	bool DrawHelperD3D9::Initialize()
	{
		// Create the vertex shader
		HRESULT hr = g_D3DDevice9->CreateVertexShader((const DWORD*)g_draw2D9MainVS, m_VertShader2D.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// Create the pixel shader
		hr = g_D3DDevice9->CreatePixelShader((const DWORD*)g_draw2D9MainPS, m_PixShader2D.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// Create vertex buffer
		hr = g_D3DDevice9->CreateVertexBuffer(
			sizeof(Vertex2D) * 5, 
			D3DUSAGE_WRITEONLY, 
			D3DFVF_XYZ | D3DFVF_DIFFUSE, 
			D3DPOOL_MANAGED, 
			m_2DGeomBuffer.GetAddressOf(), 
			nullptr);
		if (FAILED(hr))
		{
			return false;
		}

		// Create input layout
		D3DVERTEXELEMENT9 declaration[] =
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
			D3DDECL_END()
		};
		g_D3DDevice9->CreateVertexDeclaration(declaration, m_VertexDecl.GetAddressOf());

		// Initialize view matrix
		m_ViewMatrix = XMMatrixIdentity();
		return true;
	}

	void DrawHelperD3D9::Prepare()
	{
		// Setup shaders
		g_D3DDevice9->SetVertexShader(m_VertShader2D.Get());
		g_D3DDevice9->SetPixelShader(m_PixShader2D.Get());

		XMFLOAT4X4 d3dmatrix;
		XMStoreFloat4x4(&d3dmatrix, m_ViewMatrix);
		HRESULT hr = g_D3DDevice9->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&d3dmatrix);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"SetTransform failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		g_D3DDevice9->SetVertexDeclaration(m_VertexDecl.Get());
		g_D3DDevice9->SetVertexShaderConstantF(0, (const float*)&m_ViewMatrix.r[0], 4);
		hr = g_D3DDevice9->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"SetFVF failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		// Setup vertex buffer
		hr = g_D3DDevice9->SetStreamSource(0, m_2DGeomBuffer.Get(), 0, sizeof(Vertex2D));
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"SetStreamSource failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}
	}

	template<typename T>
	inline T Color32Reverse(T x)
	{
		return
			// Source is in format: 0xAABBGGRR
			((x & 0xFF000000)) |		//AA______
			((x & 0x000000FF) << 16) |	//__RR____
			((x & 0x0000FF00)) |		//__GG____
			((x & 0x00FF00FF) >> 16);	//BB______
			// Return value is in format:  0xAARRGGBB
	}

	void DrawHelperD3D9::DrawPoint(float X, float Y, std::int32_t Color)
	{
		Vertex2D *Vertices;
		HRESULT hr = m_2DGeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Lock failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		Vertices[0] = { X, Y, 0.0f, Color32Reverse(Color) };
		hr = m_2DGeomBuffer->Unlock();
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Unlock failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		// Draw command
		Dispatch(D3DPT_POINTLIST, 1);
	}

	void DrawHelperD3D9::DrawLine(float X1, float Y1, float X2, float Y2, std::int32_t Color)
	{
		Vertex2D *Vertices;
		HRESULT hr = m_2DGeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Lock failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		std::int32_t reversed = Color32Reverse(Color);
		Vertices[0] = { X1, Y1, 0.0f, reversed };
		Vertices[1] = { X2, Y2, 0.0f, reversed };
		hr = m_2DGeomBuffer->Unlock();
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Unlock failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		// Draw command
		Dispatch(D3DPT_LINESTRIP, 1);
	}

	void DrawHelperD3D9::DrawRect(float X, float Y, float W, float H, std::int32_t Color)
	{
		Vertex2D *Vertices;
		HRESULT hr = m_2DGeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Lock failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		std::int32_t reversed = Color32Reverse(Color);
		Vertices[0] = { X + 0, Y + H, 0.0f, reversed };
		Vertices[1] = { X + 0, Y + 0, 0.0f, reversed };
		Vertices[2] = { X + W, Y + 0, 0.0f, reversed };
		Vertices[3] = { X + W, Y + H, 0.0f, reversed };
		Vertices[4] = { X + 0, Y + H, 0.0f, reversed };
		hr = m_2DGeomBuffer->Unlock();
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Unlock failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		// Draw command
		Dispatch(D3DPT_LINESTRIP, 4);
	}

	void DrawHelperD3D9::FillRect(float X, float Y, float W, float H, std::int32_t Color)
	{
		Vertex2D *Vertices;
		HRESULT hr = m_2DGeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Lock failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		std::int32_t reversed = Color32Reverse(Color);
		Vertices[0] = { X, Y + H,		0.0f, reversed };
		Vertices[1] = { X, Y,			0.0f, reversed };
		Vertices[2] = { X + W, Y + H, 0.0f, reversed };
		Vertices[3] = { X + W, Y,		0.0f, reversed };
		hr = m_2DGeomBuffer->Unlock();
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Unlock failed", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		// Draw command
		Dispatch(D3DPT_TRIANGLESTRIP, 2);
	}

	void DrawHelperD3D9::UpdateViewMatrix(const XMMATRIX & ViewMatrix)
	{
		m_ViewMatrix = ViewMatrix;
	}

	void DrawHelperD3D9::Dispatch(D3DPRIMITIVETYPE Topology, UINT Count)
	{
		HRESULT hr = g_D3DDevice9->DrawPrimitive(Topology, 0, Count);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"DrawPrimitive failed", L"Error", MB_ICONERROR | MB_OK);
		}
	}
}
