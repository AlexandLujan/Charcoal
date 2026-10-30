#pragma once

//#include "CCameraControlDlg.h"
//#include "Builder.h"

#include <afxwin.h>
#include <cstdint>
#include <DirectXMath.h>
#include <DirectXMathMisc.inl>
#include <DirectXMathVector.inl>
#include <map>
#include <memory>
#include <string>
#include <windows.h>

#define CAMERA_SELECTED 1
#define CAMERA_ZOOM_CHANGED 2
//#define CAMERA_SCALE_FACTOR 1000.0f
//#define CAMERA_SCALE_FACTOR 10.0f
#define CAMERA_SCALE_FACTOR 1.0f
#define CAMERA_ZOOM_FACTOR  1.0f

using namespace DirectX;
using namespace std;
// When performing transformations on the camera, 
// it is sometimes useful to express which space this 
// transformation should be applied.
enum class Space
{
	Local,
	World,
};
class Camera
{
public:

	Camera(string camera_name = "Default Camera");
	virtual ~Camera();

	void XM_CALLCONV set_LookAt(DirectX::FXMVECTOR eye, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up);
	DirectX::XMMATRIX get_ViewMatrix() const;
	DirectX::XMMATRIX get_InverseViewMatrix() const;
	void Update(UINT64 frameCount);
	/**
	 * Set the camera to a perspective projection matrix.
	 * @param fovy The vertical field of view in degrees.
	 * @param aspect The aspect ratio of the screen.
	 * @param zNear The distance to the near clipping plane.
	 * @param zFar The distance to the far clipping plane.
	 */
	void set_Projection(float fovy, float aspect, float zNear, float zFar);
	DirectX::XMMATRIX get_ProjectionMatrix() const;
	DirectX::XMMATRIX get_InverseProjectionMatrix() const;

	/**
	 * Set the field of view in degrees.
	 */
	void set_FoV(float fovy);

	/**
	 * Get the field of view in degrees.
	 */
	float get_FoV() const;

	/**
	 * Set the camera's position in world-space.
	 */
	void XM_CALLCONV set_Translation(DirectX::FXMVECTOR translation);
	DirectX::XMVECTOR get_Translation() const;

	/**
	 * Set the focal point for the camera (in world-space).
	 */
	void XM_CALLCONV set_FocalPoint(DirectX::FXMVECTOR focalPoint);
	DirectX::XMVECTOR get_FocalPoint() const;

	/**
	 * Set the camera's rotation in world-space.
	 * @param rotation The rotation quaternion.
	 */
	void XM_CALLCONV set_Rotation(DirectX::FXMVECTOR rotation);
	/**
	 * Query the camera's rotation.
	 * @returns The camera's rotation quaternion.
	 */
	DirectX::XMVECTOR get_Rotation() const;

	void XM_CALLCONV Translate(DirectX::FXMVECTOR translation, Space space = Space::Local);
	void Rotate(DirectX::FXMVECTOR quaternion);
	void XM_CALLCONV MoveFocalPoint(DirectX::FXMVECTOR focalPoint, Space space = Space::Local);

protected:
	virtual void UpdateViewMatrix() const;
	virtual void UpdateInverseViewMatrix() const;
	virtual void UpdateProjectionMatrix() const;
	virtual void UpdateInverseProjectionMatrix() const;

	// This data must be aligned otherwise the SSE intrinsics fail
	// and throw exceptions.
	__declspec(align(16)) struct AlignedData
	{
		// World-space position of the camera.
		DirectX::XMVECTOR m_Translation;
		// World-space rotation of the camera.
		// THIS IS A QUATERNION!!!!
		DirectX::XMVECTOR m_Rotation;

		// World-space position of the focus object.
		DirectX::XMVECTOR m_FocalPoint;

		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_InverseViewMatrix;
		DirectX::XMMATRIX m_ProjectionMatrix;
		DirectX::XMMATRIX m_InverseProjectionMatrix;
	};
	AlignedData* pData;

	// projection parameters
	float m_vFoV;   // Vertical field of view.
	float m_AspectRatio; // Aspect ratio
	float m_zNear;      // Near clip distance
	float m_zFar;       // Far clip distance.
	float m_Pitch = 0;
	float m_Yaw = 0;
	float m_Roll = 0;

	// True if the view matrix needs to be updated.
	mutable bool m_ViewDirty;
	mutable bool m_InverseViewDirty;
	// True if the projection matrix needs to be updated.
	mutable bool m_ProjectionDirty;
	mutable bool m_InverseProjectionDirty;
	float m_X = 0.0f;
	float m_Y = 0.0f;
	float m_Z = 0.0f;
	float m_PreviousPitch = 0.0f;
	float m_PreviousYaw = 0.0f;

public:
	string name;
	string target_object_name;
	int32_t tracking;
	// Limit rotation to pitch and yaw.
	XMFLOAT4 position = { 0,0,0,1 };
	XMFLOAT4& Position() { return position; }
	XMFLOAT4 target_location = { 0,0,0,1 };
	XMFLOAT4& Target() { return target_location; }
	XMFLOAT4 target_vector = { 0,0,0,0 };
	XMFLOAT4& TargetVector() { return target_vector; }
	UINT zoom;

	float& X() { return position.x; }
	float& Y() { return position.y; }
	float& Z() { return position.z; }
	float& TargetX() { return target_location.x; }
	float& TargetY() { return target_location.y; }
	float& TargetZ() { return target_location.z; }
	XMVECTOR CameraPositionVector()
	{
		return XMVectorSet(position.x / CAMERA_SCALE_FACTOR,
						   position.y / CAMERA_SCALE_FACTOR,
						   position.z / CAMERA_SCALE_FACTOR,
						   1.0f);
	}
	XMVECTOR TargetPositionVector()
	{
		return XMVectorSet(target_location.x / CAMERA_SCALE_FACTOR,
						   target_location.y / CAMERA_SCALE_FACTOR,
						   target_location.z / CAMERA_SCALE_FACTOR,
						   1.0f);
	}
	XMFLOAT4 CameraLocation()
	{
		return position;
	//	{
	//		(float)position.x / CAMERA_SCALE_FACTOR,
	//			 (float)position.y / CAMERA_SCALE_FACTOR,
	//			 (float)position.z / CAMERA_SCALE_FACTOR };
	}
	XMFLOAT4 TargetLocation()
	{
		return target_location;

	//	{
	//		(float)target_location.x / CAMERA_SCALE_FACTOR,
	//			 (float)target_location.y / CAMERA_SCALE_FACTOR,
	//			 (float)target_location.z / CAMERA_SCALE_FACTOR };
	}

	UINT& Zoom() { return zoom; }
	INT& Tracking() { return tracking; }
	float ZoomFloat() { return (float)zoom / CAMERA_ZOOM_FACTOR; }

	void ResetView()
	{
		// Reset previous deltas.
		m_X = 0.0f;
		m_Y = 0.0f;
		m_Z = 0.0f;
		m_PreviousPitch = 0.0f;
		m_PreviousYaw = 0.0f;
		m_Pitch = 0.0f;
		m_Yaw = 90.0f;

		XMVECTOR rotation =
			XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_Pitch), XMConvertToRadians(m_Yaw), 0.0f);
		set_Rotation(rotation);
		set_Translation({ 0, 2, -2, 1 });
		set_FocalPoint({ 0, 0, 0, 1 });
	}

private:

};


























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
#pragma once
#include <DirectXMath.h>
#include <string>
#include "utility.h"
using namespace DirectX;

class Camera
{
public:
	Camera(std::string name = "DefaultCamera");

	void Init(XMFLOAT3 position);
	void Update(float elapsedSeconds);
	XMMATRIX GetViewMatrix();
	XMMATRIX GetProjectionMatrix(float fov, float aspectRatio, float nearPlane = 1.0f, float farPlane = 1000.0f);
	void SetMoveSpeed(float unitsPerSecond);
	void SetTurnSpeed(float radiansPerSecond);

	void OnKeyDown(WPARAM key);
	void OnKeyUp(WPARAM key);
private:
	void Reset();

	struct KeysPressed
	{
		bool w;
		bool a;
		bool s;
		bool d;

		bool left;
		bool right;
		bool up;
		bool down;
	};

	XMFLOAT3 m_initialPosition;
	XMFLOAT3 m_position;
	float m_yaw;                // Relative to the +z axis.
	float m_pitch;                // Relative to the xz plane.
	XMFLOAT3 m_lookDirection;
	XMFLOAT3 m_upDirection;
	float m_moveSpeed;            // Speed at which the camera moves, in units per second.
	float m_turnSpeed;            // Speed at which the camera turns, in radians per second.

	KeysPressed m_keysPressed;
	// things that i have added
	std::string m_name;
	XMFLOAT3 m_target;
public:
	std::string Name() { return m_name; }
	XMFLOAT3& Position() { return m_position; }
	XMFLOAT3& Target() { return m_target; }
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 look, XMFLOAT3 up)
	{
		m_position = eye;
		m_target = look;
		m_lookDirection = VECTOR(look, eye);
		m_upDirection = up;
	}
};
#endif