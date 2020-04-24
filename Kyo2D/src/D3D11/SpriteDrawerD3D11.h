
#pragma once

#include "../SpriteDrawer.h"
#include <d3d11.h>
#include <comptr.h>
#include "../Texture.h"
using namespace Microsoft::WRL;
using namespace DirectX;

extern ComPtr<ID3D11Device> g_D3DDevice11;
extern ComPtr<ID3D11DeviceContext> g_D3DDeviceContext11;

namespace Kyo2D
{
	/// Base class for sprite rendering.
	class SpriteDrawerD3D11 : public SpriteDrawer
	{
	public:

		/// Default constructor.
		SpriteDrawerD3D11();
		/// Destructor.
		virtual ~SpriteDrawerD3D11();

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
		virtual bool IsScale2XEnabled() const override { return m_Scale2XEnabled; }

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

		/// 
		bool CreateSpriteShaders();
		/// 
		bool CreateScale2XShaders();
		/// 
		bool CreateSampler();
		/// 
		bool CreateBlendState();
		/// 
		bool CreateBuffers();
		/// 
		bool CreateRasterState();
		/// Updates the transformation matrix of the sprite.
		/// @param x X coordinate of the upper left corner of the sprite in pixels.
		/// @param y Y coordinate of the upper left corner of the sprite in pixels.
		/// @param Rotation Sprite rotation in radians. Rotation is applied to the sprites center.
		void UpdateTransform(float x, float y, float Rotation);
		///
		void Dispatch();

	private:

		/// Structure of the per-sprite constant buffer which is sent to the graphics card for every new sprite rendering.
		struct cbPerObject
		{
			/// Transformation matrix used to rotate sprite and put it in the right place.
			XMMATRIX Transform;
			/// Target width of the sprite in pixels.
			FLOAT TargetWidth;
			/// Target height of the sprite in pixels.
			FLOAT TargetHeight;
			/// Width of the sprites texture in pixels (whole texture, not just subsprite width).
			/// Needed for Scale2X algorithm.
			FLOAT RawWidth;
			/// Height of the sprites texture in pixels (whole texture, not just subsprite width).
			/// Needed for Scale2X algorithm.
			FLOAT RawHeight;
		};

		/// Vertex structure used for rendering 2D sprites.
		struct SpriteVertex2D
		{
			FLOAT X, Y, Z;			// position
			std::uint32_t Color;	// color
			FLOAT U, V;				// texture coordinates
			std::uint32_t ColorKey;	// color key
		};

	private:

		ComPtr<ID3D11VertexShader> m_VertShaderSprite;
		ComPtr<ID3D11PixelShader> m_PixShaderSprite;
		ComPtr<ID3D11VertexShader> m_VertShaderSpriteScale2X;
		ComPtr<ID3D11PixelShader> m_PixShaderSpriteScale2X;
		ComPtr<ID3D11Buffer> m_SpriteGeomBuffer;
		ComPtr<ID3D11Buffer> m_PerObjCBuffer;
		ComPtr<ID3D11Buffer> m_ViewBuffer;
		ComPtr<ID3D11InputLayout> m_SpriteInputLayout;
		ComPtr<ID3D11InputLayout> m_SpriteScale2XInputLayout;
		ComPtr<ID3D11SamplerState> m_SpriteSampler;
		ComPtr<ID3D11BlendState> m_BlendState;
		ComPtr<ID3D11RasterizerState> m_RasterState;
		cbPerObject m_PerObjectBuffer;
		XMMATRIX m_ViewMatrix;
		bool m_Scale2XEnabled;
	};
}
