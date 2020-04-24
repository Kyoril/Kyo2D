
#include "RenderTargetD3D9.h"

namespace Kyo2D
{
	RenderTargetD3D9::RenderTargetD3D9()
		: m_Handle(nullptr)
		, m_Width(0)
		, m_Height(0)
		, m_Fullscreen(false)
		, m_VSync(false)
		, m_PendingUpdate(false)
	{
	}

	RenderTargetD3D9::~RenderTargetD3D9()
	{
	}

	bool RenderTargetD3D9::Initialize(HWND hwnd, std::uint16_t width, std::uint16_t height, bool fullscreen)
	{
		m_Handle = hwnd;
		m_Width = width;
		m_Height = height;
		m_Fullscreen = fullscreen;

		if (!IsWindow(hwnd))
		{
			MessageBox(hwnd, L"Invalid window handle provided for render target creation!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		if (!RecreateSwapChain())
		{
			MessageBox(hwnd, L"Could not create swap chain!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		// Create view matrix
		m_ViewMatrix = XMMatrixOrthographicOffCenterLH(0.0f, m_Width, m_Height, 0.0f, 0.0f, 1.0f);

		return true;
	}

	void RenderTargetD3D9::Set()
	{
		if (!m_SwapChain)
		{
			MessageBox(nullptr, L"Swap chain not initialized", L"Error", MB_ICONERROR | MB_OK);
			return;
		}

		// Set active render target
		HRESULT hr = g_D3DDevice9->SetRenderTarget(0, m_BackBuffer.Get());
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Coult not set render target", L"Error", MB_ICONERROR | MB_OK);
		}

		// Set viewport
		D3DVIEWPORT9 vp;
		ZeroMemory(&vp, sizeof(vp));
		vp.Width = m_Width;
		vp.Height = m_Height;
		vp.MinZ = -1.0f;
		vp.MaxZ = 1.0f;
		hr = g_D3DDevice9->SetViewport(&vp);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Coult not set viewport", L"Error", MB_ICONERROR | MB_OK);
		}
	}

	void RenderTargetD3D9::Clear(float R, float G, float B)
	{
		if (!m_SwapChain)
		{
			return;
		}

		if (m_PendingUpdate)
		{
			m_PendingUpdate = false;
			if (!RecreateSwapChain())
			{
				MessageBox(m_Handle, L"Could not re-create swap chain!", L"Error", MB_ICONERROR | MB_OK);
				return;
			}

			Set();
		}

		const D3DCOLOR ClearColor = D3DCOLOR_XRGB(
			static_cast<DWORD>(R * 255.0f), 
			static_cast<DWORD>(G * 255.0f), 
			static_cast<DWORD>(B * 255.0f));
		HRESULT hr = g_D3DDevice9->Clear(0, nullptr, D3DCLEAR_TARGET, ClearColor, 1.0f, 0);
		if (FAILED(hr))
		{
			MessageBox(m_Handle, L"Clear failed", L"Error", MB_ICONERROR | MB_OK);
		}
		hr = g_D3DDevice9->BeginScene();
		if (FAILED(hr))
		{
			MessageBox(m_Handle, L"BeginScene failed", L"Error", MB_ICONERROR | MB_OK);
		}
	}

	void RenderTargetD3D9::Present()
	{
		if (!m_SwapChain)
		{
			return;
		}

		HRESULT hr = g_D3DDevice9->EndScene();
		if (FAILED(hr))
		{
			MessageBox(m_Handle, L"EndScene failed", L"Error", MB_ICONERROR | MB_OK);
		}
		hr = m_SwapChain->Present(nullptr, nullptr, nullptr, nullptr, 0);
		if (FAILED(hr))
		{
			MessageBox(m_Handle, L"Present failed", L"Error", MB_ICONERROR | MB_OK);
		}
	}

	void RenderTargetD3D9::SetFullscreenState(bool Fullscreen)
	{
		if (m_Fullscreen != Fullscreen)
		{
			m_Fullscreen = Fullscreen;
			m_PendingUpdate = true;
		}
	}

	void RenderTargetD3D9::SetVSyncEnabled(bool Enable)
	{
		if (m_VSync != Enable)
		{
			m_VSync = Enable;
			m_PendingUpdate = true;
		}
	}

	bool RenderTargetD3D9::Resize(std::uint16_t Width, std::uint16_t Height)
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

		m_PendingUpdate = true;

		// Create view matrix
		m_ViewMatrix = XMMatrixOrthographicOffCenterLH(0.0f, m_Width, m_Height, 0.0f, -1.0f, 1.0f);

		// Everything done!
		return true;
	}
	bool RenderTargetD3D9::RecreateSwapChain()
	{
		m_BackBuffer.Reset();
		m_SwapChain.Reset();

		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = !m_Fullscreen;
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.hDeviceWindow = m_Handle;
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferWidth = m_Width;
		d3dpp.BackBufferHeight = m_Height;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.PresentationInterval = (m_VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE);

		HRESULT hr = g_D3DDevice9->CreateAdditionalSwapChain(&d3dpp, m_SwapChain.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		hr = m_SwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, m_BackBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}
}
