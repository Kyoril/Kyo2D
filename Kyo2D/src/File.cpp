#include "File.h"
#include <fstream>

//=============================================================================
bool readFileContents(const std::wstring& filename, FileData& out_data)
{
	// Load the file stream
	std::ifstream stream(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if (!stream.is_open())
	{
		return false;
	}

	// Get file size in bytes and set pointer to the beginning
	auto size = stream.tellg();
	stream.seekg(0, std::ios::beg);

	// Resize data array
	out_data.resize(static_cast<size_t>(size));

	// Read data
	if (!stream.read(reinterpret_cast<char*>(&out_data[0]), static_cast<std::streamsize>(size)))
	{
		return false;
	}

	// Close stream again
	stream.close();
	return true;
}