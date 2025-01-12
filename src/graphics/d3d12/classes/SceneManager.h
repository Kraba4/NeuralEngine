#pragma once
#include <DirectXMath.h>
#include <utils/Macros.h>
#include <graphics/d3d12/CommonGraphicsHeaders.h>
#include "ResourceManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <par_shapes.h>

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
    struct Object {
        DirectX::XMFLOAT4X4 worldMatrix;
        DirectX::XMFLOAT4 color;
        size_t meshIndex;
    };

    void initialize(ID3D12Device* a_device, int a_numMeshes);
    void loadParMesh(int a_meshId, const char* a_meshName, par_shapes_mesh* a_parMesh, bool a_needLoadUV = true,  bool a_needLoadNormals = true);
    void loadMesh(int a_meshId, const char* a_meshName, const std::vector<Vertex>& a_vertices,
                  const std::vector<uint32_t>& a_indices,
                  MeshTransform a_transform = {});
    void loadMeshFromFile(int a_meshId, const char* a_meshName, const char* a_path, MeshTransform a_transform = {});
    void uploadMeshesOnGPU(ID3D12GraphicsCommandList* a_commandList,
                           ResourceManager* a_pResourceManager, 
                           int a_vertexBufferId, int a_indexBufferId,
                           int a_vertexBufferUploadId, int a_indexBufferUploadId);

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

    const MeshInfo& getMeshInfo(int a_meshId) {
        assert(a_meshId < m_meshes.size());
        return m_meshes[a_meshId];
    }
    const std::vector<Object>& getObjects() {
        return m_objects;
    }
    Object& getObjectByName(std::string a_objectName) {
        assert(m_objectNameToIndex.contains(a_objectName));
        return m_objects[m_objectNameToIndex[a_objectName]];
    }
    void newObject(std::string a_name, int a_meshId, DirectX::XMFLOAT4 color, DirectX::XMFLOAT4X4 a_worldMatrix) {
        assert(!m_objectNameToIndex.contains(a_name));
        assert(a_meshId < m_meshes.size());
        m_objectNameToIndex[a_name] = m_objects.size();
        m_objects.push_back({a_worldMatrix, color, (size_t)a_meshId});
    }

    std::vector<const char*> getObjectNames() {
        std::vector<const char*> names;
        for (auto& pair : m_objectNameToIndex) {
            // warning uses fact that m_objectNameToIndex reallocate not change string data
            names.push_back(pair.first.c_str());
        }
        return names;
    }
    size_t numObjects() {
        return m_objects.size();
    }
private:
    ID3D12Device* m_device;

    std::vector<Object> m_objects;
    std::unordered_map<std::string, size_t> m_objectNameToIndex;
    std::vector<MeshInfo> m_meshes;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    Buffer m_vertexBuffer;
    Buffer m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};
}