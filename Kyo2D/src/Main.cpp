#include "Kyo2D.h"
#include "D3D11/RenderTargetD3D11.h"
#include "D3D11/TextureD3D11.h"
#include "D3D11/DrawHelperD3D11.h"
#include "D3D11/SpriteDrawerD3D11.h"
#include "D3D11/TextDrawerD3D11.h"
#include "D3D9/RenderTargetD3D9.h"
#include "D3D9/TextureD3D9.h"
#include "D3D9/DrawHelperD3D9.h"
#include "D3D9/SpriteDrawerD3D9.h"
#include "D3D9/TextDrawerD3D9.h"
#include "Font.h"
#include <vector>
#include <string>
#include "IL/il.h"
using namespace Microsoft::WRL;
using namespace DirectX;



////////////////////////////////////////////////////////////////////////////////////////////////////
// Link libraries
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d9.lib")



////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D11 GLOBALS
////////////////////////////////////////////////////////////////////////////////////////////////////

ComPtr<ID3D11Device> g_D3DDevice11;
ComPtr<ID3D11DeviceContext> g_D3DDeviceContext11;



////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D9 GLOBALS
////////////////////////////////////////////////////////////////////////////////////////////////////

ComPtr<IDirect3D9> g_D3D9;
ComPtr<IDirect3DDevice9> g_D3DDevice9;
HWND g_D3D9Wnd = nullptr;



////////////////////////////////////////////////////////////////////////////////////////////////////
// SWITCH
////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_HasD3D11 = true;
bool g_UseD3D11 = true;



////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDER TARGET MANAGEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////

std::uint32_t g_NextRenderTarget = 1;
std::map<std::uint32_t, std::shared_ptr<Kyo2D::RenderTarget>> g_RenderTargets;
std::weak_ptr<Kyo2D::RenderTarget> g_ActiveRenderTarget;



////////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE MANAGEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////

std::uint32_t g_NextTexture = 1;
std::map<std::uint32_t, std::shared_ptr<Kyo2D::Texture>> g_Textures;



////////////////////////////////////////////////////////////////////////////////////////////////////
// FONT MANAGEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////

std::uint32_t g_NextFont = 1;
std::map<std::uint32_t, std::shared_ptr<Kyo2D::Font>> g_Fonts;



////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDER STAGE
////////////////////////////////////////////////////////////////////////////////////////////////////

// Draw helper
std::shared_ptr<Kyo2D::DrawHelper> g_DrawHelper;
std::shared_ptr<Kyo2D::SpriteDrawer> g_SpriteDrawer;

// Render stage enumeration: Used to reduce d3d11 state changes to a minimum.
namespace render_stage
{
	enum Type
	{
		None			= 0,
		Drawer2D		= 1,
		Sprite			= 2,
		SpriteScale2X	= 3
	};
}

// Shortcut typedef
typedef render_stage::Type RenderStage;

// Caches the current render stage
RenderStage g_RenderStage = render_stage::None;



////////////////////////////////////////////////////////////////////////////////////////////////////
/// INTERNAL HELPER METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
	/// Prepares the render state according to the required stage.
	/// This method tries to change the render state as less as possible.
	/// @param stage The render stage to prepare. This decides, what shaders and resource will be bound.
	static void PrepareStage(RenderStage stage)
	{
		if (g_RenderStage == stage)
			return;

		switch (stage)
		{
			case render_stage::Drawer2D:
			{
				// Prepare the helper
				g_DrawHelper->Prepare();
				break;
			}
			case render_stage::Sprite:
			case render_stage::SpriteScale2X:
			{
				g_SpriteDrawer->Prepare();
				break;
			}
		}

		// Apply new stage
		g_RenderStage = stage;
	}

	/// Binds the requested texture (by id) to the shader stage for rendering. Also calls PrepareStage
	/// if the texture was found, so it doesn't need to be called manually.
	/// @param texture The texture id to use.
	/// @param outW Used to return the textures width in pixels.
	/// @param outH Used to return the textures height in pixels.
	/// @returns true on success, false otherwise.
	static bool BindTextureStage(std::uint32_t texture, std::int32_t &outW, std::int32_t &outH)
	{
		if (!g_SpriteDrawer)
			return false;

		// Try to find the given texture
		auto it = g_Textures.find(texture);
		if (it == g_Textures.end())
		{
			return false;
		}

		outW = it->second->GetWidth();
		outH = it->second->GetHeight();

		// Change stage
		PrepareStage(g_SpriteDrawer->IsScale2XEnabled() ? render_stage::SpriteScale2X : render_stage::Sprite);

		return it->second->Set();
	}

	/// 
	static bool CreateD3D11Device()
	{
		// Already initialized?
		if (g_D3DDevice11.Get() || g_D3DDeviceContext11.Get())
		{
			return true;
		}

		// Device creation flags
		UINT flags = 0;
#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		// Create the D3D11 device
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			g_D3DDevice11.GetAddressOf(),
			nullptr,
			g_D3DDeviceContext11.GetAddressOf()
		);
		if (FAILED(hr))
		{
			MessageBox(nullptr, L"Could not initialize Direct3D 11 device!", L"Error", MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return false;
		}

		return true;
	}

	/// 
	static bool CreateD3D9Device()
	{
		if (g_D3D9)
			return true;

		// Setup D3D9
		g_D3D9 = Direct3DCreate9(D3D_SDK_VERSION);
		if (!g_D3D9)
		{
			return false;
		}

		// Setup device
		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = &DefWindowProc;
		wc.lpszClassName = L"d3d9_test_wc";
		if (!RegisterClass(&wc))
			return false;

		// Create an empty test window
		g_D3D9Wnd = CreateWindow(L"d3d9_test_wc", L"", 0, 0, 0, 0, 0, 0, 0, 0, 0);
		if (!g_D3D9Wnd)
			return false;

		// Create device
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.hDeviceWindow = g_D3D9Wnd;
		d3dpp.Flags = 0;
		HRESULT hr = g_D3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_D3D9Wnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, g_D3DDevice9.GetAddressOf());
		if (FAILED(hr))
		{
			MessageBox(g_D3D9Wnd, L"Could not create Direct3D9 device!", L"Error", MB_ICONERROR | MB_OK);
			return false;
		}

		// Turn off culling, lighting and zbuffer
		g_D3DDevice9->SetRenderState(D3DRS_LIGHTING, 0);
		g_D3DDevice9->SetRenderState(D3DRS_ZENABLE, 0);
		g_D3DDevice9->SetRenderState(D3DRS_ZWRITEENABLE, 0);
		g_D3DDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		// Enable alpha blending
		g_D3DDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		g_D3DDevice9->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		g_D3DDevice9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		g_D3DDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		g_D3DDevice9->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
		g_D3DDevice9->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		g_D3DDevice9->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);

		g_D3DDevice9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		g_D3DDevice9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		g_D3DDevice9->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

		return true;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL ENGINE METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////

K2D_API void K2D_Init(bool useD3D11)
{
	g_UseD3D11 = (useD3D11 && g_HasD3D11);

	// Initialize DevIL
	ilInit();

	// Create D3D11 pipeline
	if (g_UseD3D11)
	{
		if (!CreateD3D11Device())
			return;
		
		// Setup the draw helper
		g_DrawHelper = std::make_shared<Kyo2D::DrawHelperD3D11>();
		if (!g_DrawHelper || !g_DrawHelper->Initialize())
			return;

		// Setup the sprite drawer
		g_SpriteDrawer = std::make_shared<Kyo2D::SpriteDrawerD3D11>();
		if (!g_SpriteDrawer || !g_SpriteDrawer->Initialize())
			return;
	}
	else
	{
		// Create D3D9 fallback pipeline
		if (!CreateD3D9Device())
			return;

		// Setup the draw helper
		g_DrawHelper = std::make_shared<Kyo2D::DrawHelperD3D9>();
		if (!g_DrawHelper || !g_DrawHelper->Initialize())
			return;

		// Setup the sprite drawer
		g_SpriteDrawer = std::make_shared<Kyo2D::SpriteDrawerD3D9>();
		if (!g_SpriteDrawer || !g_SpriteDrawer->Initialize())
			return;
	}
}

K2D_API void K2D_Terminate()
{
	// Kill sprites
	g_Textures.clear();
	g_NextTexture = 1;

	// Kill render targets
	g_RenderTargets.clear();
	g_NextRenderTarget = 1;

	// Kill sprite drawer
	g_SpriteDrawer.reset();

	// Kill draw helper
	g_DrawHelper.reset();

	// Kill D3D11 API
	if (g_UseD3D11)
	{
		g_D3DDeviceContext11.Reset();
		g_D3DDevice11.Reset();
	}
	else
	{
		// TODO: Kill D3D9 API
		g_D3DDevice9.Reset();
		g_D3D9.Reset();

		if (g_D3D9Wnd)
		{
			DestroyWindow(g_D3D9Wnd);
			g_D3D9Wnd = nullptr;
		}
	}

	// Shutdown image loader library
	ilShutDown();
}

K2D_API bool K2D_Direct3D11Supported()
{
	OSVERSIONINFO version;
	ZeroMemory(&version, sizeof(OSVERSIONINFO));
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&version);
	
	bool isWinXPOrEarlier = (version.dwMajorVersion < 6);
	if (isWinXPOrEarlier)
		return false;

	return g_HasD3D11;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDER TARGET MANAGEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////

K2D_API std::uint32_t K2D_CreateRenderTarget(HWND Handle, std::uint16_t Width, std::uint16_t Height, bool Fullscreen)
{
	// Create render target instance and try to initialize it
	std::shared_ptr<Kyo2D::RenderTarget> renderTarget;
	if (g_UseD3D11)
		renderTarget = std::make_shared<Kyo2D::RenderTargetD3D11>();
	else
		renderTarget = std::make_shared<Kyo2D::RenderTargetD3D9>();

	// Check if render target was created
	if (!renderTarget.get())
		return 0;

	// Initialize the render target
	if (!renderTarget->Initialize(Handle, Width, Height, Fullscreen))
		return 0;

	// Store render target for later use
	std::uint32_t renderTargetIndex = g_NextRenderTarget++;
	g_RenderTargets[renderTargetIndex] = std::move(renderTarget);

	return renderTargetIndex;
}

K2D_API bool K2D_DestroyRenderTarget(std::uint32_t RenderTarget)
{
	auto it = g_RenderTargets.find(RenderTarget);
	if (it != g_RenderTargets.end())
	{
		// This will destroy the render target
		it = g_RenderTargets.erase(it);
		return true;
	}
	
	return false;
}

K2D_API bool K2D_SetRenderTarget(std::uint32_t RenderTarget)
{
	auto it = g_RenderTargets.find(RenderTarget);
	if (it == g_RenderTargets.end())
	{
		return false;
	}

	g_ActiveRenderTarget = it->second;
	it->second->Set();

	// Update view matrix for draw helper
	if (g_DrawHelper)
		g_DrawHelper->UpdateViewMatrix(it->second->GetViewMatrix());

	// Same for sprite drawer
	if (g_SpriteDrawer)
		g_SpriteDrawer->SetViewMatrix(it->second->GetViewMatrix());

	return true;
}

K2D_API bool K2D_ClearRenderTarget(float r, float g, float b)
{
	auto rt = g_ActiveRenderTarget.lock();
	if (!rt)
	{
		return false;
	}

	rt->Clear(r, g, b);
	return true;
}

K2D_API bool K2D_SetVSyncEnabled(bool Enable)
{
	auto rt = g_ActiveRenderTarget.lock();
	if (!rt)
	{
		return false;
	}

	rt->SetVSyncEnabled(Enable);
	return true;
}

K2D_API bool K2D_PresentRenderTarget()
{
	auto rt = g_ActiveRenderTarget.lock();
	if (!rt)
	{
		return false;
	}

	rt->Present();
	return true;
}

K2D_API bool K2D_ResizeRenderTarget(std::uint16_t Width, std::uint16_t Height)
{
	auto rt = g_ActiveRenderTarget.lock();
	if (!rt)
	{
		return false;
	}

	if (!rt->Resize(Width, Height))
	{
		return false;
	}

	rt->Set();

	// Update view matrix for draw helper
	if (g_DrawHelper)
		g_DrawHelper->UpdateViewMatrix(rt->GetViewMatrix());

	// Same for sprite drawer
	if (g_SpriteDrawer)
		g_SpriteDrawer->SetViewMatrix(rt->GetViewMatrix());
	
	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE MANAGEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////

K2D_API std::uint32_t K2D_CreateTexture(const wchar_t *Filename)
{
	// Filename valid?
	if (!Filename)
	{
		return 0;
	}

	// Create texture
	std::shared_ptr<Kyo2D::Texture> texture;
	if (g_UseD3D11)
		texture = std::make_shared<Kyo2D::TextureD3D11>();
	else
		texture = std::make_shared<Kyo2D::TextureD3D9>();

	// Check if creation was successful
	if (!texture.get())
	{
		return 0;
	}

	// Initialize (load) texture
	if (!texture->Initialize(Filename))
	{
		return 0;
	}

	// Save sprite
	std::uint32_t textureId = g_NextTexture++;
	g_Textures[textureId] = std::move(texture);

	return textureId;
}

K2D_API std::uint32_t K2D_CreateTextureFromMemory(const char *data, std::uint32_t size)
{
	// Validate data
	if (!data || size == 0)
	{
		return 0;
	}

	// Create texture
	std::shared_ptr<Kyo2D::Texture> texture;
	if (g_UseD3D11)
		texture = std::make_shared<Kyo2D::TextureD3D11>();
	else
		texture = std::make_shared<Kyo2D::TextureD3D9>();

	// Check if creation was successful
	if (!texture.get())
	{
		return 0;
	}

	// Initialize (load) texture
	if (!texture->Initialize(data, size))
	{
		return 0;
	}

	// Save sprite
	std::uint32_t textureId = g_NextTexture++;
	g_Textures[textureId] = std::move(texture);

	return textureId;
}

K2D_API bool K2D_DestroyTexture(std::uint32_t TextureId)
{
	auto it = g_Textures.find(TextureId);
	if (it == g_Textures.end())
	{
		return false;
	}

	it = g_Textures.erase(it);
	return true;
}

K2D_API K2D_Point K2D_GetTextureSize(std::uint32_t TextureId)
{
	K2D_Point point;
	
	auto it = g_Textures.find(TextureId);
	if (it != g_Textures.end())
	{
		point.X = static_cast<float>(it->second->GetWidth());
		point.Y = static_cast<float>(it->second->GetHeight());
	}
	else
	{
		point.X = 0.0f;
		point.Y = 0.0f;
	}

	return point;
}

K2D_API std::uint32_t K2D_RegisterTextureDecoder(const wchar_t *extension, DecodeTexPtr decoder)
{
	// TODO

	return 0;
}

K2D_API bool K2D_UnregisterTextureDecoder(std::uint32_t decoderId)
{
	// TODO

	return false;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// SPRITE RENDERING
////////////////////////////////////////////////////////////////////////////////////////////////////

K2D_API void K2D_SetScale2XEnabled(bool enable)
{
	if (g_SpriteDrawer)
		g_SpriteDrawer->SetScale2XEnabled(enable);
}

K2D_API bool K2D_DrawSpriteAt(std::uint32_t TextureId, float X, float Y, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey)
{
	// Try to find the given texture
	std::int32_t w = 0, h = 0;
	if (!BindTextureStage(TextureId, w, h))
		return false;

	g_SpriteDrawer->DrawSpriteAt(w, h, X, Y, Z, Rotation, color, colorkey);
	return true;
}

K2D_API bool K2D_DrawSubspriteAt(std::uint32_t TextureId, float X, float Y, float srcX, float srcY, float srcW, float srcH, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey)
{
	// Try to find the given texture
	std::int32_t w = 0, h = 0;
	if (!BindTextureStage(TextureId, w, h))
		return false;

	g_SpriteDrawer->DrawSubspriteAt(w, h, X, Y, Z, srcX, srcY, srcW, srcH, Rotation, color, colorkey);
	return true;
}

K2D_API bool K2D_DrawSpriteScaled(std::uint32_t TextureId, float X, float Y, float W, float H, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey)
{
	// Try to find the given texture
	std::int32_t w = 0, h = 0;
	if (!BindTextureStage(TextureId, w, h))
		return false;

	g_SpriteDrawer->DrawSpriteScaled(w, h, X, Y, Z, W, H, Rotation, color, colorkey);
	return true;
}

K2D_API bool K2D_DrawSubspriteScaled(std::uint32_t TextureId, float X, float Y, float W, float H, float srcX, float srcY, float srcW, float srcH, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey)
{
	// Try to find the given texture
	std::int32_t w = 0, h = 0;
	if (!BindTextureStage(TextureId, w, h))
		return false;

	g_SpriteDrawer->DrawSubspriteScaled(w, h, X, Y, Z, W, H, srcX, srcY, srcW, srcH, Rotation, color, colorkey);
	return true;
}

K2D_API bool K2D_DrawSpriteTiled(std::uint32_t TextureId, float X, float Y, float W, float H, float tX, float tY, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey)
{
	// Try to find the given texture
	std::int32_t w = 0, h = 0;
	if (!BindTextureStage(TextureId, w, h))
		return false;

	g_SpriteDrawer->DrawSpriteTiled(w, h, X, Y, Z, W, H, tX, tY, Rotation, color, colorkey);
	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// FONT METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////

K2D_API std::uint32_t K2D_CreateFont(const wchar_t * Filename, float PointSize, float Outline)
{
	// Filename valid?
	if (!Filename)
		return 0;

	// Create texture
	auto font = std::make_shared<Kyo2D::Font>();
	if (!font)
	{
		return 0;
	}

	// Initialize the font
	if (!font->Initialize(std::wstring(Filename), PointSize, Outline))
	{
		return 0;
	}

	// Save sprite
	std::uint32_t fontId = g_NextFont++;
	g_Fonts[fontId] = std::move(font);

	return fontId;
}

K2D_API std::uint32_t K2D_CreateFontFromMemory(const std::uint8_t* Buffer, std::uint32_t BufferSize, float PointSize, float Outline)
{
	if (!Buffer)
		return 0;

	if (BufferSize == 0)
		return 0;

	// Create texture
	auto font = std::make_shared<Kyo2D::Font>();
	if (!font)
	{
		return 0;
	}

	// Initialize the font
	if (!font->Initialize(Buffer, BufferSize, PointSize, Outline))
	{
		return 0;
	}

	// Save sprite
	std::uint32_t fontId = g_NextFont++;
	g_Fonts[fontId] = std::move(font);

	return fontId;
}

K2D_API bool K2D_DestroyFont(std::uint32_t FontId)
{
	auto it = g_Fonts.find(FontId);
	if (it == g_Fonts.end())
	{
		return false;
	}

	it = g_Fonts.erase(it);
	return true;
}

K2D_API bool K2D_DrawText(std::uint32_t FontId, const wchar_t * Text, float X, float Y, std::uint32_t RGBA)
{
	auto it = g_Fonts.find(FontId);
	if (it == g_Fonts.end())
	{
		return false;
	}

	// Draw text
	it->second->drawText(Text, Kyo2D::Vector2(X, Y), 1.0f);

	return false;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D DRAWING OPERATION HELPERS
////////////////////////////////////////////////////////////////////////////////////////////////////

K2D_API bool K2D_DrawPoint(float X, float Y, std::uint32_t RGBA)
{
	PrepareStage(render_stage::Drawer2D);
	g_DrawHelper->DrawPoint(X, Y, RGBA);

	return true;
}

K2D_API bool K2D_DrawRect(float X, float Y, float Width, float Height, std::uint32_t RGBA)
{
	PrepareStage(render_stage::Drawer2D);
	g_DrawHelper->DrawRect(X, Y, Width, Height, RGBA);

	return true;
}

K2D_API bool K2D_FillRect(float X, float Y, float Width, float Height, std::uint32_t RGBA)
{
	PrepareStage(render_stage::Drawer2D);
	g_DrawHelper->FillRect(X, Y, Width, Height, RGBA);

	return true;
}

K2D_API bool K2D_DrawLine(float X1, float Y1, float X2, float Y2, std::uint32_t RGBA)
{
	PrepareStage(render_stage::Drawer2D);
	g_DrawHelper->DrawLine(X1, Y1, X2, Y2, RGBA);

	return true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// DLL ENTRY POINT
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Procedural entry point of the dll.
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	// Handle different reasons
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// Determine if Direct3D 11 is available on the system
			if(!LoadLibrary(L"d3d11.dll"))
			{
				g_HasD3D11 = false;
				g_UseD3D11 = false;
			}
			//g_UseD3D11 = false;
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}

