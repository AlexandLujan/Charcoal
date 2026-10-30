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
#include <DirectXMathMisc.inl>
#include <DirectXMathVector.inl>
#include <malloc.h>
#include <string>
#include "Camera.h"
#include "Utility.h"

using namespace std;
using namespace DirectX;

Camera::Camera(string name) :
	CameraName(name),
	m_initialPosition(0, 0, 0),
	m_position(m_initialPosition),
	m_initialTarget(0, 0, 0),
	m_target(m_initialTarget),
	m_Yaw(XM_PI),
	m_Pitch(0.0f),
	m_Roll(0.0f),
	m_lookDirection(0, 0, -1),
	m_upDirection(0, 1, 0),
	m_moveSpeed(20.0f),
	m_turnSpeed(XM_PIDIV2)
{
	pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
	pData->m_Translation = XMVectorZero();
	pData->m_Rotation = XMQuaternionIdentity();
	pData->m_FocalPoint = XMVectorZero();
}
Camera::~Camera()
{
	_aligned_free(pData);
}
void Camera::Init(XMFLOAT3 position)
{
	m_initialPosition = position;
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
	m_Yaw = XM_PI;
	m_Pitch = 0.0f;
	m_Roll = 0.0f;
	m_lookDirection = { 0, 0, -1 };
}

void XM_CALLCONV Camera::SetLookAt(XMFLOAT3 eyeBall, XMFLOAT3 look_here, XMVECTOR up)
{
	StartPosition() = eyeBall;
	Position() = eyeBall;
	StartTarget() = look_here;
	Target() = look_here;
	m_upDirection = up;
	m_lookDirection = VECTOR(look_here, eyeBall);
	XMVECTOR eye = XMVectorSet(eyeBall.x, eyeBall.y, eyeBall.z, 1.0f);
	XMVECTOR target = XMVectorSet(look_here.x, look_here.y, look_here.z, 1.0f);
	pData->m_ViewMatrix = XMMatrixLookAtRH(eye, target, up);
	pData->m_Translation = eye;
	pData->m_Rotation = XMQuaternionRotationMatrix(XMMatrixTranspose(pData->m_ViewMatrix));

	m_InverseViewDirty = true;
	m_ViewDirty = false;
}

void Camera::Update(float elapsedSeconds)
{}

XMMATRIX Camera::GetViewMatrix()
{
	if (m_ViewDirty)
	{
		UpdateViewMatrix();
	}
	return pData->m_ViewMatrix;

//	return XMMatrixLookToRH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookDirection), m_upDirection);
}

XMMATRIX Camera::GetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	set_Projection(fov, aspectRatio, nearPlane, farPlane);
	UpdateProjectionMatrix();
	return pData->m_ProjectionMatrix;
//	return XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
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
