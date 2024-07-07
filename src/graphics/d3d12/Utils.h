#pragma once

#include <utils/Macros.h>

#include <d3d12.h>
#include <wrl.h>

#include <string_view>
#include <fstream>
#pragma once
#include <vector>

using Microsoft::WRL::ComPtr;
namespace neural::graphics::utils {
std::vector<char> loadBinary(std::string a_filename);
}