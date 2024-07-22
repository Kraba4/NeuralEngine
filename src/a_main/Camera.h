#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

namespace neural {
class Camera {
public:
    // Camera();
    // ~Camera();
    // Get/Set world camera position.
    DirectX::XMVECTOR getPosition() const;
    DirectX::XMFLOAT3 getPosition3f() const;
    void setPosition(float x, float y, float z);
    void setPosition(const DirectX::XMFLOAT3& v);

    // Get camera basis vectors.
    DirectX::XMVECTOR getRight() const;
    DirectX::XMFLOAT3 getRight3f() const;
    DirectX::XMVECTOR getUp() const;
    DirectX::XMFLOAT3 getUp3f() const;
    DirectX::XMVECTOR getForward() const;
    DirectX::XMFLOAT3 getForward3f() const;

    // Get frustum properties.
    float getNearZ() const;
    float getFarZ() const;
    float getAspect() const;
    float getFovY() const;
    float getFovX() const;

    // Get near and far plane dimensions in view space coordinates.
    float getNearWindowWidth() const;
    float getNearWindowHeight() const;
    float getFarWindowWidth() const;
    float getFarWindowHeight() const;

    // Set frustum.
    void setFrustum(float a_fovY, float a_aspect, float a_zNear, float a_zFar);

    // Define camera space via LookAt parameters.
    void lookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target,
        DirectX::FXMVECTOR worldUp);
    void lookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target,
        const DirectX::XMFLOAT3& up);

    // Get View/Proj matrices.
    DirectX::XMMATRIX getView() const;
    DirectX::XMMATRIX getProj() const;
    DirectX::XMFLOAT4X4 getView4x4f() const;
    DirectX::XMFLOAT4X4 getProj4x4f() const;

    // Strafe/Walk the camera a distance d.
    void moveRight(float d);
    void moveForward(float d);

    // Rotate the camera.
    void rotateHorizontal(float angle);
    void rotateVertical(float angle);

    // After modifying camera position/orientation, call to rebuild the view
    // matrix.
    void updateViewMatrix();

private:
    // Camera coordinate system with coordinates relative to world space.
    DirectX::XMFLOAT3 m_position = { 0.0f, 3.0f, 0.0f };
    DirectX::XMFLOAT3 m_right = { 1.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_up = { 0.0f, 1.0f, 0.0f };
    DirectX::XMFLOAT3 m_forward = { 0.0f, 0.0f, 1.0f };

    // Cache frustum properties.
    float m_nearZ = 0.0f;
    float m_farZ = 0.0f;
    float m_aspect = 0.0f;
    float m_fovY = 0.0f;
    float m_nearWindowHeight = 0.0f;
    float m_farWindowHeight = 0.0f;
    bool m_viewDirty = true;

    // Cache View/Proj matrices.
    DirectX::XMFLOAT4X4 m_view;
    DirectX::XMFLOAT4X4 m_proj;
};
}  // namespace neural
