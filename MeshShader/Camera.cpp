#include "stdafx.h"
#include <cmath>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathMisc.inl>
#include <DirectXMathVector.inl>
#include <malloc.h>
#include <string>
#include "Camera.h"
#include "Utility.h"

using namespace std;
using namespace DirectX;

Camera::Camera(string camera_name)
	: m_ViewDirty(true)
	, m_InverseViewDirty(true)
	, m_ProjectionDirty(true)
	, m_InverseProjectionDirty(true)
	, m_vFoV(90.0f)
	, m_AspectRatio(1.0f)
	, m_zNear(0.001f)
	, m_zFar(10000.0f)
	, name(camera_name)
{
 //   ResetView();
	pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
	pData->m_Translation = XMVectorZero();
	pData->m_Rotation = XMQuaternionIdentity();
	pData->m_FocalPoint = XMVectorZero();
}

Camera::~Camera()
{
	_aligned_free(pData);
}
#include <windows.h>

void Camera::Update(UINT64 frameCount)
{
	UINT64 span = frameCount; // (float)((LONG64)(frameCount / 100000.0f) % 1000) / 1000.0f;
	float y = (0.5f); // *sinf(((float)frameCount / 240.0f) * XM_PIDIV2));
#if 0
	float x = 1.0; // cosf(XM_2PI * span) * 100.0f;
	float z = 3.5; // sinf(XM_2PI * span) * 100.0f;
#else
	float x = cosf(((float)frameCount / 1960.0f) * XM_2PI) * 3.50f;
	float z = (sinf(((float)frameCount / 1960.0f) * XM_2PI) * 3.50f) + 0.6f;
#endif

	XMVECTOR eye = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorSet(0, 0, 0, 1.0f);
//	position = { x,y,z,1 };
//	target_location = { 0,0,0,1 };
//	target_vector = VECTOR(target_location, position);
	FXMVECTOR up = XMVectorSet(0, 1, 0, 0.0f);
	set_LookAt(eye, target, up);
}

void XM_CALLCONV Camera::set_LookAt(FXMVECTOR eye, FXMVECTOR target, FXMVECTOR up)
{
	pData->m_ViewMatrix = XMMatrixLookAtLH(eye, target, up);

	pData->m_Translation = eye;
	pData->m_Rotation = XMQuaternionRotationMatrix(XMMatrixTranspose(pData->m_ViewMatrix));

	m_InverseViewDirty = true;
	m_ViewDirty = false;
}

XMMATRIX Camera::get_ViewMatrix() const
{
	if (m_ViewDirty)
	{
		UpdateViewMatrix();
	}
	return pData->m_ViewMatrix;
}

XMMATRIX Camera::get_InverseViewMatrix() const
{
	if (m_ViewDirty || m_InverseViewDirty)
	{
		pData->m_InverseViewMatrix = XMMatrixInverse(nullptr, get_ViewMatrix());
		m_InverseViewDirty = false;
	}

	return pData->m_InverseViewMatrix;
}

void Camera::set_Projection(float fovy, float aspect, float zNear, float zFar)
{
	m_vFoV = fovy;
	m_AspectRatio = aspect;
	m_zNear = zNear;
	m_zFar = zFar;

	m_ProjectionDirty = true;
	m_InverseProjectionDirty = true;
}

XMMATRIX Camera::get_ProjectionMatrix() const
{
	if (m_ProjectionDirty)
	{
		UpdateProjectionMatrix();
	}

	return pData->m_ProjectionMatrix;
}

XMMATRIX Camera::get_InverseProjectionMatrix() const
{
	if (m_ProjectionDirty || m_InverseProjectionDirty)
	{
		UpdateInverseProjectionMatrix();
	}

	return pData->m_InverseProjectionMatrix;
}

void Camera::set_FoV(float fovy)
{
	if (m_vFoV != fovy)
	{
		m_vFoV = fovy;
		m_ProjectionDirty = true;
		m_InverseProjectionDirty = true;
	}
}

float Camera::get_FoV() const
{
	return m_vFoV;
}

void XM_CALLCONV Camera::set_Translation(FXMVECTOR translation)
{
	pData->m_Translation = translation;
	m_ViewDirty = true;
}

XMVECTOR Camera::get_Translation() const
{
	return pData->m_Translation;
}

void XM_CALLCONV Camera::set_FocalPoint(DirectX::FXMVECTOR focalPoint)
{
	pData->m_FocalPoint = focalPoint;
	m_ViewDirty = true;
}

DirectX::XMVECTOR Camera::get_FocalPoint() const
{
	return pData->m_FocalPoint;
}

void Camera::set_Rotation(FXMVECTOR rotation)
{
	pData->m_Rotation = rotation;
	m_ViewDirty = true;
}

XMVECTOR Camera::get_Rotation() const
{
	return pData->m_Rotation;
}

void XM_CALLCONV Camera::Translate(FXMVECTOR translation, Space space)
{
	switch (space)
	{
		case Space::Local:
			{
				pData->m_Translation += XMVector3Rotate(translation, pData->m_Rotation);
			}
			break;
		case Space::World:
			{
				pData->m_Translation += translation;
			}
			break;
	}

	pData->m_Translation = XMVectorSetW(pData->m_Translation, 1.0f);

	m_ViewDirty = true;
}

void XM_CALLCONV Camera::MoveFocalPoint(DirectX::FXMVECTOR focalPoint, Space space)
{
	switch (space)
	{
		case Space::Local:
			{
				pData->m_FocalPoint += XMVector3Rotate(focalPoint, pData->m_Rotation);
			}
			break;
		case Space::World:
			{
				pData->m_FocalPoint += focalPoint;
			}
			break;
	}

	pData->m_FocalPoint = XMVectorSetW(pData->m_FocalPoint, 1.0f);

	m_ViewDirty = true;
}


void Camera::Rotate(FXMVECTOR quaternion)
{
	pData->m_Rotation = XMQuaternionMultiply(quaternion, pData->m_Rotation);

	m_ViewDirty = true;
}

void Camera::UpdateViewMatrix() const
{
	XMMATRIX rotationMatrix = XMMatrixTranspose(XMMatrixRotationQuaternion(pData->m_Rotation));
	XMMATRIX translationMatrix = XMMatrixTranslationFromVector(-(pData->m_Translation));
	XMMATRIX focalMatrix = XMMatrixTranslationFromVector(-(pData->m_FocalPoint));

	pData->m_ViewMatrix = focalMatrix * rotationMatrix * translationMatrix;

	m_InverseViewDirty = true;
	m_ViewDirty = false;
}

void Camera::UpdateInverseViewMatrix() const
{
	if (m_ViewDirty)
	{
		UpdateViewMatrix();
	}

	pData->m_InverseViewMatrix = XMMatrixInverse(nullptr, pData->m_ViewMatrix);
	m_InverseViewDirty = false;
}

void Camera::UpdateProjectionMatrix() const
{
	pData->m_ProjectionMatrix =
		XMMatrixPerspectiveFovLH(XMConvertToRadians(m_vFoV), m_AspectRatio, m_zNear, m_zFar);

	m_ProjectionDirty = false;
	m_InverseProjectionDirty = true;
}

void Camera::UpdateInverseProjectionMatrix() const
{
	pData->m_InverseProjectionMatrix = XMMatrixInverse(nullptr, get_ProjectionMatrix());
	m_InverseProjectionDirty = false;
}



#if 0
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include <cmath>
#include <DirectXMath.h>
#include <DirectXMathConvert.inl>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <string>
#include <windows.h>
#include "Camera.h"

using namespace DirectX;
using namespace std;


Camera::Camera(std::string nam) :
	m_initialPosition(0, 0, -200),
	m_position(m_initialPosition),
	m_target(0, 0, 1000),
	m_yaw(XM_PI),
	m_pitch(0.0f),
	m_lookDirection(0, 0, 1),
	m_upDirection(0, 1, 0),
	m_moveSpeed(0.0f),
	m_turnSpeed(0.1f),
	m_keysPressed{}
{}

void Camera::Init(DirectX::XMFLOAT3 position)
{
//	m_initialPosition = position;
	Reset();
}

void Camera::SetMoveSpeed(float unitsPerSecond)
{
	m_moveSpeed = unitsPerSecond;
}

void Camera::SetTurnSpeed(float radiansPerSecond)
{
	m_turnSpeed = radiansPerSecond;
}

void Camera::Reset()
{
	m_position = m_initialPosition;
	m_yaw = XM_PI;
	m_pitch = 0.0f;
	m_lookDirection = { 0, 0, 1 };
}

void Camera::Update(float elapsedSeconds)
{
	// Calculate the move vector in camera space.
	XMFLOAT3 move(0, 0, 0);

	if (m_keysPressed.a)move.x -= 1.0f;
	if (m_keysPressed.d)move.x += 1.0f;
	if (m_keysPressed.w)move.z -= 1.0f;
	if (m_keysPressed.s)move.z += 1.0f;

	if (fabs(move.x) > 0.1f && fabs(move.z) > 0.1f)
	{
		XMVECTOR vector = XMVector3Normalize(XMLoadFloat3(&move));
		move.x = XMVectorGetX(vector);
		move.z = XMVectorGetZ(vector);
	}

	float moveInterval = m_moveSpeed * elapsedSeconds;
	float rotateInterval = m_turnSpeed * elapsedSeconds;

	if (m_keysPressed.left)m_yaw += rotateInterval;
	if (m_keysPressed.right)m_yaw -= rotateInterval;
	if (m_keysPressed.up)m_pitch += rotateInterval;
	if (m_keysPressed.down)m_pitch -= rotateInterval;
	m_yaw += rotateInterval;
	// Prevent looking too far up or down.
	m_pitch = min(m_pitch, XM_PIDIV4);
	m_pitch = max(-XM_PIDIV4, m_pitch);

	// Move the camera in model space.
	float x = move.x * -cosf(m_yaw) - move.z * sinf(m_yaw);
	float z = move.x * sinf(m_yaw) - move.z * cosf(m_yaw);
	m_position.x += x * moveInterval;
	m_position.z += z * moveInterval;

	// Determine the look direction.
	float r = cosf(m_pitch);
	m_lookDirection.x = r * sinf(m_yaw);
	m_lookDirection.y = sinf(m_pitch);
	m_lookDirection.z = r * cosf(m_yaw);
}

XMMATRIX Camera::GetViewMatrix()
{
	return XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookDirection), XMLoadFloat3(&m_upDirection));
//	return XMMatrixLookToRH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookDirection), XMLoadFloat3(&m_upDirection));
}

XMMATRIX Camera::GetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	return XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
//	return XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
}

void Camera::OnKeyDown(WPARAM key)
{
	switch (key)
	{
		case 'W':
			m_keysPressed.w = true;
			break;
		case 'A':
			m_keysPressed.a = true;
			break;
		case 'S':
			m_keysPressed.s = true;
			break;
		case 'D':
			m_keysPressed.d = true;
			break;
		case VK_LEFT:
			m_keysPressed.left = true;
			break;
		case VK_RIGHT:
			m_keysPressed.right = true;
			break;
		case VK_UP:
			m_keysPressed.up = true;
			break;
		case VK_DOWN:
			m_keysPressed.down = true;
			break;
		case VK_ESCAPE:
			Reset();
			break;
	}
}

void Camera::OnKeyUp(WPARAM key)
{
	switch (key)
	{
		case 'W':
			m_keysPressed.w = false;
			break;
		case 'A':
			m_keysPressed.a = false;
			break;
		case 'S':
			m_keysPressed.s = false;
			break;
		case 'D':
			m_keysPressed.d = false;
			break;
		case VK_LEFT:
			m_keysPressed.left = false;
			break;
		case VK_RIGHT:
			m_keysPressed.right = false;
			break;
		case VK_UP:
			m_keysPressed.up = false;
			break;
		case VK_DOWN:
			m_keysPressed.down = false;
			break;
	}
}

#endif
