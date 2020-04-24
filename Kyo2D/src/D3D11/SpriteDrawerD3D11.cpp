
#include "SpriteDrawerD3D11.h"
#include "shaders/d3d11/Sprite11_VS.h"
#include "shaders/d3d11/Sprite11_PS.h"
#include "shaders/d3d11/SpriteScale2X11_PS.h"
#include "shaders/d3d11/SpriteScale2X11_VS.h"
#include <algorithm>

namespace Kyo2D
{
	SpriteDrawerD3D11::SpriteDrawerD3D11()
		: m_Scale2XEnabled(true)
	{
	}

	SpriteDrawerD3D11::~SpriteDrawerD3D11()
	{
	}

	bool SpriteDrawerD3D11::Initialize()
	{
		if (!CreateSpriteShaders())
			return false;

		if (!CreateScale2XShaders())
			return false;

		if (!CreateSampler())
			return false;

		if (!CreateBlendState())
			return false;

		if (!CreateRasterState())
			return false;

		if (!CreateBuffers())
			return false;

		return true;
	}

	bool SpriteDrawerD3D11::Prepare()
	{
		g_D3DDeviceContext11->RSSetState(m_RasterState.Get());

		// Prepare the sprite renderer
		if (!m_Scale2XEnabled)
		{
			// Setup shader objects
			g_D3DDeviceContext11->VSSetShader(m_VertShaderSprite.Get(), 0, 0);
			g_D3DDeviceContext11->PSSetShader(m_PixShaderSprite.Get(), 0, 0);

			g_D3DDeviceContext11->PSSetSamplers(0, 1, m_SpriteSampler.GetAddressOf());

			// Setup blend desc
			g_D3DDeviceContext11->OMSetBlendState(m_BlendState.Get(), 0, 0xFFFFFFFF);

			ID3D11Buffer *buffers[] = {
				m_ViewBuffer.Get(),
				m_PerObjCBuffer.Get()
			};
			g_D3DDeviceContext11->VSSetConstantBuffers(0, 2, buffers);

			// Set input layout
			g_D3DDeviceContext11->IASetInputLayout(m_SpriteInputLayout.Get());
		}
		else
		{
			// Setup shader objects
			g_D3DDeviceContext11->VSSetShader(m_VertShaderSpriteScale2X.Get(), 0, 0);
			g_D3DDeviceContext11->PSSetShader(m_PixShaderSpriteScale2X.Get(), 0, 0);

			// Set texture sampler
			g_D3DDeviceContext11->PSSetSamplers(0, 1, m_SpriteSampler.GetAddressOf());

			// Setup blend desc
			g_D3DDeviceContext11->OMSetBlendState(m_BlendState.Get(), 0, 0xFFFFFFFF);

			ID3D11Buffer *buffers[] = {
				m_ViewBuffer.Get(),
				m_PerObjCBuffer.Get()
			};
			g_D3DDeviceContext11->VSSetConstantBuffers(0, 2, buffers);

			// Set input layout
			g_D3DDeviceContext11->IASetInputLayout(m_SpriteScale2XInputLayout.Get());
		}

		return true;
	}

	void SpriteDrawerD3D11::SetViewMatrix(const XMMATRIX & ViewMatrix)
	{
		g_D3DDeviceContext11->UpdateSubresource(m_ViewBuffer.Get(), 0, nullptr, &ViewMatrix, 0, 0);
	}

	void SpriteDrawerD3D11::SetScale2XEnabled(bool Enable)
	{
		m_Scale2XEnabled = Enable;
	}

	void SpriteDrawerD3D11::DrawSpriteAt(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		// Generate vertices
		SpriteVertex2D Vertices[] =
		{
			// 2--4
			// | /|
			// |/ |
			// 1--3
			{ texW * -0.5f, texH *  0.5f, Z, color, 0.0f, 1.0f, colorkey },
			{ texW * -0.5f, texH * -0.5f, Z, color, 0.0f, 0.0f, colorkey },
			{ texW *  0.5f, texH *  0.5f, Z, color, 1.0f, 1.0f, colorkey },
			{ texW *  0.5f, texH * -0.5f, Z, color, 1.0f, 0.0f, colorkey }
		};

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_SpriteGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, Vertices, sizeof(Vertices));														// copy the data
			g_D3DDeviceContext11->Unmap(m_SpriteGeomBuffer.Get(), 0);												// unmap the buffer
		}

		// Apply size
		m_PerObjectBuffer.TargetWidth = static_cast<FLOAT>(texW);
		m_PerObjectBuffer.TargetHeight = static_cast<FLOAT>(texH);
		m_PerObjectBuffer.RawWidth = static_cast<FLOAT>(texW);
		m_PerObjectBuffer.RawHeight = static_cast<FLOAT>(texH);

		// Apply rotation
		UpdateTransform(X + texW * 0.5f, Y + texH * 0.5f, Rotation);

		Dispatch();
	}

	void SpriteDrawerD3D11::DrawSubspriteAt(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float srcX, float srcY, float srcW, float srcH, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		float srcU = srcX / (float)texW;
		float srcV = srcY / (float)texH;
		float dstU = srcU + (srcW / (float)texW);
		float dstV = srcV + (srcH / (float)texH);

		// Generate vertices
		SpriteVertex2D Vertices[] =
		{
			// 2--4
			// | /|
			// |/ |
			// 1--3
			{ srcW * -0.5f, srcH *  0.5f, Z, color, srcU, dstV, colorkey },
			{ srcW * -0.5f, srcH * -0.5f, Z, color, srcU, srcV, colorkey },
			{ srcW *  0.5f, srcH *  0.5f, Z, color, dstU, dstV, colorkey },
			{ srcW *  0.5f, srcH * -0.5f, Z, color, dstU, srcV, colorkey }
		};

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_SpriteGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, Vertices, sizeof(Vertices));														// copy the data
			g_D3DDeviceContext11->Unmap(m_SpriteGeomBuffer.Get(), 0);												// unmap the buffer
		}

		m_PerObjectBuffer.TargetWidth = srcW;
		m_PerObjectBuffer.TargetHeight = srcH;
		m_PerObjectBuffer.RawWidth = static_cast<FLOAT>(texW);
		m_PerObjectBuffer.RawHeight = static_cast<FLOAT>(texH);

		// Apply rotation
		UpdateTransform(X + srcW * 0.5f, Y + srcH * 0.5f, Rotation);

		Dispatch();
	}

	void SpriteDrawerD3D11::DrawSpriteScaled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		// Generate vertices
		SpriteVertex2D Vertices[] =
		{
			// 2--4
			// | /|
			// |/ |
			// 1--3
			{ W * -0.5f, H *  0.5f, Z, color, 0.0f, 1.0f, colorkey },
			{ W * -0.5f, H * -0.5f, Z, color, 0.0f, 0.0f, colorkey },
			{ W *  0.5f, H *  0.5f, Z, color, 1.0f, 1.0f, colorkey },
			{ W *  0.5f, H * -0.5f, Z, color, 1.0f, 0.0f, colorkey }
		};

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_SpriteGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, Vertices, sizeof(Vertices));														// copy the data
			g_D3DDeviceContext11->Unmap(m_SpriteGeomBuffer.Get(), 0);												// unmap the buffer
		}

		m_PerObjectBuffer.TargetWidth = W;
		m_PerObjectBuffer.TargetHeight = H;
		m_PerObjectBuffer.RawWidth = static_cast<FLOAT>(texW);
		m_PerObjectBuffer.RawHeight = static_cast<FLOAT>(texH);

		// Apply rotation
		UpdateTransform(X + std::max<float>(W * 0.5f, W * -0.5f), Y + std::max<float>(H * 0.5f, H * -0.5f), Rotation);

		Dispatch();
	}

	void SpriteDrawerD3D11::DrawSubspriteScaled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float srcX, float srcY, float srcW, float srcH, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		float srcU = srcX / (float)texW;
		float srcV = srcY / (float)texH;
		float dstU = srcU + (srcW / (float)texW);
		float dstV = srcV + (srcH / (float)texH);

		// Generate vertices
		SpriteVertex2D Vertices[] =
		{
			// 2--4
			// | /|
			// |/ |
			// 1--3
			{ W * -0.5f, H *  0.5f, Z, color, srcU, dstV, colorkey },
			{ W * -0.5f, H * -0.5f, Z, color, srcU, srcV, colorkey },
			{ W *  0.5f, H *  0.5f, Z, color, dstU, dstV, colorkey },
			{ W *  0.5f, H * -0.5f, Z, color, dstU, srcV, colorkey }
		};

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_SpriteGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, Vertices, sizeof(Vertices));														// copy the data
			g_D3DDeviceContext11->Unmap(m_SpriteGeomBuffer.Get(), 0);												// unmap the buffer
		}

		m_PerObjectBuffer.TargetWidth = W;
		m_PerObjectBuffer.TargetHeight = H;
		m_PerObjectBuffer.RawWidth = static_cast<FLOAT>(texW);
		m_PerObjectBuffer.RawHeight = static_cast<FLOAT>(texH);

		// Apply rotation
		UpdateTransform(X + std::max<float>(W * 0.5f, W * -0.5f), Y + std::max<float>(H * 0.5f, H * -0.5f), Rotation);

		Dispatch();
	}

	void SpriteDrawerD3D11::DrawSpriteTiled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float tX, float tY, float Rotation, std::uint32_t color, std::uint32_t colorkey)
	{
		// Generate vertices
		SpriteVertex2D Vertices[] =
		{
			// 2--4
			// | /|
			// |/ |
			// 1--3
			{ W * -0.5f, H *  0.5f, Z, color, 0.0f,		 1.0f * tY, colorkey },
			{ W * -0.5f, H * -0.5f, Z, color, 0.0f,		 0.0f	  , colorkey },
			{ W *  0.5f, H *  0.5f, Z, color, 1.0f * tX, 1.0f * tY, colorkey },
			{ W *  0.5f, H * -0.5f, Z, color, 1.0f * tX, 0.0f	  , colorkey }
		};

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_SpriteGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, Vertices, sizeof(Vertices));														// copy the data
			g_D3DDeviceContext11->Unmap(m_SpriteGeomBuffer.Get(), 0);												// unmap the buffer
		}

		m_PerObjectBuffer.TargetWidth = W;
		m_PerObjectBuffer.TargetHeight = H;
		m_PerObjectBuffer.RawWidth = static_cast<FLOAT>(texW);
		m_PerObjectBuffer.RawHeight = static_cast<FLOAT>(texH);

		// Apply rotation
		UpdateTransform(X + W * 0.5f, Y + H * 0.5f, Rotation);

		Dispatch();
	}

	bool SpriteDrawerD3D11::CreateSpriteShaders()
	{
		// Load sprite vertex shader
		HRESULT hr = g_D3DDevice11->CreateVertexShader(g_sprite11MainVS, sizeof(g_sprite11MainVS), nullptr, m_VertShaderSprite.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create sprite vertex shader!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Create sprite input layout
		D3D11_INPUT_ELEMENT_DESC iedSprite[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		hr = g_D3DDevice11->CreateInputLayout(iedSprite, 4, g_sprite11MainVS, sizeof(g_sprite11MainVS), m_SpriteInputLayout.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create sprite input layout!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Load pixel shader
		hr = g_D3DDevice11->CreatePixelShader(g_sprite11MainPS, sizeof(g_sprite11MainPS), nullptr, m_PixShaderSprite.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create sprite pixel shader!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		return true;
	}

	bool SpriteDrawerD3D11::CreateScale2XShaders()
	{
		// Load sprite vertex shader
		HRESULT hr = g_D3DDevice11->CreateVertexShader(g_spriteScale2X11MainVS, sizeof(g_spriteScale2X11MainVS), nullptr, m_VertShaderSpriteScale2X.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create scale2x vertex shader!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Create sprite input layout
		D3D11_INPUT_ELEMENT_DESC iedSprite[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		hr = g_D3DDevice11->CreateInputLayout(iedSprite, 4, g_spriteScale2X11MainVS, sizeof(g_spriteScale2X11MainVS), m_SpriteScale2XInputLayout.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create scale2x input layout!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Load pixel shader
		hr = g_D3DDevice11->CreatePixelShader(g_spriteScale2X11MainPS, sizeof(g_spriteScale2X11MainPS), nullptr, m_PixShaderSpriteScale2X.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create scale2x pixel shader!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		return true;
	}

	bool SpriteDrawerD3D11::CreateSampler()
	{
		// Create sprite sampler
		D3D11_SAMPLER_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		sd.MaxAnisotropy = 0;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.MinLOD = 0.0f;
		sd.MaxLOD = 0.0f;
		sd.MipLODBias = 0.0f;
		HRESULT hr = g_D3DDevice11->CreateSamplerState(&sd, m_SpriteSampler.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create sprite sampler!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		return true;
	}

	bool SpriteDrawerD3D11::CreateBlendState()
	{
		// Create the blend state to enable alpha blending
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		HRESULT hr = g_D3DDevice11->CreateBlendState(&blendDesc, m_BlendState.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create blend state!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		return true;
	}

	bool SpriteDrawerD3D11::CreateBuffers()
	{
		HRESULT hr;

		// Initialize object buffer data
		ZeroMemory(&m_PerObjectBuffer, sizeof(m_PerObjectBuffer));
		m_PerObjectBuffer.Transform = XMMatrixIdentity();

		// Prepare initial data
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = &m_PerObjectBuffer;

		// Create per-object constant buffer
		D3D11_BUFFER_DESC cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = 64 + 16;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = g_D3DDevice11->CreateBuffer(&cbd, &initData, m_PerObjCBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create per-object constant buffer!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Create constant buffer
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = 64;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = g_D3DDevice11->CreateBuffer(&cbd, nullptr, m_ViewBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create constant buffer!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Create sprite geometry buffer
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
		bd.ByteWidth = sizeof(SpriteVertex2D) * 4;     // size is the SpriteVertex2D struct * 4
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
		hr = g_D3DDevice11->CreateBuffer(&bd, nullptr, m_SpriteGeomBuffer.GetAddressOf());       // create the buffer
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create sprite vertex buffer!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		return true;
	}

	bool SpriteDrawerD3D11::CreateRasterState()
	{
		// Create the blend state to enable alpha blending
		D3D11_RASTERIZER_DESC rasterDesc;
		ZeroMemory(&rasterDesc, sizeof(rasterDesc));
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		HRESULT hr = g_D3DDevice11->CreateRasterizerState(&rasterDesc, m_RasterState.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create rasterizer state!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		return true;
	}

	void SpriteDrawerD3D11::UpdateTransform(float x, float y, float Rotation)
	{
		if (!g_D3DDeviceContext11)
			return;

		m_PerObjectBuffer.Transform =
			XMMatrixRotationZ(Rotation) *
			XMMatrixTranslation(x, y, 0.0f);

		g_D3DDeviceContext11->UpdateSubresource(m_PerObjCBuffer.Get(), 0, nullptr, &m_PerObjectBuffer, 0, 0);
	}

	void SpriteDrawerD3D11::Dispatch()
	{
		UINT stride = sizeof(SpriteVertex2D);
		UINT offset = 0;
		g_D3DDeviceContext11->IASetVertexBuffers(0, 1, m_SpriteGeomBuffer.GetAddressOf(), &stride, &offset);

		// Draw the actual geometry
		g_D3DDeviceContext11->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		g_D3DDeviceContext11->Draw(4, 0);
	}
}
