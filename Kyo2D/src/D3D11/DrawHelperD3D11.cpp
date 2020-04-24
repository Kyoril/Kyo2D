
#include "DrawHelperD3D11.h"
#include "Kyo2D.h"
#include "shaders/d3d11/Draw2D11_VS.h"
#include "shaders/d3d11/Draw2D11_PS.h"

namespace Kyo2D
{
	DrawHelperD3D11::DrawHelperD3D11()
		: DrawHelper()
	{
	}

	DrawHelperD3D11::~DrawHelperD3D11()
	{
	}

	bool DrawHelperD3D11::Initialize()
	{
		// Load vertex shader
		HRESULT hr = g_D3DDevice11->CreateVertexShader(g_draw2D11MainVS, sizeof(g_draw2D11MainVS), nullptr, m_VertShader2D.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create vertex shader!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Create input layout
		D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		hr = g_D3DDevice11->CreateInputLayout(ied, 2, g_draw2D11MainVS, sizeof(g_draw2D11MainVS), m_2DInputLayout.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create input layout!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Load pixel shader
		hr = g_D3DDevice11->CreatePixelShader(g_draw2D11MainPS, sizeof(g_draw2D11MainPS), nullptr, m_PixShader2D.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create pixel shader!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Create constant buffer
		D3D11_BUFFER_DESC cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = 64;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = g_D3DDevice11->CreateBuffer(&cbd, nullptr, m_2DCBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create constant buffer!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

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
		hr = g_D3DDevice11->CreateBlendState(&blendDesc, m_2DBlendState.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create blend state!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		// Create buffer
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
		bd.ByteWidth = sizeof(Vertex2D) * 5;           // size is the Vertex2D struct * 5
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
		hr = g_D3DDevice11->CreateBuffer(&bd, nullptr, m_2DGeomBuffer.GetAddressOf());       // create the buffer
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not create vertex buffer!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		return true;
	}

	void DrawHelperD3D11::Prepare()
	{
		// Setup shader objects
		g_D3DDeviceContext11->VSSetShader(m_VertShader2D.Get(), 0, 0);
		g_D3DDeviceContext11->PSSetShader(m_PixShader2D.Get(), 0, 0);

		// Setup blend desc
		g_D3DDeviceContext11->OMSetBlendState(m_2DBlendState.Get(), 0, 0xFFFFFFFF);

		// Set constant buffer
		ID3D11Buffer *buffers[] = {
			m_2DCBuffer.Get()
		};
		g_D3DDeviceContext11->VSSetConstantBuffers(0, 1, buffers);

		// Set input layout
		g_D3DDeviceContext11->IASetInputLayout(m_2DInputLayout.Get());
	}

	void DrawHelperD3D11::DrawPoint(float X, float Y, std::int32_t Color)
	{
		// Generate vertices
		Vertex2D Vertex = { X, Y, 0.0f, Color };

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_2DGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, &Vertex, sizeof(Vertex));															// copy the data
			g_D3DDeviceContext11->Unmap(m_2DGeomBuffer.Get(), 0);												// unmap the buffer
		}

		// Draw command
		Dispatch(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, 1);
	}

	void DrawHelperD3D11::DrawLine(float X1, float Y1, float X2, float Y2, std::int32_t Color)
	{
		// Generate vertices
		Vertex2D Vertices[] =
		{
			{ X1, Y1, 0.0f, Color },
			{ X2, Y2, 0.0f, Color }
		};

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_2DGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, Vertices, sizeof(Vertices));														// copy the data
			g_D3DDeviceContext11->Unmap(m_2DGeomBuffer.Get(), 0);												// unmap the buffer
		}

		// Draw command
		Dispatch(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, 2);
	}

	void DrawHelperD3D11::DrawRect(float X, float Y, float W, float H, std::int32_t Color)
	{
		// Generate vertices
		Vertex2D Vertices[] =
		{
			{ X + 1, Y + H, 0.0f, Color },
			{ X + 1, Y + 1,	0.0f, Color },
			{ X + W, Y + 1, 0.0f, Color },
			{ X + W, Y + H, 0.0f, Color },
			{ X + 1, Y + H, 0.0f, Color },
		};

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_2DGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, Vertices, sizeof(Vertices));														// copy the data
			g_D3DDeviceContext11->Unmap(m_2DGeomBuffer.Get(), 0);												// unmap the buffer
		}

		// Draw command
		Dispatch(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, 5);
	}

	void DrawHelperD3D11::FillRect(float X, float Y, float W, float H, std::int32_t Color)
	{
		// Generate vertices
		Vertex2D Vertices[] =
		{
			// 2--4
			// | /|
			// |/ |
			// 1--3
			{ X, Y + H,		0.0f, Color },
			{ X, Y,			0.0f, Color },
			{ X + W, Y + H, 0.0f, Color },
			{ X + W, Y,		0.0f, Color }
		};

		// Update vertex buffer
		D3D11_MAPPED_SUBRESOURCE ms;
		{
			HRESULT hr = g_D3DDeviceContext11->Map(m_2DGeomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);   // map the buffer
			if (FAILED(hr))
			{
				return;
			}
			memcpy(ms.pData, Vertices, sizeof(Vertices));														// copy the data
			g_D3DDeviceContext11->Unmap(m_2DGeomBuffer.Get(), 0);												// unmap the buffer
		}

		// Draw command
		Dispatch(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4);
	}

	void DrawHelperD3D11::UpdateViewMatrix(const XMMATRIX & ViewMatrix)
	{
		g_D3DDeviceContext11->UpdateSubresource(m_2DCBuffer.Get(), 0, 0, &ViewMatrix, 0, 0);
	}

	void DrawHelperD3D11::Dispatch(D3D11_PRIMITIVE_TOPOLOGY Topology, UINT Count)
	{
		if (!g_D3DDeviceContext11)
			return;

		// Render point
		UINT stride = 0;
		UINT offset = 0;
		stride = sizeof(Vertex2D);
		g_D3DDeviceContext11->IASetVertexBuffers(0, 1, m_2DGeomBuffer.GetAddressOf(), &stride, &offset);

		// Draw the actual geometry
		g_D3DDeviceContext11->IASetPrimitiveTopology(Topology);
		g_D3DDeviceContext11->Draw(Count, 0);
	}
}
