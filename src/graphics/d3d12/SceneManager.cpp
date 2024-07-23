#include "SceneManager.h"
#include <iostream>
#include <utils/Utils.h>

namespace neural::graphics {
using utils::transformFloat3;
using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;
using DirectX::XMMatrixScaling;
using DirectX::XMMatrixMultiply;
using DirectX::XMMatrixRotationRollPitchYaw;

void SceneManager::initialize(ID3D12Device* a_device) {
    m_device = a_device;
}
void SceneManager::loadMesh(const char* a_meshName, const std::vector<Vertex>& a_vertices,
                            const std::vector<uint32_t>& a_indices, MeshTransform a_transform)
{
    XMMATRIX scalingMatrix = XMMatrixScaling(a_transform.scale, a_transform.scale, a_transform.scale);
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(a_transform.rotation.y),
        DirectX::XMConvertToRadians(a_transform.rotation.x),
        DirectX::XMConvertToRadians(a_transform.rotation.z));
    XMMATRIX transformMatrix = XMMatrixMultiply(scalingMatrix, rotationMatrix);

    MeshInfo meshInfo;
    meshInfo.startVertex = m_vertices.size();
    for (int i = 0; i < a_vertices.size(); ++i) {
        XMFLOAT3 position =
            transformFloat3(a_vertices[i].position, transformMatrix);
        XMFLOAT3 normal =
            transformFloat3(a_vertices[i].normal, transformMatrix);

        m_vertices.push_back({
            { position.x, position.y, position.z },
            { normal.x, normal.y,  normal.z },
            { 1, 1}
            });
    }
    meshInfo.vertexCount = m_vertices.size() - meshInfo.startVertex;

    meshInfo.startIndex = m_indices.size();
    for (int i = 0; i < a_indices.size(); ++i) {
        m_indices.push_back(a_indices[i]);
    }
    meshInfo.indexCount = m_indices.size() - meshInfo.startIndex;
    m_meshes[a_meshName] = meshInfo;
}
void SceneManager::loadMeshFromFile(const char* a_meshName, const char* a_path, MeshTransform a_transform) {
    Assimp::Importer assetImporter;
    const aiScene* scene = assetImporter.ReadFile(a_path,
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    assert(scene);
    const aiMesh* mesh = scene->mMeshes[0];

    XMMATRIX scalingMatrix = XMMatrixScaling(a_transform.scale, a_transform.scale, a_transform.scale);
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(a_transform.rotation.y),
                                                           DirectX::XMConvertToRadians(a_transform.rotation.x),
                                                           DirectX::XMConvertToRadians(a_transform.rotation.z));
    XMMATRIX transformMatrix = XMMatrixMultiply(scalingMatrix, rotationMatrix);

    MeshInfo meshInfo;
    meshInfo.startVertex = m_vertices.size();
    for (int i = 0; i < mesh->mNumVertices; ++i) {
        XMFLOAT3 position = 
            transformFloat3({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z }, transformMatrix);
        XMFLOAT3 normal =
            transformFloat3({ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z }, transformMatrix);

        m_vertices.push_back({
            { position.x, position.y, position.z },
            { normal.x, normal.y,  normal.z },
            { 1, 1}
            });
    }
    meshInfo.vertexCount = m_vertices.size() - meshInfo.startVertex;

    meshInfo.startIndex = m_indices.size();
    for (int i = 0; i < mesh->mNumFaces; ++i) {
        const auto face = mesh->mFaces[i];
#ifdef _DEBUG 
        if (face.mNumIndices == 3) {
            m_indices.push_back(face.mIndices[0]);
            m_indices.push_back(face.mIndices[1]);
            m_indices.push_back(face.mIndices[2]);
        }
#else
        m_indices.push_back(face.mIndices[0]);
        m_indices.push_back(face.mIndices[1]);
        m_indices.push_back(face.mIndices[2]);
#endif
    }
    meshInfo.indexCount = m_indices.size() - meshInfo.startIndex;
#ifdef _DEBUG 
    if (meshInfo.indexCount != mesh->mNumFaces * 3) {
        std::cout << a_path << ": not just triangles\n";
    }
#endif
    m_meshes[a_meshName] = meshInfo;
}
void SceneManager::uploadMeshesOnGPU(ID3D12GraphicsCommandList* a_commandList) {
    m_vertexBuffer.initialize(m_device, {
            .resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(m_vertices[0]) * m_vertices.size()),
        });
    NAME_DX_OBJECT(m_vertexBuffer.getID3D12Resource(), L"VertexBuffer");

    m_indexBuffer.initialize(m_device, {
            .resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(m_indices[0]) * m_indices.size())
        });
    NAME_DX_OBJECT(m_indexBuffer.getID3D12Resource(), L"IndexBuffer");

    m_vertexBuffer.initializeUpload(m_device);
    m_indexBuffer.initializeUpload(m_device);

    D3D12_RESOURCE_BARRIER barriers[] = {
    CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.getID3D12Resource(),
    D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST),
    CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.getID3D12Resource(),
    D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST)
    };
    a_commandList->ResourceBarrier(_countof(barriers), barriers);

    m_vertexBuffer.uploadData(a_commandList, m_vertices.data());
    m_indexBuffer.uploadData(a_commandList, m_indices.data());

    m_vertexBufferView = m_vertexBuffer.getVertexBufferView(sizeof(Vertex));
    m_indexBufferView = m_indexBuffer.getIndexBufferView();
}
}