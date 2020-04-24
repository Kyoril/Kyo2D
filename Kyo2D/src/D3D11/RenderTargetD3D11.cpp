
#include "RenderTargetD3D11.h"

namespace Kyo2D
{
	RenderTargetD3D11::RenderTargetD3D11()
		: m_Handle(nullptr)
		, m_Width(0)
		, m_Height(0)
		, m_Fullscreen(false)
		, m_VSync(false)
	{
	}

	RenderTargetD3D11::~RenderTargetD3D11()
	{
		if (m_SwapChain)
		{
			m_SwapChain->SetFullscreenState(FALSE, nullptr);
		}
	}

	bool RenderTargetD3D11::Initialize(HWND hwnd, std::uint16_t width, std::uint16_t height, bool fullscreen)
	{
		m_Handle = hwnd;
		m_Width = width;
		m_Height = height;

		if (!IsWindow(hwnd))
		{
			MessageBox(hwnd, L"Invalid window handle provided for render target creation!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		// Create the swap chain and the render target
		ComPtr<IDXGIDevice> dxgiDevice;
		HRESULT hr = g_D3DDevice11.As(&dxgiDevice);
		if (FAILED(hr))
		{
			MessageBox(hwnd, L"Could not get DXGI device!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(hwnd, L"Could not get DXGI adapter!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		ComPtr<IDXGIFactory> dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
		if (FAILED(hr))
		{
			MessageBox(hwnd, L"Could not get DXGI factory!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		DXGI_SWAP_CHAIN_DESC sd;
		memset(&sd, 0, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.SampleDesc.Count = 1;
		sd.Windowed = !fullscreen;
		hr = dxgiFactory->CreateSwapChain(g_D3DDevice11.Get(), &sd, m_SwapChain.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(hwnd, L"Could not create swap chain!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		hr = dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
		if (FAILED(hr))
		{
			MessageBox(hwnd, L"Could not disable Alt+Enter!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		ComPtr<ID3D11Texture2D> backBuffer;
		hr = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
		if (FAILED(hr))
		{
			MessageBox(hwnd, L"Could not get backbuffer!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		hr = g_D3DDevice11->CreateRenderTargetView(backBuffer.Get(), nullptr, m_RenderTargetView.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(hwnd, L"Could not create render target view!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		D3D11_TEXTURE2D_DESC texd;
		ZeroMemory(&texd, sizeof(texd));
		texd.Width = width;
		texd.Height = height;
		texd.ArraySize = 1;
		texd.MipLevels = 1;
		texd.SampleDesc.Count = 1;
		texd.Format = DXGI_FORMAT_D32_FLOAT;
		texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ComPtr<ID3D11Texture2D> depthBuffer;
		g_D3DDevice11->CreateTexture2D(&texd, nullptr, depthBuffer.GetAddressOf());

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
		ZeroMemory(&dsvd, sizeof(dsvd));
		dsvd.Format = DXGI_FORMAT_D32_FLOAT;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		g_D3DDevice11->CreateDepthStencilView(depthBuffer.Get(), &dsvd, m_DepthTargetView.GetAddressOf());

		// Set depth stencil state
		D3D11_DEPTH_STENCIL_DESC dsd;
		ZeroMemory(&dsd, sizeof(dsd));
		dsd.DepthEnable = TRUE;
		dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		g_D3DDevice11->CreateDepthStencilState(&dsd, m_DepthStencilState.GetAddressOf());
		g_D3DDeviceContext11->OMSetDepthStencilState(m_DepthStencilState.Get(), 0);

		// Create view matrix
		m_ViewMatrix = XMMatrixOrthographicOffCenterRH(0.0f, m_Width, m_Height, 0.0f, 0.0f, 1000.0f);

		return true;
	}

	void RenderTargetD3D11::Set()
	{
		if (!m_RenderTargetView)
		{
			return;
		}

		// Set active render target
		g_D3DDeviceContext11->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthTargetView.Get());

		// Setup viewport
		D3D11_VIEWPORT vp;
		ZeroMemory(&vp, sizeof(vp));
		vp.Width = static_cast<float>(m_Width);
		vp.Height = static_cast<float>(m_Height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		g_D3DDeviceContext11->RSSetViewports(1, &vp);
	}

	void RenderTargetD3D11::Clear(float R, float G, float B)
	{
		if (!m_RenderTargetView)
		{
			return;
		}

		const FLOAT ClearColor[4] = { R, G, B, 1.0f };
		g_D3DDeviceContext11->ClearRenderTargetView(m_RenderTargetView.Get(), ClearColor);

		// clear the depth buffer
		g_D3DDeviceContext11->ClearDepthStencilView(m_DepthTargetView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void RenderTargetD3D11::Present()
	{
		if (!m_SwapChain)
		{
			return;
		}

		m_SwapChain->Present(m_VSync ? 1 : 0, 0);
	}

	void RenderTargetD3D11::SetFullscreenState(bool Fullscreen)
	{
		if (m_SwapChain)
		{
			if (m_Fullscreen != Fullscreen)
			{
				m_Fullscreen = Fullscreen;
				m_SwapChain->SetFullscreenState(Fullscreen, nullptr);
			}
		}
	}

	void RenderTargetD3D11::SetVSyncEnabled(bool Enable)
	{
		m_VSync = Enable;
	}

	bool RenderTargetD3D11::Resize(std::uint16_t Width, std::uint16_t Height)
	{
		if (!m_SwapChain)
		{
			return false;
		}

		if (m_Width == Width && m_Height == Height)
		{
			return true;
		}

		// Apply new size values
		m_Width = Width;
		m_Height = Height;

		// Temporary turn off full screen
		if (m_Fullscreen)
		{
			m_SwapChain->SetFullscreenState(FALSE, nullptr);
		}

		// First, we need to destroy the old render target view
		m_DepthTargetView.Reset();
		m_RenderTargetView.Reset();

		// Next, we need to resize the swap chains back buffer
		HRESULT hr = m_SwapChain->ResizeBuffers(1, m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		if (FAILED(hr))
		{
			// Error!
			return false;
		}

		// Now recreate the render target view
		ComPtr<ID3D11Texture2D> backBuffer;
		hr = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		hr = g_D3DDevice11->CreateRenderTargetView(backBuffer.Get(), nullptr, m_RenderTargetView.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		D3D11_TEXTURE2D_DESC texd;
		ZeroMemory(&texd, sizeof(texd));
		texd.Width = m_Width;
		texd.Height = m_Height;
		texd.ArraySize = 1;
		texd.MipLevels = 1;
		texd.SampleDesc.Count = 1;
		texd.Format = DXGI_FORMAT_D32_FLOAT;
		texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ComPtr<ID3D11Texture2D> depthBuffer;
		g_D3DDevice11->CreateTexture2D(&texd, nullptr, depthBuffer.GetAddressOf());

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
		ZeroMemory(&dsvd, sizeof(dsvd));
		dsvd.Format = DXGI_FORMAT_D32_FLOAT;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		g_D3DDevice11->CreateDepthStencilView(depthBuffer.Get(), &dsvd, m_DepthTargetView.GetAddressOf());

		// Set depth stencil state
		g_D3DDeviceContext11->OMSetDepthStencilState(m_DepthStencilState.Get(), 0);

		// Create view matrix
		m_ViewMatrix = XMMatrixOrthographicOffCenterRH(0.0f, m_Width, m_Height, 0.0f, 0.0f, 1000.0f);

		// Re-enable full screen
		if (m_Fullscreen)
		{
			m_SwapChain->SetFullscreenState(TRUE, nullptr);
		}

		// Everything done!
		return true;
	}
}
