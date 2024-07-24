#include "Camera.h"

namespace neural {
using namespace DirectX;
XMVECTOR Camera::getPosition() const
{
    return XMLoadFloat3(&m_position);
}

XMFLOAT3 Camera::getPosition3f() const
{
    return m_position;
}

void Camera::setPosition(float x, float y, float z)
{
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
}

void Camera::setPosition(const DirectX::XMFLOAT3& a_position)
{
    m_position = a_position;
}

DirectX::XMVECTOR Camera::getRight() const
{
    return XMLoadFloat3(&m_right);
}

DirectX::XMFLOAT3 Camera::getRight3f() const
{
    return m_right;
}

DirectX::XMVECTOR Camera::getUp() const
{
    return XMLoadFloat3(&m_up);
}

DirectX::XMFLOAT3 Camera::getUp3f() const
{
    return m_up;
}

DirectX::XMVECTOR Camera::getForward() const
{
    return  XMLoadFloat3(&m_forward);
}

DirectX::XMFLOAT3 Camera::getForward3f() const
{
    return m_forward;
}

float Camera::getNearZ() const
{
    return m_nearZ;
}

float Camera::getFarZ() const
{
    return m_farZ;
}

float Camera::getAspect() const
{
    return m_aspect;
}

float Camera::getFovY() const
{
    return m_fovY;
}

float Camera::getFovX() const
{
    float halfWidth = 0.5f * getNearWindowWidth();
    return 2.0f * atan(halfWidth / m_nearZ);
}

float Camera::getNearWindowWidth() const
{
    return m_aspect * m_nearWindowHeight;
}

float Camera::getNearWindowHeight() const
{
    return m_nearWindowHeight;
}

float Camera::getFarWindowWidth() const
{
    return  m_aspect * m_farWindowHeight;
}

float Camera::getFarWindowHeight() const
{
    return m_farWindowHeight;
}

void Camera::setFrustum(float a_fovY, float a_aspect, float a_zNear, float a_zFar)
{
    m_fovY = a_fovY;
    m_aspect = a_aspect;
    m_nearZ = a_zNear;
    m_farZ = a_zFar;
    m_nearWindowHeight = 2.0f * m_nearZ * tanf(0.5f * m_fovY);
    m_farWindowHeight = 2.0f * m_farZ * tanf(0.5f * m_fovY);
    XMMATRIX P = XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
    XMStoreFloat4x4(&m_proj, P);
}

void Camera::lookAt(DirectX::FXMVECTOR a_pos, DirectX::FXMVECTOR a_target, DirectX::FXMVECTOR a_worldUp)
{
    XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(a_pos, a_target, a_worldUp));

    m_right = { m_view(0,0), m_view(0,1), m_view(0,2) };
    m_up = { m_view(1,0), m_view(1,1), m_view(1,2) };
    m_forward = { m_view(2,0), m_view(2,1), m_view(2,2) };
    XMStoreFloat3(&m_position, a_pos);
}

void Camera::lookAt(const DirectX::XMFLOAT3& a_pos, const DirectX::XMFLOAT3& a_target, const DirectX::XMFLOAT3& a_worldUp)
{
    XMVECTOR pos = XMLoadFloat3(&a_pos);
    XMVECTOR target = XMLoadFloat3(&a_target);
    XMVECTOR worldUp = XMLoadFloat3(&a_worldUp);

    XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(pos, target, worldUp));

    m_right = { m_view(0,0), m_view(0,1), m_view(0,2) };
    m_up = { m_view(1,0), m_view(1,1), m_view(1,2) };
    m_forward = { m_view(2,0), m_view(2,1), m_view(2,2) };
    XMStoreFloat3(&m_position, pos);
}

DirectX::XMMATRIX Camera::getView() const
{
    return XMLoadFloat4x4(&m_view);
}

DirectX::XMMATRIX Camera::getProj() const
{
    return XMLoadFloat4x4(&m_proj);
}

DirectX::XMFLOAT4X4 Camera::getView4x4f() const
{
    return m_view;
}

DirectX::XMFLOAT4X4 Camera::getProj4x4f() const 
{
    return m_proj;
}

float* Camera::getViewPtr()
{
    return &m_view._11;
}

float* Camera::getProjPtr()
{
    return &m_proj._11;
}

void Camera::moveRight(float d)
{
    XMVECTOR s = XMVectorReplicate(d);
    XMVECTOR r = XMLoadFloat3(&m_right);
    XMVECTOR p = XMLoadFloat3(&m_position);
    XMStoreFloat3(&m_position, XMVectorMultiplyAdd(s, r, p));
    m_viewDirty = true;
}

void Camera::moveForward(float d)
{
    XMVECTOR s = XMVectorReplicate(d);
    XMVECTOR l = XMLoadFloat3(&m_forward);
    XMVECTOR p = XMLoadFloat3(&m_position);
    XMStoreFloat3(&m_position, XMVectorMultiplyAdd(s, l, p));
    m_viewDirty = true;
}

void Camera::rotateHorizontal(float angle)
{
    // Rotate the basis vectors about the world y-axis.
    XMMATRIX R = XMMatrixRotationY(angle);
    XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), R));
    XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
    XMStoreFloat3(&m_forward, XMVector3TransformNormal(XMLoadFloat3(&m_forward), R));
    m_viewDirty = true;
}

void Camera::rotateVertical(float angle)
{
    // Rotate up and look vector about the right vector.
    XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_right), angle);
    XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
    XMStoreFloat3(&m_forward, XMVector3TransformNormal(XMLoadFloat3(&m_forward), R));
    m_viewDirty = true;
}

void Camera::updateViewMatrix()
{
    if (m_viewDirty)
    {
        XMVECTOR R = XMLoadFloat3(&m_right);
        XMVECTOR U = XMLoadFloat3(&m_up);
        XMVECTOR L = XMLoadFloat3(&m_forward);
        XMVECTOR P = XMLoadFloat3(&m_position);
        // Keep camera’s axes orthogonal to each other and of unit length.
        L = XMVector3Normalize(L);
        U = XMVector3Normalize(XMVector3Cross(L, R));
        // U, L already ortho-normal, so no need to normalize cross product.
        R = XMVector3Cross(U, L);
        // Fill in the view matrix entries.
        float x = -XMVectorGetX(XMVector3Dot(P, R));
        float y = -XMVectorGetX(XMVector3Dot(P, U));
        float z = -XMVectorGetX(XMVector3Dot(P, L));
        XMStoreFloat3(&m_right, R);
        XMStoreFloat3(&m_up, U);
        XMStoreFloat3(&m_forward, L);
        m_view(0, 0) = m_right.x;
        m_view(1, 0) = m_right.y;
        m_view(2, 0) = m_right.z;
        m_view(3, 0) = x;
        m_view(0, 1) = m_up.x;
        m_view(1, 1) = m_up.y;
        m_view(2, 1) = m_up.z;
        m_view(3, 1) = y;
        m_view(0, 2) = m_forward.x;
        m_view(1, 2) = m_forward.y;
        m_view(2, 2) = m_forward.z;
        m_view(3, 2) = z;
        m_view(0, 3) = 0.0f;
        m_view(1, 3) = 0.0f;
        m_view(2, 3) = 0.0f;
        m_view(3, 3) = 1.0f;
        m_viewDirty = false;
    }
}
}