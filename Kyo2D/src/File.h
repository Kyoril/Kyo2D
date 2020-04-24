#pragma once

#include <string>
#include <vector>


/// Contains file data
typedef std::vector<std::uint8_t> FileData;


/// Reads the contents of a file into a vector.
/// @param filename The file name.
/// @param out_data Reference to the struct which will hold the file data.
/// @return false if there was an error loading the file.
bool readFileContents(const std::wstring& filename, FileData& out_data);
