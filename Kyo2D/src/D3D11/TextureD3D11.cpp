
#include "TextureD3D11.h"

namespace Kyo2D
{
	TextureD3D11::TextureD3D11()
		: m_Width(0)
		, m_Height(0)
	{
	}

	TextureD3D11::~TextureD3D11()
	{
	}

	bool TextureD3D11::Initialize(const void * data, size_t dataSize)
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

	bool TextureD3D11::Initialize(const std::wstring &filename)
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

	bool TextureD3D11::Set()
	{
		if (!m_ShaderResView)
		{
			return false;
		}

		g_D3DDeviceContext11->PSSetShaderResources(0, 1, m_ShaderResView.GetAddressOf());
		return true;
	}

	bool TextureD3D11::InitializeImpl(ILuint &idImage)
	{
		// Save image informations
		m_Width = ilGetInteger(IL_IMAGE_WIDTH);
		m_Height = ilGetInteger(IL_IMAGE_HEIGHT);

		// Convert to RGBA byte data
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		unsigned char * pData = ilGetData();

		// Setup texture description
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(td));
		td.Width = m_Width;
		td.Height = m_Height;
		td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		td.CPUAccessFlags = 0;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		//td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		// Fill in converted texture data
		D3D11_SUBRESOURCE_DATA data;
		memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = pData;
		data.SysMemPitch = 4 * m_Width;

		// Create texture
		HRESULT hr = g_D3DDevice11->CreateTexture2D(&td, &data, m_Texture.GetAddressOf());
		if (FAILED(hr))
		{
			// Could not load texture
			return false;
		}

		// Memory cleanup
		ilDeleteImages(1, &idImage);
		idImage = 0;

		// Create shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC svd;
		ZeroMemory(&svd, sizeof(svd));
		svd.Format = td.Format;
		svd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		svd.Texture2D.MipLevels = -1;
		hr = g_D3DDevice11->CreateShaderResourceView(m_Texture.Get(), &svd, m_ShaderResView.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}
}
