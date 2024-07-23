#include "Utils.h"

namespace neural::utils {
std::vector<char> loadBinary(std::string a_filename)
{
    std::ifstream fin(a_filename, std::ios::binary);

    fin.seekg(0, std::ios_base::end);
    auto size = fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    std::vector<char> buffer(size);
    fin.read(buffer.data(), size);

    return buffer;
}

DirectX::XMFLOAT3 transformFloat3(DirectX::XMFLOAT3 a_vector, DirectX::FXMMATRIX a_matrix) {
    DirectX::XMVECTOR vector = DirectX::XMLoadFloat3(&a_vector);
    vector = DirectX::XMVector3Transform(vector, a_matrix);
    DirectX::XMFLOAT3 vectorf3;
    DirectX::XMStoreFloat3(&vectorf3, vector);
    return vectorf3;
}
}