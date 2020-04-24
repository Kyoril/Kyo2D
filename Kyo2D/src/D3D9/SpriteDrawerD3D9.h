
#pragma once

#include "../SpriteDrawer.h"
#include <Windows.h>
#include <comptr.h>
#include <d3d9.h>
using namespace Microsoft::WRL;

extern ComPtr<IDirect3D9> g_D3D9;
extern ComPtr<IDirect3DDevice9> g_D3DDevice9;

namespace Kyo2D
{
	/// Base class for sprite rendering.
	class SpriteDrawerD3D9 : public SpriteDrawer
	{
	public:

		/// Default constructor.
		SpriteDrawerD3D9();
		/// Destructor.
		virtual ~SpriteDrawerD3D9();

		/// @copydoc SpriteDrawer::Initialize()
		virtual bool Initialize() override;
		/// @copydoc SpriteDrawer::Prepare()
		virtual bool Prepare() override;
		/// @copydoc SpriteDrawer::SetViewMatrix(const XMMATRIX &)
		virtual void SetViewMatrix(const XMMATRIX &ViewMatrix) override;
		/// @copydoc SpriteDrawer::SetScale2XEnabled(bool)
		virtual void SetScale2XEnabled(bool Enable) override;
		
	public:

		/// @copydoc SpriteDrawer::IsScale2XEnabled()
		virtual bool IsScale2XEnabled() const override { return false; }

	public:

		/// Draws a simple sprite at a given location.
		virtual void DrawSpriteAt(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float Rotation, std::uint32_t color, std::uint32_t colorkey) override;
		/// Draws a subarea of a sprite at a given location.
		virtual void DrawSubspriteAt(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float srcX, float srcY, float srcW, float srcH, float Rotation, std::uint32_t color, std::uint32_t colorkey) override;
		/// Draws a sprite and stretches it to the given area.
		virtual void DrawSpriteScaled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float Rotation, std::uint32_t color, std::uint32_t colorkey) override;
		/// Draws a subarea of a sprite and stretches it to the given area.
		virtual void DrawSubspriteScaled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float srcX, float srcY, float srcW, float srcH, float Rotation, std::uint32_t color, std::uint32_t colorkey) override;
		/// Draws a sprite and tiles it in the given area.
		virtual void DrawSpriteTiled(std::int32_t texW, std::int32_t texH, float X, float Y, float Z, float W, float H, float tX, float tY, float Rotation, std::uint32_t color, std::uint32_t colorkey) override;

	private:

		/// Updates the transformation matrix of the sprite.
		/// @param x X coordinate of the upper left corner of the sprite in pixels.
		/// @param y Y coordinate of the upper left corner of the sprite in pixels.
		/// @param Rotation Sprite rotation in radians. Rotation is applied to the sprites center.
		void UpdateTransform(float x, float y, float Rotation);

	private:

		/// Vertex structure used for rendering 2D sprites.
		struct SpriteVertex2D
		{
			FLOAT X, Y, Z;			// position
			std::uint32_t Color;	// color
			FLOAT U, V;				// texture coordinates
			FLOAT A, R, G, B;		// color key
		};

	private:

		ComPtr<IDirect3DVertexShader9> m_VertShader;
		ComPtr<IDirect3DPixelShader9> m_PixShader;
		ComPtr<IDirect3DVertexBuffer9> m_GeomBuffer;
		ComPtr<IDirect3DVertexDeclaration9> m_VertexDecl;
		XMMATRIX m_Matrices[2];
	};
}
