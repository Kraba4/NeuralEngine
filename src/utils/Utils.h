#pragma once

#include <utils/Macros.h>

#include <d3d12.h>
#include <wrl.h>

#include <string_view>
#include <fstream>
#pragma once
#include <vector>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
namespace neural::utils {
std::vector<char> loadBinary(std::string a_filename);
DirectX::XMFLOAT3 transformFloat3(DirectX::XMFLOAT3 a_vector, DirectX::FXMMATRIX a_matrix);
}