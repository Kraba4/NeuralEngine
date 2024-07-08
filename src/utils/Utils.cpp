#include "Utils.h"

std::vector<char> neural::utils::loadBinary(std::string a_filename)
{
	std::ifstream fin(a_filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = fin.tellg();
	fin.seekg(0, std::ios_base::beg);

	std::vector<char> buffer(size);
	fin.read(buffer.data(), size);

	return buffer;
}
