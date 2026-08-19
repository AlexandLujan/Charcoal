#pragma once

//#include "CCameraControlDlg.h"
//#include "Builder.h"

#include <cmath>
#include <DirectXMath.h>
#include <string>
#include <vector>

#define CAMERA_SELECTED 1
#define CAMERA_ZOOM_CHANGED 2
//#define CAMERA_SCALE_FACTOR 1000.0f
#define CAMERA_SCALE_FACTOR 10.0f
#define CAMERA_ZOOM_FACTOR  2.0f

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
	XMINT3 position = { 0,0,0 };
	XMINT3& Position() { return position; }
	XMINT3 target_location = { 0,0,0 };
	XMINT3& Target() { return target_location; }
	XMFLOAT3 target_vector = { 0,0,0 };
	UINT zoom;

	INT& X() { return position.x; }
	INT& Y() { return position.y; }
	INT& Z() { return position.z; }
	INT& TargetX() { return target_location.x; }
	INT& TargetY() { return target_location.y; }
	INT& TargetZ() { return target_location.z; }
	XMVECTOR CameraPositionVector()
	{
		return XMVectorSet((float)position.x / CAMERA_SCALE_FACTOR,
						   (float)position.y / CAMERA_SCALE_FACTOR,
						   (float)position.z / CAMERA_SCALE_FACTOR,
						   1.0f);
	}
	XMVECTOR TargetPositionVector()
	{
		return XMVectorSet((float)target_location.x / CAMERA_SCALE_FACTOR,
						   (float)target_location.y / CAMERA_SCALE_FACTOR,
						   (float)target_location.z / CAMERA_SCALE_FACTOR,
						   1.0f);
	}
	XMFLOAT3 CameraLocation()
	{
		return { (float)position.x / CAMERA_SCALE_FACTOR,
				 (float)position.y / CAMERA_SCALE_FACTOR,
				 (float)position.z / CAMERA_SCALE_FACTOR };
	}
	XMFLOAT3 TargetLocation()
	{
		return { (float)target_location.x / CAMERA_SCALE_FACTOR,
				 (float)target_location.y / CAMERA_SCALE_FACTOR,
				 (float)target_location.z / CAMERA_SCALE_FACTOR };
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

extern map<string, std::shared_ptr<Camera>> CameraList;
