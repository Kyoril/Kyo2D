#include "Font.h"
#include FT_STROKER_H
#define NOMINMAX
#include "Kyo2D.h"
#include <atomic>
#include <algorithm>

//=============================================================================
// Constants

/// Amount of pixels to put between two glyphs
static constexpr std::uint32_t INTER_GLYPH_PAD_SPACE = 4;
/// A multiplication coefficient to convert FT_Pos values into normal floats
static constexpr float FT_POS_COEF = (1.0f / 64.0f);
/// Number of glyphs per page. Must be a power of two.
static constexpr std::uint32_t GLYPHS_PER_PAGE = 256;
/// Amount of bits in a uint32.
static constexpr std::uint32_t BITS_PER_UINT = sizeof(std::uint32_t) * 8;


//=============================================================================
// Globals

/// FreeType library handle.
static FT_Library s_freeTypeLib;
/// Usage counter of free type library handle.
static std::atomic<std::int32_t> s_freeTypeUsageCount = 0;


namespace Kyo2D
{
	Font::Font()
		: m_pointSize(0.0f)
		, m_ascender(0)
		, m_descender(0)
		, m_height(0)
		, m_maxCodepoint(0)
		, m_outlineWidth(0.0f)
	{
		// Initializes the free type library if not already done and also increase
		// the reference counter.
		if (!s_freeTypeUsageCount++)
			FT_Init_FreeType(&s_freeTypeLib);
	}

	Font::~Font()
	{
		// Unload font face now before we eventually dispose the freetype library
		m_fontFace.reset();

		// Release freetype library handle if all using fonts are deleted
		if (!--s_freeTypeUsageCount)
			FT_Done_FreeType(s_freeTypeLib);
	}

	bool Font::Initialize(const void * data, size_t dataSize, float pointSize, float outline)
	{
		// We need to keep the memory as long as we want to be able to rasterize
		// font glyphs, so we will copy the memory from the source area
		m_fileData.resize(dataSize, 0);
		memcpy(&m_fileData[0], data, dataSize);

		// Apply point size
		m_pointSize = pointSize;
		m_outlineWidth = outline;

		// Negative outline size is not supported!
		if (m_outlineWidth < 0.0f) m_outlineWidth = 0.0f;

		// Now initialize the font
		return initializeInternal();
	}

	bool Font::Initialize(const std::wstring & filename, float pointSize, float outline)
	{
		// Read file contents
		if (!readFileContents(filename, m_fileData))
			return false;

		// Apply point size
		m_pointSize = pointSize;
		m_outlineWidth = outline;

		// Negative outline size is not supported!
		if (m_outlineWidth < 0.0f) m_outlineWidth = 0.0f;

		// Now initialize the font
		return initializeInternal();
	}

	bool Font::initializeInternal()
	{
		// Save error code
		FT_Error error;

		// Initialize memory face
		FT_Face tmpFace = nullptr;
		if ((error = FT_New_Memory_Face(s_freeTypeLib, m_fileData.data(), static_cast<FT_Long>(m_fileData.size()), 0, &tmpFace)) != 0)
			return false;

		// Initialize smart pointer to automatically free the font if needed
		m_fontFace.reset(tmpFace);

		// Check that the default unicode character map is available
		if (!m_fontFace->charmap)
			return false;

		// Initialize the character size
		const std::int32_t dpi = 96;
		const float pointSize64 = m_pointSize * 64.0f;
		if (FT_Set_Char_Size(m_fontFace.get(), FT_F26Dot6(pointSize64), FT_F26Dot6(pointSize64), dpi, dpi))
		{
			// For bitmap fonts we can render only at specific point sizes.
			// Try to find nearest point size and use it, if that is possible
			const float pointSize72 = (m_pointSize * 72.0f) / dpi;
			float bestDelta = 99999.0f;
			float bestSize = 0.0f;
			for (int i = 0; i < m_fontFace->num_fixed_sizes; ++i)
			{
				float size = m_fontFace->available_sizes[i].size * FT_POS_COEF;
				float delta = ::fabs(size - pointSize72);
				if (delta < bestDelta)
				{
					bestDelta = delta;
					bestSize = size;
				}
			}

			// Check if we found a valid size and try to apply it
			if ((bestSize <= 0.0f) ||
				FT_Set_Char_Size(m_fontFace.get(), 0, FT_F26Dot6(bestSize * 64.0f), 0, 0))
				return false;
		}

		// We have the font size set up, get some informations
		if (m_fontFace->face_flags & FT_FACE_FLAG_SCALABLE)
		{
			const float yScale = m_fontFace->size->metrics.y_scale * FT_POS_COEF * (1.0f / 65536.0f);
			m_ascender = m_fontFace->ascender * yScale;
			m_descender = m_fontFace->descender * yScale;
			m_height = m_fontFace->height * yScale;
		}
		else
		{
			m_ascender = m_fontFace->size->metrics.ascender * FT_POS_COEF;
			m_descender = m_fontFace->size->metrics.descender * FT_POS_COEF;
			m_height = m_fontFace->size->metrics.height * FT_POS_COEF;
		}

		// Create an empty FontGlyph structure for every glyph of the font
		FT_UInt glyphIndex;
		FT_ULong codepoint = FT_Get_First_Char(m_fontFace.get(), &glyphIndex);
		FT_ULong maxCodepoint = codepoint;
		while (glyphIndex)
		{
			if (maxCodepoint < codepoint)
				maxCodepoint = codepoint;

			// Load-up required glyph metrics
			if (FT_Load_Char(m_fontFace.get(), codepoint, FT_LOAD_DEFAULT | FT_LOAD_FORCE_AUTOHINT))
				continue; // Glyph error

						  // Create a new empty FontGlyph with given character code
			const float advance = m_fontFace->glyph->metrics.horiAdvance * FT_POS_COEF;
			m_glyphMap[codepoint] = FontGlyph(advance);

			// Proceed to next glyph
			codepoint = FT_Get_Next_Char(m_fontFace.get(), codepoint, &glyphIndex);
		}

		// Update the amount of code points
		setMaxCodepoint(maxCodepoint);
		return true;
	}

	std::uint32_t Font::getTextureSize(GlyphMap::const_iterator start, GlyphMap::const_iterator end) const
	{
		/// A texture may only grow up to a 4k texture
		const std::uint32_t maxTexsize = 4096;

		// Start with 32x32 pixel texture
		std::uint32_t texSize = 32;
		std::uint32_t glyphCount = 0;

		while (texSize < maxTexsize)
		{
			std::uint32_t x = INTER_GLYPH_PAD_SPACE, y = INTER_GLYPH_PAD_SPACE;
			std::uint32_t yb = INTER_GLYPH_PAD_SPACE;

			bool isTooSmall = false;
			for (GlyphMap::const_iterator c = start; c != end; ++c)
			{
				// Skip glpyhs that are already rendered
				if (c->second.getImage())
					continue;	// Already rendered

								// Load glyph metrics
				if (FT_Load_Char(m_fontFace.get(), c->first, FT_LOAD_DEFAULT | FT_LOAD_FORCE_AUTOHINT))
					continue;	// Could not load glyph metrics, skip it!

				// Calculate the glyph size in pixels
				const std::uint32_t glyphW = static_cast<std::uint32_t>(::ceil(m_fontFace->glyph->metrics.width * FT_POS_COEF)) + INTER_GLYPH_PAD_SPACE;
				const std::uint32_t glyphH = static_cast<std::uint32_t>(::ceil(m_fontFace->glyph->metrics.height * FT_POS_COEF)) + INTER_GLYPH_PAD_SPACE;

				// Adjust the offset
				x += glyphW;
				if (x > texSize)
				{
					x = INTER_GLYPH_PAD_SPACE;
					y = yb;
				}

				std::uint32_t yy = y + glyphH;
				if (yy > texSize)
					isTooSmall = true;

				if (yy > yb)
					yb = yy;

				++glyphCount;
			}

			// The texture size is enough for holding our glyphs
			if (!isTooSmall)
				break;

			// Double the texture size because it's too small
			texSize *= 2;
		}

		return glyphCount ? texSize : 0;
	}

	void Font::setMaxCodepoint(std::uint32_t codepoint)
	{
		m_maxCodepoint = codepoint;

		// Resize the glyph page map
		const std::uint32_t pageCount = (codepoint + GLYPHS_PER_PAGE) / GLYPHS_PER_PAGE;
		const std::uint32_t size = (pageCount + BITS_PER_UINT - 1) / BITS_PER_UINT;
		m_glyphPageLoaded.resize(size, 0);
	}

#pragma pack(push, 1)
	struct TGAHeader
	{
		std::uint8_t	IDLength;
		std::uint8_t	ColorMapType;
		std::uint8_t	DataTypeCode;
		std::uint16_t	wColorMapOrigin;
		std::uint16_t	wColorMapLength;
		std::uint8_t	ColorMapDepth;
		std::uint16_t	wOriginX;
		std::uint16_t	wOriginY;
		std::uint16_t	wWidth;
		std::uint16_t	wHeight;
		std::uint8_t	BitDepth;
		std::uint8_t	ImageDescriptor;
	};
#pragma pack(pop)

	void Font::rasterCallback(const int y, const int count, const FT_Span * const spans, void * const user)
	{
		Spans *sptr = (Spans *)user;
		for (int i = 0; i < count; ++i)
			sptr->push_back(Span(spans[i].x, y, spans[i].len, spans[i].coverage));
	}

	void Font::renderSpans(FT_Library &library, FT_Outline * const outline, Font::Spans *spans)
	{
		FT_Raster_Params params;
		memset(&params, 0, sizeof(params));
		params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
		params.gray_spans = rasterCallback;
		params.user = spans;

		FT_Outline_Render(library, outline, &params);
	}

	void Font::rasterize(std::uint32_t startCodepoint, std::uint32_t endCodepoint)
	{
		GlyphMap::const_iterator start = m_glyphMap.lower_bound(startCodepoint);
		if (start == m_glyphMap.end())
			return;

		GlyphMap::const_iterator origStart = start;
		GlyphMap::const_iterator end = m_glyphMap.upper_bound(endCodepoint);
		while (true)
		{
			// Create a new imageset for glyphs, so calculate how much texture size we will need
			std::uint32_t texSize = getTextureSize(start, end);
			if (!texSize)
				break;	// If all glyphs were already rendered, do nothing

			// Create the imageset
			m_imageSets.emplace_back(FontImageset());
			FontImageset* imageSet = &m_imageSets.back();

			// Create a memory buffer where the glyphs will be rendered and initialize it to 0
			std::vector<std::uint32_t> mem;
			mem.resize(texSize * texSize, 0);

			// Go ahead, line by line, top-left to bottom-right
			std::uint32_t x = INTER_GLYPH_PAD_SPACE, y = INTER_GLYPH_PAD_SPACE;
			std::uint32_t yb = INTER_GLYPH_PAD_SPACE;

			// Set to true when we finish rendering all glyphs we were asked to
			bool finished = false;
			// Set to false when we reach m_glyphMap.end() and we start going backward
			bool forward = true;

			// To converve texture space we will render more glyphs than asked, but never
			// less than asked. First we render all glyphs from start to end.
			// After that, we render glyphs until we reach m_glyphMap.end(), and if there
			// is still free texture space we will go backward from start, until we hit 
			// m_glyphMap.begin()
			while (start != m_glyphMap.end())
			{
				// Check if we finished rendering all the required glyphs
				finished |= (start == end);

				// Check if glyph already rendered
				if (!start->second.getImage())
				{
					// Render the glyph
					if (FT_Load_Char(m_fontFace.get(), start->first, FT_LOAD_NO_BITMAP | FT_LOAD_FORCE_AUTOHINT/* | FT_LOAD_TARGET_NORMAL*/))
					{
						// Error while rendering the glyph - use an empty image for this glyph!
						RectF area;
						Vector2 offset;
						FontImage& image = imageSet->defineImage(area, offset);
						((FontGlyph&)start->second).setImage(&image);
					}
					else
					{
						// Render normal glyph spans
						Spans spans;
						renderSpans(s_freeTypeLib, &m_fontFace->glyph->outline, &spans);

						// Next we need the spans for the outline.
						Spans outlineSpans;
						if (m_outlineWidth > 0.0f)
						{
							FT_Stroker stroker;
							FT_Stroker_New(s_freeTypeLib, &stroker);
							FT_Stroker_Set(stroker, (int)(m_outlineWidth * 64.0f), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

							FT_Glyph glyph;
							if (FT_Get_Glyph(m_fontFace->glyph, &glyph) == 0)
							{
								FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);
								// Again, this needs to be an outline to work.
								if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
								{
									// Render the outline spans to the span list
									FT_Outline *o =
										&reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
									renderSpans(s_freeTypeLib, o, &outlineSpans);
								}

								// Clean up afterwards.
								FT_Stroker_Done(stroker);
								FT_Done_Glyph(glyph);
							}
						}

						// Now we need to put it all together.
						if (spans.empty())
						{
							// Error while rendering the glyph - use an empty image for this glyph!
							RectF area;
							Vector2 offset;
							FontImage& image = imageSet->defineImage(area, offset);
							((FontGlyph&)start->second).setImage(&image);
						}
						else
						{
							// Calculate glyph bounds
							int minX = spans.front().x;
							int maxX = minX;
							int minY = spans.front().y;
							int maxY = minY;
							for (const auto& span : spans)
							{
								if (span.x < minX) minX = span.x;
								if (span.y < minY) minY = span.y;
								if (span.y > maxY) maxY = span.y;
								if (span.x + span.width > maxX) maxX = span.x + span.width;
							}
							for (const auto& span : outlineSpans)
							{
								if (span.x < minX) minX = span.x;
								if (span.y < minY) minY = span.y;
								if (span.y > maxY) maxY = span.y;
								if (span.x + span.width > maxX) maxX = span.x + span.width;
							}

							// Shortcut for glyph size
							const std::int32_t glyphW = (int)(maxX - minX) + INTER_GLYPH_PAD_SPACE;
							const std::int32_t glyphH = (int)(maxY - minY) + INTER_GLYPH_PAD_SPACE;

							// Check if glyph right margin does not exceed texture size
							std::uint32_t xNext = x + glyphW;
							if (xNext > texSize)
							{
								x = INTER_GLYPH_PAD_SPACE;
								xNext = x + glyphW;
								y = yb;
							}

							// Check if glyph bottom margin does not exceed texture size
							std::uint32_t yBot = y + glyphH;
							if (yBot > texSize)
								break;

							if (m_outlineWidth > 0.0f)
							{
								// Loop over the outline spans and just draw them into the image.
								for (Spans::iterator s = outlineSpans.begin(); s != outlineSpans.end(); ++s)
								{
									const int bufferX = (int)x + (s->x - minX) + s->width;
									const int bufferY = (int)((int)y + (int)glyphH - (s->y - minY));
									if (bufferX < static_cast<std::int32_t>(texSize) && bufferY >= 0 && bufferX - s->width >= 0 && bufferY < static_cast<std::int32_t>(texSize))
									{
										std::uint32_t* buffer = mem.data() + bufferY * static_cast<std::int32_t>(texSize) + bufferX;
										for (int w = 0; w < s->width; ++w)
										{
											*buffer-- = Pixel32(0, 0, 0, s->coverage).integer;
										}
									}
								}

								// Then loop over the regular glyph spans and blend them into the image.
								for (Spans::iterator s = spans.begin(); s != spans.end(); ++s)
								{
									const int bufferX = (int)x + (s->x - minX) + s->width;
									const int bufferY = (int)((int)y + (int)glyphH - (s->y - minY));
									if (bufferX < static_cast<std::int32_t>(texSize) && bufferY >= 0 && bufferX - s->width >= 0 && bufferY < static_cast<std::int32_t>(texSize))
									{
										std::uint32_t* buffer = mem.data() + bufferY * static_cast<std::int32_t>(texSize) + bufferX;

										for (int w = 0; w < s->width; ++w)
										{
											Pixel32 &dst = (Pixel32&)*buffer--;
											Pixel32 src = Pixel32(255, 255, 255, s->coverage);
											dst.r = (int)(dst.r + ((src.r - dst.r) * src.a) / 255.0f);
											dst.g = (int)(dst.g + ((src.g - dst.g) * src.a) / 255.0f);
											dst.b = (int)(dst.b + ((src.b - dst.b) * src.a) / 255.0f);
											dst.a = std::min(255, dst.a + src.a);
										}
									}
								}
							}
							else
							{
								// No outline, just render the spans
								for (Spans::iterator s = spans.begin(); s != spans.end(); ++s)
								{
									const int bufferX = (int)x + (s->x - minX) + s->width;
									const int bufferY = (int)((int)y + (int)glyphH - (s->y - minY));
									if (bufferX < static_cast<std::int32_t>(texSize) && bufferY >= 0 && bufferX - s->width >= 0 && bufferY < static_cast<std::int32_t>(texSize))
									{
										std::uint32_t* buffer = mem.data() + bufferY * static_cast<std::int32_t>(texSize) + bufferX;
										for (int w = 0; w < s->width; ++w)
										{
											*buffer-- = Pixel32(255, 255, 255, s->coverage).integer;
										}
									}
								}
							}

							RectF area(
								static_cast<float>(x),
								static_cast<float>(y),
								static_cast<float>(glyphW - minX),
								static_cast<float>(glyphH - minY + INTER_GLYPH_PAD_SPACE));
							Vector2 offset(
								m_fontFace->glyph->metrics.horiBearingX * FT_POS_COEF,
								-m_fontFace->glyph->metrics.horiBearingY * FT_POS_COEF + m_descender);
							
							FontImage& image = imageSet->defineImage(area, offset);
							((FontGlyph&)start->second).setImage(&image);

							// Advance to next position
							x = xNext;
							if (yBot > yb)
							{
								yb = yBot;
							}
						}
					}
				}

				// Go to next glyph if we are going forward
				if (forward)
				{
					if (++start == m_glyphMap.end())
					{
						finished = true;
						forward = false;
						start = origStart;
					}
				}

				// Go to previous glyph if we are going backward
				if (!forward)
				{
					if ((start == m_glyphMap.begin()) || (--start == m_glyphMap.end()))
						break;
				}
			}

			// Holds the texture data
			std::vector<std::uint8_t> textureData;
			textureData.resize(sizeof(TGAHeader) + mem.size() * sizeof(std::uint32_t), 0);

			// Create texture file in memory
			TGAHeader Header;
			Header.IDLength = 0;
			Header.ColorMapType = 0;
			Header.DataTypeCode = 2;
			Header.wColorMapOrigin = 0;
			Header.wColorMapLength = 0;
			Header.ColorMapDepth = 0;
			Header.wOriginX = 0;
			Header.wOriginY = 0;
			Header.wWidth = texSize;
			Header.wHeight = texSize;
			Header.BitDepth = 32;
			Header.ImageDescriptor = 32;
			memcpy(&textureData[0], &Header, sizeof(Header));

			// Copy the image data
			memcpy(&textureData[sizeof(Header)], &mem[0], mem.size() * sizeof(std::uint32_t));

			// Create imageset texture
			std::uint32_t textureId = K2D_CreateTextureFromMemory(reinterpret_cast<const char*>(&textureData[0]), (std::uint32_t)textureData.size());
			imageSet->setTexture(textureId);

			// Check if finished
			if (finished)
				break;
		}
	}

	void Font::drawSpansToBuffer(std::uint32_t * buffer, std::uint32_t textureSize, const Font::Spans& spans) const
	{

	}

	float Font::getTextWidth(const std::wstring & text, float scale)
	{
		float curWidth = 0.0f, advWidth = 0.0f, width = 0.0f;

		// Iterate through all characters of the string
		for (size_t c = 0; c < text.length(); ++c)
		{
			// Get the glyph data
			const FontGlyph* glyph = getGlyphData(text[c]);
			if (glyph)
			{
				// Adjust the width
				width = glyph->getRenderedAdvance(scale);
				if (advWidth + width > curWidth)
					curWidth = advWidth + width;

				advWidth += glyph->getAdvance(scale);
			}
		}

		return std::max(advWidth, curWidth);
	}

	const FontGlyph * Font::getGlyphData(std::uint32_t codepoint)
	{
		if (codepoint > m_maxCodepoint)
			return nullptr;

		// First see if the glyph data is already rasterized
		const std::uint32_t page = codepoint / GLYPHS_PER_PAGE;
		const std::uint32_t mask = 1 << (page & (BITS_PER_UINT - 1));
		if (!(m_glyphPageLoaded[page / BITS_PER_UINT] & mask))
		{
			// Not yet rasterized, so do it now and remember that we did it
			m_glyphPageLoaded[page / BITS_PER_UINT] |= mask;
			rasterize(
				codepoint & ~(GLYPHS_PER_PAGE - 1),
				codepoint | (GLYPHS_PER_PAGE - 1));
		}

		// Fint the glyph data
		auto it = m_glyphMap.find(codepoint);
		return (it != m_glyphMap.end()) ? &it->second : nullptr;
	}

	void Font::drawText(const std::wstring & text, const Vector2 & position, float scale)
	{
		const float baseY = position.Y + getBaseline(scale);
		Vector2 glyphPos(position);

		for (size_t c = 0; c < text.length(); ++c)
		{
			const FontGlyph* glyph = nullptr;
			if ((glyph = getGlyphData(text[c])))
			{
				const FontImage* const image = glyph->getImage();
				glyphPos.Y = baseY - (image->getOffsetY() - image->getOffsetY() * scale);

				image->draw(glyphPos, glyph->getImage()->getSize());
				glyphPos.X += glyph->getAdvance(scale);
			}
		}
	}
}
