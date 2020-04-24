
#pragma once

#include <memory>
#include <string>

namespace Kyo2D
{
	/// Base class for a texture.
	class Texture : public std::enable_shared_from_this<Texture>
	{
	public:

		/// Default constructor.
		Texture();
		/// Destructor.
		virtual ~Texture();

		/// Initializes this texture by loading it from memory.
		virtual bool Initialize(const void *data, size_t dataSize) = 0;
		/// Initializes this texture by loading it from a file.
		virtual bool Initialize(const std::wstring &filename) = 0;
		/// Activates this texture as the current one.
		virtual bool Set() = 0;

		/// Gets the width of this texture in pixels.
		virtual std::int32_t GetWidth() const = 0;
		/// Gets the height of this texture in pixels.
		virtual std::int32_t GetHeight() const = 0;
	};
}
