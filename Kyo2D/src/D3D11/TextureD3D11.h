
#pragma once

#include <memory>
#include <string>
#include <d3d11.h>
#include <comptr.h>
#include "../Texture.h"
#include "IL/il.h"
using namespace Microsoft::WRL;

extern ComPtr<ID3D11Device> g_D3DDevice11;
extern ComPtr<ID3D11DeviceContext> g_D3DDeviceContext11;

namespace Kyo2D
{
	class TextureD3D11 : public Texture
	{
	public:

		/// Default constructor.
		TextureD3D11();
		/// Destructor
		virtual ~TextureD3D11();

		/// @copydoc Texture::Initialize(const std::wstring &)
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

		ComPtr<ID3D11Texture2D> m_Texture;
		ComPtr<ID3D11ShaderResourceView> m_ShaderResView;
		std::int32_t m_Width, m_Height;
	};
}
