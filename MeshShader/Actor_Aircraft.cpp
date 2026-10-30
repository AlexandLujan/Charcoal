#include "stdafx.h"
#include <afxstr.h>
#include <cstdint>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <Windows.h>
#include "Actor.h"
#include "Object.h"
#include "Scripting.h"
#include "struct.h"
#include "Utility.h"

using namespace DirectX;
using namespace std;

#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define max(a,b)            (((a) > (b)) ? (a) : (b))


void Aircraft::GatherStats()
{
	CString st;
	shared_ptr<Object> obj = GetAssembly();
	for (auto& [key, info] : SurfacePairs)
	{
		MeshStatistics ms = obj->MeshDirectory()[key];
		ControlBounds[info].first = ms.meshMin;
		ControlBounds[info].second = ms.meshMax;

		//st.Format(_T(" key = %32s   min = (%11.6f,%11.6f,%11.6f)  max = (%11.6f,%11.6f,%11.6f)\r\n"),
		//		  key.c_str(),
		//		  ms.meshMin.x,
		//		  ms.meshMin.y,
		//		  ms.meshMin.z,
		//		  ms.meshMax.x,
		//		  ms.meshMax.y,
		//		  ms.meshMax.z);
		//OutputDebugString(st);
	}
}

XMMATRIX Aircraft::AdjustMesh(uint64_t frameNumber, std::string key)
{
	XMMATRIX transform = XMMatrixIdentity();
	if (not BoundsFound)
	{
		GatherStats();
		BoundsFound = true;
	}
	// generate the translation matrix for mesh for movement in object space
	// put the result into the instance structure
//	for (auto& [key, meshletLocator] : assembly->MeshDirectory())
	if (true)
	{
		// check to see if this is a control surface
		// move surface to current rotation
		// generate meshlets for surface
		ControlSurfaces controlKey = ControlSurfaceKey(key);
//		MeshStatistics stats = SurfacePairs[key].second;
		// this is a control surface, move to current rotation and generate meshlets for surface
		switch (controlKey)
		{
			case NoControl:
				break;
			case RightFlap:
				transform = MoveRightFlap();
				break;
			case LeftFlap:
				transform = MoveLeftFlap();
				break;
			case RightAileron:
				transform = MoveRightAileron(ailerons.currentAngle);
				break;
			case LeftAileron:
				transform = MoveLeftAileron(ailerons.currentAngle);
				break;
			case NoseGearDoorInner:
				transform = MoveNoseLandingGearDoor();
				break;
			case NoseGearStrutH:
				transform = MoveNoseLandingGearStrut();
				break;
			case NoseGearStrutB:
			case NoseGearWheel:
			case NoseGearDoorOuter:
				transform = MoveNoseLandingGear();
				break;
			case RightGearDoorInner:
				transform = MoveRightLandingGearDoor();
				break;
			case RightGearStrutH:
				transform = MoveRightLandingGearStrut();
				break;
			case RightGearDoorOuter:
			case RightGearStrutB:
			case RightGearWheel:
				transform = MoveRightLandingGear();
				break;
			case LeftGearDoorInner:
				transform = MoveLeftLandingGearDoor();
				break;
			case LeftGearDoorOuter:
			case LeftGearStrutH:
			case LeftGearStrutB:
			case LeftGearWheel:
				transform = MoveLeftLandingGear();
				break;
			case ElevatorRight:
			case ElevatorLeft:
			case Rudder:
				break;
			case SpeedBrakeRight:
				transform = MoveRightSpeedBrake();
				break;
			case SpeedBrakeLeft:
				transform = MoveLeftSpeedBrake();
				break;
			case FrontCanopyShield:
			case FrontCanopyGlass:
			case FrontCanopyFrame:
				transform = MoveFrontCanopy();
				break;
			case FrontCanopy:
				transform = MoveFrontCanopyFrame();
				break;
			case RearCanopyGlass:
			case RearCanopyShield:
			case RearCanopyFrame:
				transform = MoveRearCanopy();
				break;
			case RearCanopy:
				transform = MoveRearCanopyFrame();
				break;
		}
	}
	return transform;
}
void Aircraft::HandleAction(ScriptActionCode action, string parameter)
{
	switch (action)
	{
		case NoAction:
			break;
		case Action_GearUp:
			GearUp();
			break;
		case Action_GearDown:
			GearDown();
			break;
		case Action_FlapsUp:
			FlapsUp();
			break;
		case Action_FlapsDown:
			FlapsDown(stof(parameter));
			break;
		case Action_BankLeft:
			BankLeft(stof(parameter));
			break;
		case Action_BankRight:
			BankRight(stof(parameter));
			break;
		case Action_RudderLeft:
			RudderLeft(stof(parameter));
			break;
		case Action_RudderRight:
			RudderRight(stof(parameter));
			break;
		case Action_ElevatorClimb:
			ElevatorClimb(stof(parameter));
			break;
		case Action_ElevatorDive:
			ElevatorDive(stof(parameter));
			break;
		case Action_FrontCanopyOpen:
			FrontCanopyOpen();
			break;
		case Action_FrontCanopyClose:
			FrontCanopyClose();
			break;
		case Action_RearCanopyOpen:
			RearCanopyOpen();
			break;
		case Action_RearCanopyClose:
			RearCanopyClose();
			break;
		case Action_SpeedBrakeExtend:
			SpeedBrakeExtend();
			break;
		case Action_SpeedBrakeRetract:
			SpeedBrakeRetract();
			break;
	}
}

