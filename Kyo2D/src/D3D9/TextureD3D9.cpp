
#include "TextureD3D9.h"

namespace Kyo2D
{
	TextureD3D9::TextureD3D9()
		: m_Width(0)
		, m_Height(0)
	{
	}

	TextureD3D9::~TextureD3D9()
	{
	}

	bool TextureD3D9::Initialize(const void * data, size_t dataSize)
	{
		// Load image from file
		ILuint idImage;
		ilGenImages(1, &idImage);
		ilBindImage(idImage);
		ilLoadL(IL_TYPE_UNKNOWN, data, static_cast<ILuint>(dataSize));
		if (ilGetError() != IL_NO_ERROR)
		{
			return false;
		}

		return InitializeImpl(idImage);
	}

	bool TextureD3D9::Initialize(const std::wstring &filename)
	{
		// TODO: Make use of custom extension handlers

		// Load image from file
		ILuint idImage;
		ilGenImages(1, &idImage);
		ilBindImage(idImage);
		ilLoadImage(filename.c_str());
		if (ilGetError() != IL_NO_ERROR)
		{
			return false;
		}

		return InitializeImpl(idImage);
	}

	bool TextureD3D9::Set()
	{
		if (!m_Texture.Get())
		{
			return false;
		}

		HRESULT hr = g_D3DDevice9->SetTexture(0, m_Texture.Get());
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	bool TextureD3D9::InitializeImpl(ILuint &idImage)
	{
		// Save image informations
		m_Width = ilGetInteger(IL_IMAGE_WIDTH);
		m_Height = ilGetInteger(IL_IMAGE_HEIGHT);

		// Convert to RGBA byte data
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		unsigned char * pData = ilGetData();

		HRESULT hr = g_D3DDevice9->CreateTexture(
			m_Width,
			m_Height,
			1,
			0,
			D3DFMT_A8R8G8B8,
			D3DPOOL_MANAGED,
			m_Texture.GetAddressOf(),
			nullptr);
		if (SUCCEEDED(hr))
		{
			D3DLOCKED_RECT Rect;
			hr = m_Texture->LockRect(0, &Rect, nullptr, 0);
			if (SUCCEEDED(hr))
			{
				unsigned char *ptr = (unsigned char*)Rect.pBits;
				std::uint32_t *img = (std::uint32_t*)pData;
				for (std::int32_t i = 0; i < m_Width * m_Height * 4; i += 4)
				{
					*(ptr++) = (*img & 0x00FF0000) >> 16;		// B
					*(ptr++) = (*img & 0x0000FF00) >> 8;		// G
					*(ptr++) = (*img & 0x000000FF);				// R
					*(ptr++) = (*img & 0xFF000000) >> 24;		// A
					img++;
				}
			}

			// Unlock texture rect
			hr = m_Texture->UnlockRect(0);
		}

		// Memory cleanup
		ilDeleteImages(1, &idImage);
		idImage = 0;

		return SUCCEEDED(hr);
	}
}
