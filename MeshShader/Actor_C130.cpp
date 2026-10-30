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

C130::C130(string name = "") : Aircraft(name, "C130")
{
	SurfacePairs =
	{
	//	{ "Glass",	{NoControl, 0} },
	//	{ "Glass001", { NoControl, 0 }			},
	//	{ "Glass002", { NoControl, 0 }			},
	//	{ "Interior",	RightAileron },
	//	{ "Main_Body",	LeftAileron },
	//	{ "Main_Body001",	Rudder },
	//	{ "Main_Body002",	ElevatorRight },
		{ "Wings",	ElevatorLeft },
		{ "Wings001",	NoseGearStrutB },
		{ "Wings002",	NoseGearStrutH },
		{ "Wings003",	RightGearStrutB },
		{ "Wings004",	RightGearStrutH },
		{ "Wings005",	LeftGearStrutB },
		{ "Wings006",	LeftGearStrutH },
		{ "Wings007",	FrontCanopy },
		{ "Wings008",	RearCanopy },
		{ "Wings009",	FrontCanopyFrame },
		{ "Wings010",	RearCanopyFrame },
		{ "Wings011",	FrontCanopyShield },
		{ "Wings012",	RearCanopyShield },
		{ "Wings013",	FrontCanopyGlass },
		{ "Wings014",	RearCanopyGlass },
		{ "Wings015",	NoseGearDoorOuter },
		{ "Wings016",	NoseGearDoorInner },
		{ "Wings017",	RightGearDoorInner}
	};
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
}