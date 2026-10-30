#pragma once
#include <afx.h>
#include <cstdint>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <Windows.h>
#include "DXImage.h"
#include "Material.h"
#include "Object.h"
#include "Scripting.h"
#include "struct.h"
#include "Utility.h"

using namespace DirectX;

//extern std::map<std::string, std::shared_ptr<Assembly>> DXImage::ObjectList;
class Actor;
class Object;

class Actor
{
protected:

	std::string objectName;
	std::shared_ptr<Object> assembly;
	XMFLOAT3 position;
	bool is_visible = true;
	bool is_dirty = false;
	std::string name;
	float pitch = 0.0f; // Rotate about X Axis radians
	float yaw = 0.0f;   // Rotate about Y Axis radians
	float roll = 0.0f;  // Rotate about Z Axis radians
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleZ = 1.0f;
	MaterialProperties material;
	XMMATRIX worldMatrix;
	std::vector<InstanceData> instanceData = {};

public:
	Actor(std::string name, std::string objectName, XMFLOAT3 position = { 0,0,0 }, XMFLOAT3 rotation = { 0,0,0 }, XMFLOAT3 scale = { 1, 1, 1 }, std::string materialname = "ZERO");
	std::string& ObjectName() { return objectName; }
	XMFLOAT3& Position() { return position; }
	std::string& Name() { return name; }
	float& Pitch() { return pitch; }
	float& Yaw() { return yaw; }
	float& Roll() { return roll; }
	float& X() { return position.x; }
	float& Y() { return position.y; }
	float& Z() { return position.z; }
	virtual void Update(uint64_t frameNumber, BOOL handleTransparencies);
	void Clear();
	bool& IsVisible() { return is_visible; }
	bool& IsDirty() { return is_dirty; }
	std::shared_ptr<Object> GetAssembly();
	std::vector<InstanceData>& GetInstanceData() { return instanceData; }
	virtual XMMATRIX AdjustMesh(uint64_t frameNumber, std::string key) = 0;
	virtual void HandleAction(ScriptActionCode action, std::string parameter) = 0;

	void MoveTo(float x, float y, float z)
	{
		position.x = x;
		position.y = y;
		position.z = z;
		UpdateWorld();
	}
	void MoveBy(float dx, float dy, float dz)
	{
		position.x += dx;
		position.y += dy;
		position.z += dz;
		UpdateWorld();
	}
	void RotateTo(float _pitch, float _yaw, float _roll) //degrees
	{
		pitch = DegreesToRadians(-_pitch);
		yaw = DegreesToRadians(-_yaw);
		roll = DegreesToRadians(_roll);
		UpdateWorld();
	}
	void RotateBy(float deltaPitch, float deltaYaw, float deltaRoll)
	{
		pitch += DegreesToRadians(-deltaPitch);
		yaw += DegreesToRadians(-deltaYaw);
		roll += DegreesToRadians(deltaRoll);
		UpdateWorld();
	}
	void PitchTo(float _pitch)
	{
		pitch = DegreesToRadians(-_pitch);
		UpdateWorld();
	}
	void YawTo(float _yaw)
	{
		yaw = DegreesToRadians(-_yaw);
		UpdateWorld();
	}
	void RollTo(float _roll)
	{
		roll = DegreesToRadians(_roll);
		UpdateWorld();
	}
	void PitchBy(float _pitch)
	{
		pitch += DegreesToRadians(-_pitch);
		UpdateWorld();
	}
	void YawBy(float _yaw)
	{
		yaw += DegreesToRadians(-_yaw);
		UpdateWorld();
	}
	void RollBy(float _roll)
	{
		roll += DegreesToRadians(_roll);
		UpdateWorld();
	}
	void ScaleTo(float sX, float sY, float sZ)
	{
		scaleX = sX;
		scaleY = sY;
		scaleZ = sZ;
		UpdateWorld();
	}
	void ScaleBy(float dScaleX, float dScaleY, float dScaleZ)
	{
		scaleX += dScaleX;
		scaleY += dScaleY;
		scaleZ += dScaleZ;
		UpdateWorld();
	}
//	virtual void Erase() = 0;
	XMMATRIX& World() { return worldMatrix; }
	void UpdateWorld()
	{
		XMMATRIX matScale = DirectX::XMMatrixScaling(scaleX, scaleY, scaleZ);
		XMMATRIX matRotateX = DirectX::XMMatrixRotationX(pitch);
		XMMATRIX matRotateY = DirectX::XMMatrixRotationY(yaw);
		XMMATRIX matRotateZ = DirectX::XMMatrixRotationZ(roll);
		XMMATRIX matTranslate = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		worldMatrix = matScale * matRotateX * matRotateY * matRotateZ * matTranslate;
	}

};
#define FLAPS_UP 0
#define FLAPS_DOWN 1
#define AILERONS_NEUTRAL 0
#define AILERONS_LEFT_BANK 1
#define AILERONS_RIGHT_BANK 2
#define RUDDER_NEUTRAL 0
#define RUDDER_LEFT 1
#define RUDDER_RIGHT 2
#define ELEVATOR_NEUTRAL 0
#define ELEVATOR_CLIMB 1
#define ELEVATOR_DIVE 2
#define AIRBRAKES_RETRACTED 0
#define	AIRBRAKES_EXTENDED 1
#define CANOPY_CLOSED 0
#define	CANOPY_OPEN 1
#define LANDING_GEAR_RETRACTED						0
#define LANDING_GEAR_EXTENDING_INNER_DOORS_OPEN		1
#define LANDING_GEAR_EXTENDING						2
#define LANDING_GEAR_EXTENDING_INNER_DOORS_CLOSE	3
#define LANDING_GEAR_EXTENDED						4
#define LANDING_GEAR_RETRACTING_INNER_DOORS_OPEN	5
#define LANDING_GEAR_RETRACTING						6
#define LANDING_GEAR_RETRACTING_INNER_DOORS_CLOSE	7


struct SurfaceStatus
{
	UINT phase = 0;
	FLOAT currentAngle = 0.0f;
	FLOAT targetAngle = 0.0f;
	FLOAT maxAngle = 0.0f;
	FLOAT minAngle = 0.0f;
	FLOAT deltaAngle = 1.0f;
};
enum ControlSurfaces
{
	NoControl = 0,
	RightFlap,
	LeftFlap,
	RightAileron,
	LeftAileron,
	NoseGearDoorInner,
	NoseGearDoorOuter,
	NoseGearStrutH,
	NoseGearStrutB,
	NoseGearWheel,
	RightGearDoorInner,
	RightGearDoorOuter,
	RightGearStrutH,
	RightGearStrutB,
	RightGearWheel,
	LeftGearDoorInner,
	LeftGearDoorOuter,
	LeftGearStrutH,
	LeftGearStrutB,
	LeftGearWheel,
	ElevatorRight,
	ElevatorLeft,
	Rudder,
	SpeedBrakeRight,
	SpeedBrakeLeft,
	FrontCanopy,
	RearCanopy,
	FrontCanopyGlass,
	RearCanopyGlass,
	FrontCanopyShield,
	RearCanopyShield,
	FrontCanopyFrame,
	RearCanopyFrame,
};
#define SURFACE_MOVEMENT_STEP 1.0f
class Aircraft : public Actor
{
// clang-format off	
public:
	SurfaceStatus /*          */flaps = { FLAPS_UP,               0,  0, 60,  0,SURFACE_MOVEMENT_STEP }; // flaps up
	SurfaceStatus /*       */ailerons = { AILERONS_NEUTRAL,       0,  0, 15,-15,SURFACE_MOVEMENT_STEP }; // ailerons neutral
	SurfaceStatus /*       */elevator = { ELEVATOR_NEUTRAL,       0,  0, 15,-15,SURFACE_MOVEMENT_STEP }; // elevator neutral
	SurfaceStatus /*         */rudder = { RUDDER_NEUTRAL,         0,  0, 20,-20,SURFACE_MOVEMENT_STEP }; // rudder neutral
	SurfaceStatus /*    */frontCanopy = { CANOPY_CLOSED,          0,  0, 60,  0,SURFACE_MOVEMENT_STEP }; // canopy closed
	SurfaceStatus /*     */rearCanopy = { CANOPY_CLOSED,          0,  0, 60,  0,SURFACE_MOVEMENT_STEP }; // canopy closed
	SurfaceStatus /*    */speedBrakes = { AIRBRAKES_RETRACTED,    0,  0, 90,  0,SURFACE_MOVEMENT_STEP }; // airbrakes closed
	SurfaceStatus /*    */landingGear = { LANDING_GEAR_EXTENDED,  0,  0, 90,  0,SURFACE_MOVEMENT_STEP }; // landing gear extended
	SurfaceStatus /**/landingGearDoor = { LANDING_GEAR_EXTENDED,  0,  0, 90,  0,SURFACE_MOVEMENT_STEP }; // landing gear inner door closed
// clang-format on	
	bool BoundsFound = false;
	// name, pair of base coordinate and base rotation
	std::unordered_map<ControlSurfaces, std::pair<XMFLOAT3, XMFLOAT3>> ControlBounds;
	std::unordered_map<std::string, ControlSurfaces> SurfacePairs;

	Aircraft(std::string name, std::string objectName) : Actor(name, objectName)
	{
		ControlBounds.clear();
		SurfacePairs.clear();
	}
	~Aircraft() {}
	void GatherStats();
	virtual XMMATRIX AdjustMesh(uint64_t frameNumber, std::string key) override;
	virtual void HandleAction(ScriptActionCode action, std::string parameter) override;
	bool isControlSurface(std::string name)
	{
		return SurfacePairs.contains(name);
	}
	ControlSurfaces ControlSurfaceKey(std::string name)
	{
		if (SurfacePairs.contains(name)) return SurfacePairs[name];
		return ControlSurfaces::NoControl;
	}

	virtual void GearUp() {}
	virtual void GearDown() {}
	virtual void FlapsUp() {}
	virtual void FlapsDown(FLOAT angle) {}
	virtual void BankLeft(FLOAT angle) {}
	virtual void BankRight(FLOAT angle) {}
	virtual void RudderLeft(FLOAT angle) {}
	virtual void RudderRight(FLOAT angle) {}
	virtual void ElevatorClimb(FLOAT angle) {}
	virtual void ElevatorDive(FLOAT angle) {}
	virtual void FrontCanopyOpen() {}
	virtual void RearCanopyOpen() {}
	virtual void FrontCanopyClose() {}
	virtual void RearCanopyClose() {}
	virtual void SpeedBrakeExtend() {}
	virtual void SpeedBrakeRetract() {}
	virtual XMMATRIX MoveLeftFlap() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveRightFlap() { return XMMatrixIdentity(); }

	virtual XMMATRIX MoveRightAileron(float	targetAngle) { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveLeftAileron(float targetAngle) { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveRudder(float targetAngle) { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveElevator(float targetAngle) { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveFrontCanopy() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveRearCanopy() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveFrontCanopyFrame() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveRearCanopyFrame() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveRightSpeedBrake() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveLeftSpeedBrake() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveRightLandingGearStrut() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveRightLandingGear() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveRightLandingGearDoor() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveLeftLandingGear() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveLeftLandingGearDoor() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveNoseLandingGearStrut() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveNoseLandingGear() { return XMMatrixIdentity(); }
	virtual XMMATRIX MoveNoseLandingGearDoor() { return XMMatrixIdentity(); }
};

class T38 : public Aircraft
{
public:
	T38(std::string name);
	~T38() {}
	void GearUp();
	void GearDown() override;
	void FlapsUp() override;
	void FlapsDown(FLOAT angle) override;
	void BankLeft(FLOAT angle) override;
	void BankRight(FLOAT angle) override;
	void RudderLeft(FLOAT angle) override;
	void RudderRight(FLOAT angle) override;
	void ElevatorClimb(FLOAT angle) override;
	void ElevatorDive(FLOAT angle) override;
	void FrontCanopyOpen() override;
	void RearCanopyOpen() override;
	void FrontCanopyClose() override;
	void RearCanopyClose() override;
	void SpeedBrakeExtend() override;
	void SpeedBrakeRetract() override;
	XMMATRIX MoveLeftFlap() override;
	XMMATRIX MoveRightFlap() override;

	XMMATRIX MoveRightAileron(float	targetAngle) override;
	XMMATRIX MoveLeftAileron(float targetAngle) override;
	XMMATRIX MoveRudder(float targetAngle) override;
	XMMATRIX MoveElevator(float targetAngle) override;
	XMMATRIX MoveFrontCanopy() override;
	XMMATRIX MoveRearCanopy() override;
	XMMATRIX MoveFrontCanopyFrame() override;
	XMMATRIX MoveRearCanopyFrame() override;
	XMMATRIX MoveRightSpeedBrake() override;
	XMMATRIX MoveLeftSpeedBrake() override;
	XMMATRIX MoveRightLandingGearStrut() override;
	XMMATRIX MoveRightLandingGear() override;
	XMMATRIX MoveRightLandingGearDoor() override;
	XMMATRIX MoveLeftLandingGear() override;
	XMMATRIX MoveLeftLandingGearDoor() override;
	XMMATRIX MoveNoseLandingGearStrut() override;
	XMMATRIX MoveNoseLandingGear() override;
	XMMATRIX MoveNoseLandingGearDoor() override;
};

class C130 : public Aircraft
{
public:
	C130(std::string name);
	~C130() {}
};


/*
"Glass",
"Glass001",
"Glass002",
"Interior",
"Main_Body",
"Main_Body001",
"Main_Body002",
"Wings",
"Wings001",
"Wings002",
"Wings003",
"Wings004",
"Wings005",
"Wings006",
"Wings007",
"Wings008",
"Wings009",
"Wings010",
"Wings011",
"Wings012",
"Wings013",
"Wings014",
"Wings015",
"Wings016",
"Wings017"

*/

