#include "stdafx.h"
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


T38::T38(string name = "") : Aircraft(name, "t38")
{
	ControlBounds =
	{
		{RightAileron,{{0,0,0},{0,0,0}}},
		{LeftAileron,{{0,0,0},{0,0,0}}},
		{ElevatorRight,{{0,0,0},{0,0,0}}},
		{ElevatorLeft,{{0,0,0},{0,0,0}}},
		{NoseGearStrutB,{{0,0,0},{0,0,0}}},
		{NoseGearStrutH,{{0,0,0},{0,0,0}}}, // pi
		{RightGearStrutB,{{0,0,0},{0,0,0}}},
		{RightGearStrutH,{{0,0,0},{0,0,0}}},
		{LeftGearStrutB,{{0,0,0},{0,0,0}}},
		{LeftGearStrutH,{{0,0,0},{0,0,0}}},
		{FrontCanopy,{{0,0,0},{0,0,0}}},
		{RearCanopy,{{0,0,0},{0,0,0}}},
		{Rudder,{{0,0,0},{0,0,0}}},
		{FrontCanopyFrame,{{0,0,0},{0,0,0}}},
		{RearCanopyFrame,{{0,0,0},{0,0,0}}},
		{NoseGearDoorInner,{{0,0,0},{0,0,0}}},
		{NoseGearDoorOuter,{{0,0,0},{0,0,0}}},
		{RightGearDoorInner,{{0,0,0},{0,0,0}}},
		{RightGearDoorOuter,{{0,0,0},{0,0,0}}},
		{LeftGearDoorInner,{{0,0,0},{0,0,0}}},
		{LeftGearDoorOuter,{{0,0,0},{0,0,0}}},
		{NoseGearWheel,{{0,0,0},{0,0,0}}},
		{RightGearWheel,{{0,0,0},{0,0,0}}},
		{LeftGearWheel,{{0,0,0},{0,0,0}}},
		{SpeedBrakeRight,{{0,0,0},{0,0,0}}},
		{SpeedBrakeLeft,{{0,0,0},{0,0,0}}},
		{FrontCanopyGlass,{{0,0,0},{0,0,0}}},
		{RearCanopyGlass,{{0,0,0},{0,0,0}}},
		{FrontCanopyShield,{{0,0,0},{0,0,0}}},
		{RearCanopyShield,{{0,0,0},{0,0,0}}},
		{RightFlap,{{0,0,0},{0,0,0}}},
		{LeftFlap,{{0,0,0},{0,0,0}}},
	};

	// map of meshes of T38 with type of control surface and meshlet index
	SurfacePairs =
	{
		//{"antennes_antennes.mesh",					{NoControl, 0}			},
		//{"cover1",									{NoControl, 0}			},
		//{"cover2",									{NoControl, 0}			},
		//{"beacon_beacon.mesh",						{NoControl, 0}			},
		//{"echappes_echappes.mesh",					{NoControl, 0}			},
		//{"fuselage_fuselage.mesh",					{NoControl, 0}			},
		//{"inttourvitres_inttourvitres.mesh",		{NoControl, 0}			},
		//{"intvitrecanopyA_intvitrecanopyA.mesh",	{NoControl, 0}			},
		//{"intvitrecanopyB_intvitrecanopyB.mesh",	{NoControl, 0}			},
		//{"intvitres_intvitres.mesh",				{NoControl, 0}			},
		//{"planchet_planchet.mesh",					{NoControl, 0}			},
		//{"seat.001_seat.mesh.001",					{NoControl, 0}			},
		//{"seat_seat.mesh",							{NoControl, 0}			},
		//{"sides_sides.mesh",						{NoControl, 0}			},
		//{"slatD_slatD.mesh",						{NoControl, 0}			},
		//{"slatG_slatG.mesh",						{NoControl, 0}			},
		//{"support_support.mesh",					{NoControl, 0}			},
		//{"tanks_tanks.mesh",						{NoControl, 0}			},
		//{"trous_trous.mesh",						{NoControl, 0}			},
		//{"vitres_vitres.mesh",						{NoControl, 0}			},
		{"aileronD_aileronD.mesh",					RightAileron},
		{"aileronG_aileronG.mesh",					LeftAileron},
		{"direction_direction.mesh",				Rudder},
		{"ailes2_ailes2.mesh",						ElevatorRight},
		{"ailes_ailes.mesh",						ElevatorLeft},

		{"axeAB_axeAB.mesh",						NoseGearStrutB},
		{"axeAH_axeAH.mesh",						NoseGearStrutH},
		{"axeDB_axeDB.mesh",						RightGearStrutB},
		{"axeDH_axeDH.mesh",						RightGearStrutH},
		{"axeGB_axeGB.mesh",						LeftGearStrutB},
		{"axeGH_axeGH.mesh",						LeftGearStrutH},

		{"canopyA_canopyA.mesh",					FrontCanopy},
		{"canopyB_canopyB.mesh",					RearCanopy},
		{"intcanopyA_intcanopyA.mesh",				FrontCanopyFrame},
		{"intcanopyB_intcanopyB.mesh",				RearCanopyFrame},
		{"intvitrecanopyA_intvitrecanopyA.mesh",	FrontCanopyShield},
		{"intvitrecanopyB_intvitrecanopyB.mesh",	RearCanopyShield},
		{"vitrecanopyA_vitrecanopyA.mesh",			FrontCanopyGlass},
		{"vitrecanopyB_vitrecanopyB.mesh",			RearCanopyGlass},

		{"porteA1_porteA1.mesh",					NoseGearDoorOuter},
		{"porteA2_porteA2.mesh",					NoseGearDoorInner},
		{"porteD1_porteD1.mesh",					RightGearDoorInner},
		{"porteD2_porteD2.mesh",					RightGearDoorOuter},
		{"porteG1_porteG1.mesh",					LeftGearDoorInner},
		{"porteG2_porteG2.mesh",					LeftGearDoorOuter},
		{"roueA_roueA.mesh",						NoseGearWheel},
		{"roueD_roueD.mesh",						RightGearWheel},
		{"roueG_roueG.mesh",						LeftGearWheel},
		{"spoilerD_spoilerD.mesh",					SpeedBrakeRight},
		{"spoilerG_spoilerG.mesh",					SpeedBrakeLeft},
		{"voletD_voletD.mesh",						RightFlap},
		{"voletG_voletG.mesh",						LeftFlap},
	};


}


#include <afxstr.h>
#include <cstringt.h>

void T38::GearUp()
{
	landingGear.phase = LANDING_GEAR_RETRACTING_INNER_DOORS_OPEN;
	landingGearDoor.targetAngle = 90.0f;
	landingGear.targetAngle = 90.0f;
}
void T38::GearDown()
{
	landingGear.phase = LANDING_GEAR_EXTENDING_INNER_DOORS_OPEN;
	landingGearDoor.targetAngle = 90.0f;
	landingGear.targetAngle = 0.0f;
}
void T38::FlapsUp()
{
	flaps.phase = FLAPS_UP;
	flaps.targetAngle = 0.0f;
}
void T38::FlapsDown(FLOAT angle)
{
	flaps.phase = FLAPS_DOWN;
	float flapPercent = angle / 100.0f;
	flaps.targetAngle = 60.0f * flapPercent; // max(flaps.minAngle, min(flaps.maxAngle, angle));
}
void T38::BankLeft(FLOAT angle)
{
	ailerons.phase = AILERONS_LEFT_BANK;
	ailerons.targetAngle = max(ailerons.minAngle, min(ailerons.maxAngle, angle));
}
void T38::BankRight(FLOAT angle)
{
	ailerons.phase = AILERONS_RIGHT_BANK;
	ailerons.targetAngle = max(ailerons.minAngle, min(ailerons.maxAngle, angle));
}
void T38::RudderLeft(FLOAT angle)
{
	rudder.phase = RUDDER_LEFT;
	rudder.targetAngle = max(rudder.minAngle, min(rudder.maxAngle, angle));
}
void T38::RudderRight(FLOAT angle)
{
	rudder.phase = RUDDER_RIGHT;
	rudder.targetAngle = max(rudder.minAngle, min(rudder.maxAngle, angle));
}
void T38::ElevatorClimb(FLOAT angle)
{
	elevator.phase = ELEVATOR_CLIMB;
	elevator.targetAngle = max(elevator.minAngle, min(elevator.maxAngle, angle));
}
void T38::ElevatorDive(FLOAT angle)
{
	elevator.phase = ELEVATOR_DIVE;
	elevator.targetAngle = max(elevator.minAngle, min(elevator.maxAngle, angle));
}
void T38::FrontCanopyOpen()
{
	frontCanopy.phase = CANOPY_OPEN;
	frontCanopy.targetAngle = 45;
}
void T38::FrontCanopyClose()
{
	frontCanopy.phase = CANOPY_CLOSED;
	frontCanopy.targetAngle = 0;
}
void T38::RearCanopyOpen()
{
	rearCanopy.phase = CANOPY_OPEN;
	rearCanopy.targetAngle = 45;
}
void T38::RearCanopyClose()
{
	rearCanopy.phase = CANOPY_CLOSED;
	rearCanopy.targetAngle = 0;
}
void T38::SpeedBrakeExtend()
{
	speedBrakes.phase = AIRBRAKES_EXTENDED;
	speedBrakes.targetAngle = 90;
}
void T38::SpeedBrakeRetract()
{
	speedBrakes.phase = AIRBRAKES_RETRACTED;
	speedBrakes.targetAngle = 0;
}

//XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[xxx].second.x,  -ControlBounds[xxx].second.y, -ControlBounds[xxx].second.z);
//XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[xxx].second.x,ControlBounds[xxx].second.y,  ControlBounds[xxx].second.z);
XMMATRIX T38::MoveRightAileron(float targetAngle)
{
	XMMATRIX result = XMMatrixIdentity();
	// pivot azis is Z at the closest point to the nose (max Z)
	// RightAileron
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[RightAileron].second.x, -ControlBounds[RightAileron].second.y, -ControlBounds[RightAileron].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[RightAileron].second.x, ControlBounds[RightAileron].second.y, ControlBounds[RightAileron].second.z);
	// Move the right aileron by delta degrees
	// Move the left aileron by oppositedelta degrees
	// Move the flaps by delta degrees
	// stop at 0 degrees and 60 degrees
	// delta is positive for down and negative for up
	// move flaps to zero, rotate to setting, move back to original position
	return result;
}
XMMATRIX T38::MoveLeftAileron(float targetAngle)
{
	XMMATRIX result = XMMatrixIdentity();
	// pivot azis is Z at the closest point to the nose (max Z)
	// LeftAileron
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[LeftAileron].second.x, -ControlBounds[LeftAileron].second.y, -ControlBounds[LeftAileron].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[LeftAileron].second.x, ControlBounds[LeftAileron].second.y, ControlBounds[LeftAileron].second.z);
	// Move the left aileron by delta degrees
	return result;
}
XMMATRIX T38::MoveRudder(float targetAngle)
{
	XMMATRIX result = XMMatrixIdentity();
	// pivot azis is Y at the closest point to the nose (max Z)
	// Rudder
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[Rudder].second.x, -ControlBounds[Rudder].second.y, -ControlBounds[Rudder].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[Rudder].second.x, ControlBounds[Rudder].second.y, ControlBounds[Rudder].second.z);
	// Move the rudder by delta degrees
	// stop at 0 degrees and 60 degrees
	// delta is positive for right and negative for left
	// move rudder to zero, rotate to setting, move back to original position
	return result;
}
XMMATRIX T38::MoveElevator(float targetAngle)
{
	XMMATRIX result = XMMatrixIdentity();
	// pivot azis is Z at the closest point to the nose (max Z)
	// Elevator
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[ElevatorRight].second.x, -ControlBounds[ElevatorRight].second.y, -ControlBounds[ElevatorRight].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[ElevatorRight].second.x, ControlBounds[ElevatorRight].second.y, ControlBounds[ElevatorRight].second.z);
	// Move the elevator by delta degrees
	// stop at 0 degrees and 60 degrees
	// delta is positive for right and negative for left
	// move rudder to zero, rotate to setting, move back to original position
	return result;
}
XMMATRIX T38::MoveFrontCanopy()
{
	XMMATRIX result = XMMatrixIdentity();
	// pivot azis is Z at the closest point to the nose (max Z)
	// FrontCanopy
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[FrontCanopy].first.x, -ControlBounds[FrontCanopy].first.y, -ControlBounds[FrontCanopy].first.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[FrontCanopy].first.x, ControlBounds[FrontCanopy].first.y, ControlBounds[FrontCanopy].first.z);
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-frontCanopy.currentAngle)) * moveToPosition;
	return result;

}
XMMATRIX T38::MoveFrontCanopyFrame()
{
	XMMATRIX result = XMMatrixIdentity();
	// pivot azis is Z at the closest point to the nose (max Z)
	// FrontCanopy
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[FrontCanopy].first.x, -ControlBounds[FrontCanopy].first.y, -ControlBounds[FrontCanopy].first.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[FrontCanopy].first.x, ControlBounds[FrontCanopy].first.y, ControlBounds[FrontCanopy].first.z);
	switch (frontCanopy.phase)
	{
		case CANOPY_CLOSED:
			// move flaps to zero, rotate to setting, move back to original position
			// identity matrix is the original position, so we can use it to move back to original position
			if (frontCanopy.currentAngle > 0.0f)
			{
				// Perform the rotation logic here
				// no need if at target angle
				// move flaps to zero, rotate to setting, move back to original position
				// base coordinates are at 0 rotation to rotate to current angle
				frontCanopy.currentAngle -= frontCanopy.deltaAngle;
				if (frontCanopy.currentAngle > 0.0f)
				{
					frontCanopy.currentAngle = 0.0f; // flaps are up so use Identity
				}
			}
			break;
		case CANOPY_OPEN:
			// move flaps to zero, rotate to setting, move back to original position
			if (frontCanopy.currentAngle < frontCanopy.targetAngle)
			{
				// step angle towards target angle
				frontCanopy.currentAngle += frontCanopy.deltaAngle;
				if (frontCanopy.currentAngle > frontCanopy.targetAngle)
				{
					frontCanopy.currentAngle = frontCanopy.targetAngle;
				}
			}
			// Perform the rotation logic here
			// move flaps to zero, rotate to setting, move back to original position
			break;
	}
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-frontCanopy.currentAngle)) * moveToPosition;
	return result;
}
XMMATRIX T38::MoveRearCanopy()
{
	XMMATRIX result = XMMatrixIdentity();
	// RearCanopy
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[RearCanopy].first.x, -ControlBounds[RearCanopy].first.y, -ControlBounds[RearCanopy].first.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[RearCanopy].first.x, ControlBounds[RearCanopy].first.y, ControlBounds[RearCanopy].first.z);
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-rearCanopy.currentAngle)) * moveToPosition;
	return result;

}
XMMATRIX T38::MoveRearCanopyFrame()
{
	XMMATRIX result = XMMatrixIdentity();
	// RearCanopy
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[RearCanopy].first.x, -ControlBounds[RearCanopy].first.y, -ControlBounds[RearCanopy].first.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[RearCanopy].first.x, ControlBounds[RearCanopy].first.y, ControlBounds[RearCanopy].first.z);
	// Move the canopy by delta degrees
	// stop at 0 degrees and 60 degrees
	// delta is positive for open and negative for close
	// move canopy to zero, rotate to setting, move back to original position
	switch (rearCanopy.phase)
	{
		case CANOPY_CLOSED:
			// move flaps to zero, rotate to setting, move back to original position
			// identity matrix is the original position, so we can use it to move back to original position
			if (rearCanopy.currentAngle > 0.0f)
			{
				// Perform the rotation logic here
				// no need if at target angle
				// move flaps to zero, rotate to setting, move back to original position
				// base coordinates are at 0 rotation to rotate to current angle
				rearCanopy.currentAngle -= rearCanopy.deltaAngle;
				if (rearCanopy.currentAngle > 0.0f)
				{
					rearCanopy.currentAngle = 0.0f; // flaps are up so use Identity
				}
			}
			break;
		case CANOPY_OPEN:
			// move flaps to zero, rotate to setting, move back to original position
			if (rearCanopy.currentAngle < rearCanopy.targetAngle)
			{
				// step angle towards target angle
				rearCanopy.currentAngle += rearCanopy.deltaAngle;
				if (rearCanopy.currentAngle > rearCanopy.targetAngle)
				{
					rearCanopy.currentAngle = rearCanopy.targetAngle;
				}
			}
			// Perform the rotation logic here
			// move flaps to zero, rotate to setting, move back to original position
			break;
	}
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-rearCanopy.currentAngle)) * moveToPosition;
	return result;
}
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// The DONE Line
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
XMMATRIX T38::MoveLeftFlap()
{
	XMMATRIX result = XMMatrixIdentity();
	// pivot azis is X at the closest point to the nose (max Z)
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[LeftFlap].second.x, -ControlBounds[LeftFlap].second.y, -ControlBounds[LeftFlap].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[LeftFlap].second.x, ControlBounds[LeftFlap].second.y, ControlBounds[LeftFlap].second.z);
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-flaps.currentAngle)) * moveToPosition;
	return result;
}
XMMATRIX T38::MoveRightFlap()
{
	XMMATRIX result = XMMatrixIdentity();
	// pivot azis is X at the closest point to the nose (max Z)
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[RightFlap].second.x, -ControlBounds[RightFlap].second.y, -ControlBounds[RightFlap].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[RightFlap].second.x, ControlBounds[RightFlap].second.y, ControlBounds[RightFlap].second.z);
	// Move the flaps by delta degrees
	// stop at 0 degrees and 60 degrees
	// delta is positive for down and negative for up
	// move flaps to zero, rotate to setting, move back to original position
	switch (flaps.phase)
	{
		case FLAPS_UP:
			// move flaps to zero, rotate to setting, move back to original position
			// identity matrix is the original position, so we can use it to move back to original position
			if (flaps.currentAngle > 0.0f)
			{
				// Perform the rotation logic here
				// no need if at target angle
				// move flaps to zero, rotate to setting, move back to original position
				// base coordinates are at 0 rotation to rotate to current angle
				flaps.currentAngle -= flaps.deltaAngle;
				if (flaps.currentAngle > 0.0f)
				{
					flaps.currentAngle = 0.0f; // flaps are up so use Identity
				}
			}
			break;
		case FLAPS_DOWN:
			// move flaps to zero, rotate to setting, move back to original position
			if (flaps.currentAngle < flaps.targetAngle)
			{
				// step angle towards target angle
				flaps.currentAngle += flaps.deltaAngle;
				if (flaps.currentAngle > flaps.targetAngle)
				{
					flaps.currentAngle = flaps.targetAngle;
				}
			}
			// Perform the rotation logic here
			// move flaps to zero, rotate to setting, move back to original position
			break;
	}
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-flaps.currentAngle)) * moveToPosition;
	return result;
}
XMMATRIX T38::MoveRightSpeedBrake()
{
	XMMATRIX result = XMMatrixIdentity();
	// SpeedBrakeRight
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[SpeedBrakeRight].second.x, -ControlBounds[SpeedBrakeRight].second.y, -ControlBounds[SpeedBrakeRight].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[SpeedBrakeRight].second.x, ControlBounds[SpeedBrakeRight].second.y, ControlBounds[SpeedBrakeRight].second.z);
	// Move the airbrakes by delta degrees
	// stop at 0 degrees and 90 degrees
	// delta is positive for open and negative for close
	// move airbrakes to zero, rotate to setting, move back to original position
	switch (speedBrakes.phase)
	{
		case AIRBRAKES_RETRACTED:
			// move flaps to zero, rotate to setting, move back to original position
			// identity matrix is the original position, so we can use it to move back to original position
			if (speedBrakes.currentAngle > 0)
			{
				// Perform the rotation logic here
				// no need if at target angle
				// move flaps to zero, rotate to setting, move back to original position
				// base coordinates are at 0 rotation to rotate to current angle
				speedBrakes.currentAngle -= speedBrakes.deltaAngle;
				if (speedBrakes.currentAngle < 0.0f)
				{
					speedBrakes.currentAngle = 0.0f; // flaps are up so use Identity
				}
			}
			break;
		case AIRBRAKES_EXTENDED:
			// move flaps to zero, rotate to setting, move back to original position
			if (speedBrakes.currentAngle < speedBrakes.targetAngle)
			{
				// step angle towards target angle
				speedBrakes.currentAngle += speedBrakes.deltaAngle;
				if (speedBrakes.currentAngle > speedBrakes.targetAngle)
				{
					speedBrakes.currentAngle = speedBrakes.targetAngle;
				}
			}
			// Perform the rotation logic here
			// move flaps to zero, rotate to setting, move back to original position
			break;
	}
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-speedBrakes.currentAngle)) * moveToPosition;
	return result;
}
XMMATRIX T38::MoveLeftSpeedBrake()
{
	XMMATRIX result = XMMatrixIdentity();
	// SpeedBrakeRight
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[SpeedBrakeLeft].second.x, -ControlBounds[SpeedBrakeLeft].second.y, -ControlBounds[SpeedBrakeLeft].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[SpeedBrakeLeft].second.x, ControlBounds[SpeedBrakeLeft].second.y, ControlBounds[SpeedBrakeLeft].second.z);
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-speedBrakes.currentAngle)) * moveToPosition;
	return result;
}

XMMATRIX T38::MoveRightLandingGearStrut()
{
	XMMATRIX result = XMMatrixIdentity();
	// RightGearStrutH
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[RightGearStrutH].second.x, -ControlBounds[RightGearStrutH].second.y, -ControlBounds[RightGearStrutH].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[RightGearStrutH].second.x, ControlBounds[RightGearStrutH].second.y, ControlBounds[RightGearStrutH].second.z);
	// open landing gear door first and close last
	// Move the landing gear by delta degrees
	// stop at 0 degrees and 90 degrees
	// delta is positive for open and negative for close
	// move landing gear to zero, rotate to setting, move back to original position
	// "Right Gear Strut H" is pivot point
	switch (landingGear.phase)
	{
		case LANDING_GEAR_RETRACTED:
			break;
		case LANDING_GEAR_EXTENDING_INNER_DOORS_OPEN:
			// open inner doors
			landingGearDoor.targetAngle = 90.0f;
			break;
		case LANDING_GEAR_EXTENDING:
			// open outer doors AND LOWER LANDING GEAR -- going to 0 degrees
			if (landingGear.currentAngle > landingGear.targetAngle)
			{
				// Perform the rotation logic here
				// no need if at target angle
				// move flaps to zero, rotate to setting, move back to original position
				// base coordinates are at 0 rotation to rotate to current angle
				landingGear.currentAngle -= landingGear.deltaAngle;
				if (landingGear.currentAngle < landingGear.targetAngle)
				{
					landingGear.currentAngle = landingGear.targetAngle; // flaps are up so use Identity
				}
			}
			if (landingGear.currentAngle == landingGear.targetAngle)
			{
				landingGear.phase = LANDING_GEAR_EXTENDING_INNER_DOORS_CLOSE;
			}
			break;
		case LANDING_GEAR_EXTENDING_INNER_DOORS_CLOSE:
			// close inner doors
			landingGearDoor.targetAngle = 0.0f;
			// landing gear extended, do nothing
			break;
		case LANDING_GEAR_EXTENDED:
			break;
		case LANDING_GEAR_RETRACTING_INNER_DOORS_OPEN:
			// open inner door
			landingGearDoor.targetAngle = 90.0f;
		// retract landing gear
			break;
		case LANDING_GEAR_RETRACTING:
			if (landingGear.currentAngle < landingGear.targetAngle)
			{
				// Perform the rotation logic here
				// no need if at target angle
				// move flaps to zero, rotate to setting, move back to original position
				// base coordinates are at 0 rotation to rotate to current angle
				landingGear.currentAngle += landingGear.deltaAngle;
				if (landingGear.currentAngle > landingGear.targetAngle)
				{
					landingGear.currentAngle = landingGear.targetAngle;
				}
				else
				{
				}
			}
			if (landingGear.currentAngle == landingGear.targetAngle)
			{
				landingGear.phase = LANDING_GEAR_RETRACTING_INNER_DOORS_CLOSE;
				landingGearDoor.targetAngle = 0.0f;
			}
			break;
		case LANDING_GEAR_RETRACTING_INNER_DOORS_CLOSE:
			// close inner doors -- draw gear up
			landingGearDoor.targetAngle = 0.0f;
			break;
	}
	result = moveToZero * XMMatrixRotationZ(DegreesToRadians(landingGear.currentAngle)) * moveToPosition;
	return result;

}

XMMATRIX T38::MoveRightLandingGearDoor()
{
	XMMATRIX result = XMMatrixIdentity();
	// RightGearDoorInner
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[RightGearDoorInner].first.x, -ControlBounds[RightGearDoorInner].first.y, -ControlBounds[RightGearDoorInner].first.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[RightGearDoorInner].first.x, ControlBounds[RightGearDoorInner].first.y, ControlBounds[RightGearDoorInner].first.z);
	// open landing gear door first and close last
	// Move the landing gear by delta degrees
	// stop at 0 degrees and 90 degrees
	// delta is positive for open and negative for close
	// move landing gear to zero, rotate to setting, move back to original position
	switch (landingGear.phase)
	{
		case LANDING_GEAR_RETRACTED:
			// open inner doors
			break;
		case LANDING_GEAR_EXTENDING_INNER_DOORS_OPEN:
			// open inner doors
			if (landingGearDoor.currentAngle < 90) //landingGearDoor.targetAngle)
			{
				landingGearDoor.currentAngle += landingGearDoor.deltaAngle;
				if (landingGearDoor.currentAngle > 90) //landingGearDoor.targetAngle)
				{
					landingGearDoor.currentAngle = 90; //landingGearDoor.targetAngle;
				}
			}
			if (landingGearDoor.currentAngle == 90) //landingGearDoor.targetAngle)
			{
				landingGear.phase = LANDING_GEAR_EXTENDING;
			}
			break;
		case LANDING_GEAR_EXTENDING:
			// close inner doors
			break;
		case LANDING_GEAR_EXTENDING_INNER_DOORS_CLOSE:
			// landing gear extended, do nothing
			if (landingGearDoor.currentAngle > 0)
			{
				landingGearDoor.currentAngle -= landingGearDoor.deltaAngle;
				if (landingGearDoor.currentAngle < 0)
				{
					landingGearDoor.currentAngle = 0;
				}
			}
			if (landingGearDoor.currentAngle == 0)
			{
				landingGear.phase = LANDING_GEAR_EXTENDED;
			}
			break;
		case LANDING_GEAR_EXTENDED:
			// retract landing gear
			break;
		case LANDING_GEAR_RETRACTING_INNER_DOORS_OPEN:
			// retract landing gear
			if (landingGearDoor.currentAngle < 90) //landingGearDoor.targetAngle)
			{
				landingGearDoor.currentAngle += landingGearDoor.deltaAngle;
				if (landingGearDoor.currentAngle > 90) //landingGearDoor.targetAngle)
				{
					landingGearDoor.currentAngle = 90; //landingGearDoor.targetAngle;
				}
			}
			if (landingGearDoor.currentAngle == 90) //landingGearDoor.targetAngle)
			{
				landingGear.phase = LANDING_GEAR_RETRACTING;
			}
			break;
		case LANDING_GEAR_RETRACTING:
			// close outer doors
			break;
		case LANDING_GEAR_RETRACTING_INNER_DOORS_CLOSE:
			if (landingGearDoor.currentAngle > 0)
			{
				landingGearDoor.currentAngle -= landingGearDoor.deltaAngle;
				if (landingGearDoor.currentAngle < 0)
				{
					landingGearDoor.currentAngle = 0;
				}
			}
			if (landingGearDoor.currentAngle == 0)
			{
				landingGear.phase = LANDING_GEAR_RETRACTED;
			}
			break;
	}
	result = moveToZero * XMMatrixRotationZ(DegreesToRadians(-landingGearDoor.currentAngle)) * moveToPosition;
	return result;
}
XMMATRIX T38::MoveRightLandingGear()
{
	XMMATRIX result = XMMatrixIdentity();
	// RightGearStrutH
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[RightGearStrutH].second.x, -ControlBounds[RightGearStrutH].second.y, -ControlBounds[RightGearStrutH].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[RightGearStrutH].second.x, ControlBounds[RightGearStrutH].second.y, ControlBounds[RightGearStrutH].second.z);
	result = moveToZero * XMMatrixRotationZ(DegreesToRadians(-landingGear.currentAngle)) * moveToPosition;
	return result;
}

XMMATRIX T38::MoveLeftLandingGear()
{
	XMMATRIX result = XMMatrixIdentity();
	// LeftGearStrutH
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[LeftGearStrutH].second.x, -ControlBounds[LeftGearStrutH].second.y, -ControlBounds[LeftGearStrutH].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[LeftGearStrutH].second.x, ControlBounds[LeftGearStrutH].second.y, ControlBounds[LeftGearStrutH].second.z);
	result = moveToZero * XMMatrixRotationZ(DegreesToRadians(landingGear.currentAngle)) * moveToPosition;
	return result;
}
XMMATRIX T38::MoveLeftLandingGearDoor()
{
	XMMATRIX result = XMMatrixIdentity();
	// LeftGearDoorInner
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[LeftGearDoorInner].second.x, -ControlBounds[LeftGearDoorInner].second.y, -ControlBounds[LeftGearDoorInner].second.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[LeftGearDoorInner].second.x, ControlBounds[LeftGearDoorInner].second.y, ControlBounds[LeftGearDoorInner].second.z);
	result = moveToZero * XMMatrixRotationZ(DegreesToRadians(landingGearDoor.currentAngle)) * moveToPosition;
	return result;
}

XMMATRIX T38::MoveNoseLandingGear()
{
	XMMATRIX result = XMMatrixIdentity();
	// NoseGearStrutH
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[NoseGearStrutH].first.x, -ControlBounds[NoseGearStrutH].second.y, -ControlBounds[NoseGearStrutH].first.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[NoseGearStrutH].first.x, ControlBounds[NoseGearStrutH].second.y, ControlBounds[NoseGearStrutH].first.z);
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-landingGear.currentAngle * (106.0f / 90.0f))) * moveToPosition;
	return result;
}
XMMATRIX T38::MoveNoseLandingGearStrut()
{
	XMMATRIX result = XMMatrixIdentity();
	// NoseGearStrutH
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[NoseGearStrutH].first.x, -ControlBounds[NoseGearStrutH].second.y, -ControlBounds[NoseGearStrutH].first.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[NoseGearStrutH].first.x, ControlBounds[NoseGearStrutH].second.y, ControlBounds[NoseGearStrutH].first.z);
	result = moveToZero * XMMatrixRotationX(DegreesToRadians(-landingGear.currentAngle) * (106.0f / 90.0f)) * moveToPosition;
	return result;
}
XMMATRIX T38::MoveNoseLandingGearDoor()
{
	XMMATRIX result = XMMatrixIdentity();
	// NoseGearDoorInner
	XMMATRIX moveToZero = XMMatrixTranslation(-ControlBounds[NoseGearDoorInner].first.x, -ControlBounds[NoseGearDoorInner].first.y, -ControlBounds[NoseGearDoorInner].first.z);
	XMMATRIX moveToPosition = XMMatrixTranslation(ControlBounds[NoseGearDoorInner].first.x, ControlBounds[NoseGearDoorInner].first.y, ControlBounds[NoseGearDoorInner].first.z);
	result = moveToZero * XMMatrixRotationZ(DegreesToRadians(-landingGearDoor.currentAngle)) * moveToPosition;
	return result;
}
