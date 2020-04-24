
#pragma once

#include "../RenderTarget.h"
#include <Windows.h>
#include <comptr.h>
#include <d3d9.h>
#include <DirectXMath.h>
#include <memory>
using namespace Microsoft::WRL;
using namespace DirectX;

extern ComPtr<IDirect3D9> g_D3D9;
extern ComPtr<IDirect3DDevice9> g_D3DDevice9;

namespace Kyo2D
{
	/// Direct3D9 implementation of a render target.
	class RenderTargetD3D9 : public RenderTarget
	{
	public:

		/// Default constructor.
		RenderTargetD3D9();
		/// Destructor.
		virtual ~RenderTargetD3D9();

		/// @copydoc RenderTarget::Initialize(HWND, std::uint16_t, std::uint16_t, bool)
		virtual bool Initialize(HWND hwnd, std::uint16_t width, std::uint16_t height, bool fullscreen) override;
		/// @copydoc RenderTarget::Set()
		virtual void Set() override;
		/// @copydoc RenderTarget::Clear(float, float, float)
		virtual void Clear(float R, float G, float B) override;
		/// @copydoc RenderTarget::Present()
		virtual void Present() override;
		/// @copydoc RenderTarget::SetFullscreenState(bool)
		virtual void SetFullscreenState(bool Fullscreen) override;
		/// @copydoc RenderTarget::SetVSyncEnabled(bool)
		virtual void SetVSyncEnabled(bool Enable) override;
		/// @copydoc RenderTarget::Resize(std::uint16_t, std::uint16_t)
		virtual bool Resize(std::uint16_t Width, std::uint16_t Height) override;

		/// Determines if the render target has successfully been initialized.
		virtual bool IsInitialized() const override { return m_SwapChain; }
		/// Gets the render targets window handle.
		virtual HWND GetHandle() const override { return m_Handle; }
		/// Gets the render targets width in pixels.
		virtual std::uint16_t GetWidth() const override { return m_Width; }
		/// Gets the render targets height in pixels.
		virtual std::uint16_t GetHeight() const override { return m_Height; }
		/// Determines if the render target is a full screen render target.
		virtual bool IsFullscreen() const override { return m_Fullscreen; }
		/// Determines if the vertical sync option is enabled.
		virtual bool IsVSyncEnabled() const override { return m_VSync; }
		/// Gets this render targets view matrix.
		virtual const XMMATRIX &GetViewMatrix() const override { return m_ViewMatrix; }

	private:

		bool RecreateSwapChain();

	private:

		HWND m_Handle;
		std::uint16_t m_Width, m_Height;
		bool m_Fullscreen;
		bool m_VSync;
		ComPtr<IDirect3DSwapChain9> m_SwapChain;
		ComPtr<IDirect3DSurface9> m_BackBuffer;
		XMMATRIX m_ViewMatrix;
		bool m_PendingUpdate;
	};
}
