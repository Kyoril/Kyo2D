
#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <memory>
using namespace DirectX;

namespace Kyo2D
{
	/// Base class of a render target.
	class RenderTarget : public std::enable_shared_from_this<RenderTarget>
	{
	public:

		/// Default constructor of the render target.
		RenderTarget();
		/// Destructor of the render target.
		virtual ~RenderTarget();

		/// Initializes the render target if it hasn't been initialized already.
		virtual bool Initialize(HWND hwnd, std::uint16_t width, std::uint16_t height, bool fullscreen) = 0;
		/// Sets this render target as the active render target.
		virtual void Set() = 0;
		/// Clears the render target using the given rgb color.
		virtual void Clear(float R, float G, float B) = 0;
		/// Swaps the render targets back buffer with its front buffer, presenting it on the screen.
		virtual void Present() = 0;
		/// Updates the render targets fullscreen state.
		virtual void SetFullscreenState(bool Fullscreen) = 0;
		/// Enables or disables vertical sync.
		virtual void SetVSyncEnabled(bool Enable) = 0;
		/// Resizes the render target to match the given size.
		virtual bool Resize(std::uint16_t Width, std::uint16_t Height) = 0;

		/// Determines if the render target has successfully been initialized.
		virtual bool IsInitialized() const = 0;
		/// Gets the render targets window handle.
		virtual HWND GetHandle() const = 0;
		/// Gets the render targets width in pixels.
		virtual std::uint16_t GetWidth() const = 0;
		/// Gets the render targets height in pixels.
		virtual std::uint16_t GetHeight() const = 0;
		/// Determines if the render target is a full screen render target.
		virtual bool IsFullscreen() const = 0;
		/// Determines if the vertical sync option is enabled.
		virtual bool IsVSyncEnabled() const = 0;
		/// Gets this render targets view matrix.
		virtual const XMMATRIX &GetViewMatrix() const = 0;
	};
}
