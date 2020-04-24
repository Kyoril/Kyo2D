
#pragma once

#include <memory>
#include <string>
#include <d3d9.h>
#include <comptr.h>
#include "../Texture.h"
#include "IL/il.h"
using namespace Microsoft::WRL;

extern ComPtr<IDirect3D9> g_D3D9;
extern ComPtr<IDirect3DDevice9> g_D3DDevice9;

namespace Kyo2D
{
	/// Direct3D9 implementation of a texture.
	class TextureD3D9 : public Texture
	{
	public:

		/// Default constructor.
		TextureD3D9();
		/// Destructor
		virtual ~TextureD3D9();

		/// Initializes this texture by loading it from memory.
		virtual bool Initialize(const void *data, size_t dataSize) override;
		/// @copydoc Texture::Initialize(const std::wstring &)
		virtual bool Initialize(const std::wstring &filename) override;
		/// @copydoc Texture::Set()
		virtual bool Set() override;

		/// @copydoc Texture::GetWidth()
		virtual std::int32_t GetWidth() const override { return m_Width; }
		/// @copydoc Texture::GetHeight()
		virtual std::int32_t GetHeight() const override { return m_Height; }

	private:

		bool InitializeImpl(ILuint &idImage);

	private:

		ComPtr<IDirect3DTexture9> m_Texture;
		std::int32_t m_Width, m_Height;
	};
}
