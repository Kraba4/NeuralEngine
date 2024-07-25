#pragma once
#include <DirectXMath.h>
#include <utils/Macros.h>
#include <graphics/d3d12/CommonGraphicsHeaders.h>
#include "Resources.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_map>

namespace neural::graphics {
class SceneManager {
public:
    struct Vertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 textureCoordinates;
    };
    struct MeshInfo {
        size_t startVertex;
        size_t vertexCount;
        size_t startIndex;
        size_t indexCount;
    };
    struct MeshTransform {
        DirectX::XMFLOAT3 rotation = { 0, 0, 0 };
        float scale = 1.0f;
    };
    void initialize(ID3D12Device* a_device);
    void loadMesh(const char* a_meshName, const std::vector<Vertex>& a_vertices,
                  const std::vector<uint32_t>& a_indices,
                  MeshTransform a_transform = {});
    void loadMeshFromFile(const char* a_meshName, const char* a_path, MeshTransform a_transform = {});
    void uploadMeshesOnGPU(ID3D12GraphicsCommandList* a_commandList, ResourceManager* a_pResourceManager);

    ID3D12Resource* getVertexBuffer() {
        return m_vertexBuffer.getID3D12Resource();
    }
    ID3D12Resource* getIndexBuffer() {
        return m_indexBuffer.getID3D12Resource();
    }
    const D3D12_VERTEX_BUFFER_VIEW& getVertexBufferView() const {
        return m_vertexBufferView;
    }
    const D3D12_INDEX_BUFFER_VIEW& getIndexBufferView() const {
        return m_indexBufferView;
    }
    const MeshInfo& getMeshInfo(const char* a_meshName) {
        return m_meshes[a_meshName];
    }
private:
    ID3D12Device* m_device;

    std::unordered_map<std::string, MeshInfo> m_meshes;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    Buffer m_vertexBuffer;
    Buffer m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};
}