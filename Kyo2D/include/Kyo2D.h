
#pragma once

// Windows specific headers
#include <Windows.h>

// D3D11 specific headers
#include <d3d11.h>
#include <d3d9.h>
#include <DirectXMath.h>
#include <comptr.h>

// STL headers
#include <memory>
#include <map>

// C runtime headers
#include <cstdint>
#include <cassert>


// API Definition
#ifdef KYO2D_EXPORTS
#	ifdef __cplusplus
#		define K2D_API extern "C" __declspec(dllexport)
#	else
#		define K2D_API __declspec(dllexport)
#	endif
#else
#	ifdef __cplusplus
#		define K2D_API extern "C" __declspec(dllimport)
#	else
#		define K2D_API  __declspec(dllimport)
#	endif
#endif


struct K2D_Point
{
	float X, Y;
};

/// Vertex structure used for rendering 2D geometry using the helper functions (DrawPoint, DrawLine etc.)
struct Vertex2D
{
	FLOAT X, Y, Z;		// position
	std::int32_t Color;	// color
};


typedef void(*DecodeTexPtr)();


////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL ENGINE METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Initializes the Kyo2D engine.
K2D_API void K2D_Init(bool useD3D11);

/// Terminates the Kyo2D engine, destroying all objects which are still initialized.
K2D_API void K2D_Terminate();

/// Determines if Direct3D11 is supported.
K2D_API bool K2D_Direct3D11Supported();



////////////////////////////////////////////////////////////////////////////////////////////////////
// RENDER TARGET MANAGEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Creates a new render target.
K2D_API std::uint32_t K2D_CreateRenderTarget(HWND Handle, std::uint16_t Width, std::uint16_t Height, bool Fullscreen);

/// Destroys the given render target
K2D_API bool K2D_DestroyRenderTarget(std::uint32_t RenderTarget);

/// Activates a specific render target.
K2D_API bool K2D_SetRenderTarget(std::uint32_t RenderTarget);

/// Clears the active render target.
K2D_API bool K2D_ClearRenderTarget(float r, float g, float b);

/// Activates a specific render target.
K2D_API bool K2D_SetVSyncEnabled(bool Enable);

/// Presents the active render target.
K2D_API bool K2D_PresentRenderTarget();

/// Resizes the active render target.
K2D_API bool K2D_ResizeRenderTarget(std::uint16_t Width, std::uint16_t Height);



////////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE MANAGEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Creates a new texture from a given file.
K2D_API std::uint32_t K2D_CreateTexture(const wchar_t *Filename);

/// Creates a new texture from memory.
K2D_API std::uint32_t K2D_CreateTextureFromMemory(const char *data, std::uint32_t size);

/// Destroys a texture.
K2D_API bool K2D_DestroyTexture(std::uint32_t TextureId);

/// Returns a textures size. If the texture is invalid, the returned size will be 0.
K2D_API K2D_Point K2D_GetTextureSize(std::uint32_t TextureId);

/// Registers a new texture decoder for a specific extension.
K2D_API std::uint32_t K2D_RegisterTextureDecoder(const wchar_t *extension, DecodeTexPtr decoder);

/// Unregisters a texture decoder.
K2D_API bool K2D_UnregisterTextureDecoder(std::uint32_t decoderId);



////////////////////////////////////////////////////////////////////////////////////////////////////
// SPRITE RENDERING
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Enables or disables the Scale2X algorithm for sprite rendering. The algorithm is performed on
/// the GPU, and thus is super-performant. Changing this state has no immediate effect, so this method
/// can be called at any time, and results will affect all K2D_DrawSprite... methods after this call.
/// Per default, Scale2X is enabled.
/// @param enable true to enable the Scale2X algorithm, false to disable it.
K2D_API void K2D_SetScale2XEnabled(bool enable);

/// Draws a simple sprite at a given location.
K2D_API bool K2D_DrawSpriteAt(std::uint32_t TextureId, float X, float Y, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey);

/// Draws a subarea of a sprite at a given location.
K2D_API bool K2D_DrawSubspriteAt(std::uint32_t TextureId, float X, float Y, float srcX, float srcY, float srcW, float srcH, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey);

/// Draws a sprite and stretches it to the given area.
K2D_API bool K2D_DrawSpriteScaled(std::uint32_t TextureId, float X, float Y, float W, float H, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey);

/// Draws a subarea of a sprite and stretches it to the given area.
K2D_API bool K2D_DrawSubspriteScaled(std::uint32_t TextureId, float X, float Y, float W, float H, float srcX, float srcY, float srcW, float srcH, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey);

/// Draws a sprite and tiles it in the given area.
K2D_API bool K2D_DrawSpriteTiled(std::uint32_t TextureId, float X, float Y, float W, float H, float tX, float tY, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey);



////////////////////////////////////////////////////////////////////////////////////////////////////
// FONT MANAGEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Creates a font from a ttf file.
/// @param Filename The name of the font file.
/// @param PointSize The font size in points.
/// @param Outline The outline width in pixels.
/// @return The new font id or 0 if an error occurred.
K2D_API std::uint32_t K2D_CreateFont(const wchar_t* Filename, float PointSize, float Outline);

/// Creates a font from a ttf file.
/// @param Buffer The buffer which containst the file contents of a ttf file.
/// @param BufferSize Size of the buffer.
/// @param PointSize The font size in points.
/// @param Outline The outline width in pixels.
/// @return The new font id or 0 if an error occurred.
K2D_API std::uint32_t K2D_CreateFontFromMemory(const std::uint8_t* Buffer, std::uint32_t BufferSize, float PointSize, float Outline);

/// Destroys a created font.
/// @param FontId The id of the font to destroy.
/// @return false if the font couldn't be destroyed.
K2D_API bool K2D_DestroyFont(std::uint32_t FontId);

/// Draws a string at the given location using the given font.
/// @param FontId The id of the font to use.
/// @param Text The text to display.
/// @param X The x coordinate.
/// @param Y The y coordinate.
/// @param RGBA The text color.
/// @return false if the font couldn't be found or an error occurred.
K2D_API bool K2D_DrawText(std::uint32_t FontId, const wchar_t* Text, float X, float Y, std::uint32_t RGBA);


////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D DRAWING OPERATION HELPERS
////////////////////////////////////////////////////////////////////////////////////////////////////

/// Renders a single point at the given location.
K2D_API bool K2D_DrawPoint(float X, float Y, std::uint32_t RGBA);

/// Renders a rectangle border at the given location.
K2D_API bool K2D_DrawRect(float X, float Y, float Width, float Height, std::uint32_t RGBA);

/// Renders a rectangle at the given location.
K2D_API bool K2D_FillRect(float X, float Y, float Width, float Height, std::uint32_t RGBA);

/// Renders a line between two given points.
K2D_API bool K2D_DrawLine(float X1, float Y1, float X2, float Y2, std::uint32_t RGBA);
