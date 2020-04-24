
#include "SpriteDrawerD3D9.h"
#include "Kyo2D.h"
#include "shaders/d3d9/Sprite9_VS.h"
#include "shaders/d3d9/Sprite9_PS.h"
#include <algorithm>

namespace Kyo2D
{
	SpriteDrawerD3D9::SpriteDrawerD3D9()
	{
	}

	SpriteDrawerD3D9::~SpriteDrawerD3D9()
	{
	}

	bool SpriteDrawerD3D9::Initialize()
	{
		// Create the vertex shader
		HRESULT hr = g_D3DDevice9->CreateVertexShader((const DWORD*)g_sprite9MainVS, m_VertShader.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// Create the pixel shader
		hr = g_D3DDevice9->CreatePixelShader((const DWORD*)g_sprite9MainPS, m_PixShader.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// Create vertex buffer
		hr = g_D3DDevice9->CreateVertexBuffer(
			sizeof(SpriteVertex2D) * 4,
			D3DUSAGE_WRITEONLY,
			D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX0 | D3DFVF_TEX1,
			D3DPOOL_MANAGED,
			m_GeomBuffer.GetAddressOf(),
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
			{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,    0 },
			{ 0, 24, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,    1 },
			D3DDECL_END()
		};
		hr = g_D3DDevice9->CreateVertexDeclaration(declaration, m_VertexDecl.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// Initialize view matrix
		m_Matrices[0] = XMMatrixIdentity();
		m_Matrices[1] = XMMatrixIdentity();
		return true;
	}

	bool SpriteDrawerD3D9::Prepare()
	{
		// Setup shaders
		g_D3DDevice9->SetVertexShader(m_VertShader.Get());
		g_D3DDevice9->SetPixelShader(m_PixShader.Get());

		XMFLOAT4X4 d3dmatrix;
		XMStoreFloat4x4(&d3dmatrix, m_Matrices[0]);
		HRESULT hr = g_D3DDevice9->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&d3dmatrix);
		if (FAILED(hr))
		{
			return false;
		}

		hr = g_D3DDevice9->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX0 | D3DFVF_TEX1);
		if (FAILED(hr))
		{
			return false;
		}

		// Setup vertex buffer
		hr = g_D3DDevice9->SetStreamSource(0, m_GeomBuffer.Get(), 0, sizeof(SpriteVertex2D));
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	void SpriteDrawerD3D9::SetViewMatrix(const XMMATRIX & ViewMatrix)
	{
		m_Matrices[0] = ViewMatrix;
	}

	void SpriteDrawerD3D9::SetScale2XEnabled(bool Enable)
	{
		// TODO
	}

	template<typename T>
	inline T Color32Reverse(T x)
	{
		return
			// Source is in format: 0xAABBGGRR
			((x & 0xFF000000)) |		//AA______
			((x & 0x000000FF) << 16) |	//__RR____
			((x & 0x0000FF00)) |		//__GG____
			((x & 0x00FF0000) >> 16);	//BB______
										// Return value is in format:  0xAARRGGBB
	}

	template<typename T>
	inline void Color32Split(T x, float &r, float &g, float &b, float &a)
	{
		a = ((x & 0xFF000000) >> 24) / 255.0f;
		r = ((x & 0x000000FF)) / 255.0f;
		g = ((x & 0x0000FF00) >> 8) / 255.0f;
		b = ((x & 0x00FF0000) >> 16) / 255.0f;
	}

	void SpriteDrawerD3D9::DrawSpriteAt(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		SpriteVertex2D *Vertices;
		HRESULT hr = m_GeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			return;
		}

		std::uint32_t reversed = Color32Reverse(color);

		float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
		Color32Split(colorkey, r, g, b, a);

		Vertices[0] = { texW * -0.5f, texH *  0.5f, Z, reversed, 0.0f, 1.0f, r, g, b, a };
		Vertices[1] = { texW * -0.5f, texH * -0.5f, Z, reversed, 0.0f, 0.0f, r, g, b, a };
		Vertices[2] = { texW *  0.5f, texH *  0.5f, Z, reversed, 1.0f, 1.0f, r, g, b, a };
		Vertices[3] = { texW *  0.5f, texH * -0.5f, Z, reversed, 1.0f, 0.0f, r, g, b, a };
		hr = m_GeomBuffer->Unlock();
		if (FAILED(hr))
		{
			return;
		}

		// Apply rotation
		UpdateTransform(X + texW * 0.5f, Y + texH * 0.5f, Rotation);

		g_D3DDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}

	void SpriteDrawerD3D9::DrawSubspriteAt(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float srcX, float srcY, float srcW, float srcH, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		float srcU = srcX / (float)texW;
		float srcV = srcY / (float)texH;
		float dstU = srcU + (srcW / (float)texW);
		float dstV = srcV + (srcH / (float)texH);

		SpriteVertex2D *Vertices;
		HRESULT hr = m_GeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			return;
		}

		std::uint32_t reversed = Color32Reverse(color);

		float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
		Color32Split(colorkey, r, g, b, a);

		Vertices[0] = { srcW * -0.5f, srcH *  0.5f, Z, reversed, srcU, dstV, r, g, b, a };
		Vertices[1] = { srcW * -0.5f, srcH * -0.5f, Z, reversed, srcU, srcV, r, g, b, a };
		Vertices[2] = { srcW *  0.5f, srcH *  0.5f, Z, reversed, dstU, dstV, r, g, b, a };
		Vertices[3] = { srcW *  0.5f, srcH * -0.5f, Z, reversed, dstU, srcV, r, g, b, a };
		hr = m_GeomBuffer->Unlock();
		if (FAILED(hr))
		{
			return;
		}

		// Apply rotation
		UpdateTransform(X + srcW * 0.5f, Y + srcH * 0.5f, Rotation);

		g_D3DDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}

	void SpriteDrawerD3D9::DrawSpriteScaled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		SpriteVertex2D *Vertices;
		HRESULT hr = m_GeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			return;
		}

		std::uint32_t reversed = Color32Reverse(color);
		
		float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
		Color32Split(colorkey, r, g, b, a);

		Vertices[0] = { W * -0.5f, H *  0.5f, Z, reversed, 0.0f, 1.0f, r, g, b, a };
		Vertices[1] = { W * -0.5f, H * -0.5f, Z, reversed, 0.0f, 0.0f, r, g, b, a };
		Vertices[2] = { W *  0.5f, H *  0.5f, Z, reversed, 1.0f, 1.0f, r, g, b, a };
		Vertices[3] = { W *  0.5f, H * -0.5f, Z, reversed, 1.0f, 0.0f, r, g, b, a };
		hr = m_GeomBuffer->Unlock();
		if (FAILED(hr))
		{
			return;
		}

		// Apply rotation
		UpdateTransform(X + std::max<float>(W * 0.5f, W * -0.5f), Y + std::max<float>(H * 0.5f, H * -0.5f), Rotation);

		g_D3DDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}

	void SpriteDrawerD3D9::DrawSubspriteScaled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float srcX, float srcY, float srcW, float srcH, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		float srcU = srcX / (float)texW;
		float srcV = srcY / (float)texH;
		float dstU = srcU + (srcW / (float)texW);
		float dstV = srcV + (srcH / (float)texH);

		SpriteVertex2D *Vertices;
		HRESULT hr = m_GeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			return;
		}

		std::uint32_t reversed = Color32Reverse(color);

		float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
		Color32Split(colorkey, r, g, b, a);

		Vertices[0] = { W * -0.5f, H *  0.5f, Z, reversed, srcU, dstV, r, g, b, a };
		Vertices[1] = { W * -0.5f, H * -0.5f, Z, reversed, srcU, srcV, r, g, b, a };
		Vertices[2] = { W *  0.5f, H *  0.5f, Z, reversed, dstU, dstV, r, g, b, a };
		Vertices[3] = { W *  0.5f, H * -0.5f, Z, reversed, dstU, srcV, r, g, b, a };
		hr = m_GeomBuffer->Unlock();
		if (FAILED(hr))
		{
			return;
		}

		// Apply rotation
		UpdateTransform(X + std::max<float>(W * 0.5f, W * -0.5f), Y + std::max<float>(H * 0.5f, H * -0.5f), Rotation);

		g_D3DDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}

	void SpriteDrawerD3D9::DrawSpriteTiled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float tX, float tY, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		SpriteVertex2D *Vertices;
		HRESULT hr = m_GeomBuffer->Lock(0, 0, (void**)&Vertices, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			return;
		}

		std::uint32_t reversed = Color32Reverse(color);

		float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
		Color32Split(colorkey, r, g, b, a);

		Vertices[0] = { W * -0.5f, H *  0.5f, Z, reversed, 0.0f,			1.0f * tY,	r, g, b, a };
		Vertices[1] = { W * -0.5f, H * -0.5f, Z, reversed, 0.0f,			0.0f,		r, g, b, a };
		Vertices[2] = { W *  0.5f, H *  0.5f, Z, reversed, 1.0f * tX,	1.0f * tY,	r, g, b, a };
		Vertices[3] = { W *  0.5f, H * -0.5f, Z, reversed, 1.0f * tX,	0.0f,		r, g, b, a };
		hr = m_GeomBuffer->Unlock();
		if (FAILED(hr))
		{
			return;
		}

		// Apply rotation
		UpdateTransform(X + W * 0.5f, Y + H * 0.5f, Rotation);

		g_D3DDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}

	void SpriteDrawerD3D9::UpdateTransform(float x, float y, float Rotation)
	{
		m_Matrices[1] =
			XMMatrixRotationZ(Rotation) *
			XMMatrixTranslation(x, y, 0.0f);

		g_D3DDevice9->SetVertexDeclaration(m_VertexDecl.Get());
		g_D3DDevice9->SetVertexShaderConstantF(0, (const float*)&m_Matrices[0].r[0], 8);
	}
}
