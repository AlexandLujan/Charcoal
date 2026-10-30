#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>

enum ScriptActionCode
{
	NoAction,
	Action_GearUp,
	Action_GearDown,
	Action_FlapsUp,
	Action_FlapsDown,
	Action_BankLeft,
	Action_BankRight,
	Action_RudderLeft,
	Action_RudderRight,
	Action_ElevatorClimb,
	Action_ElevatorDive,
	Action_FrontCanopyOpen,
	Action_FrontCanopyClose,
	Action_RearCanopyOpen,
	Action_RearCanopyClose,
	Action_SpeedBrakeExtend,
	Action_SpeedBrakeRetract,
	LastAction
};

struct ScriptSchedule
{
	UINT64				scheduled_frame;
	XMFLOAT3			location;
	XMFLOAT3			rotation;
	XMFLOAT3			target;
	ScriptActionCode	actionCode;
	std::string			parameter;
};

class Scripting
{
protected:
	UINT64 start_frame;
	UINT64 end_frame;
	UINT64 last_scheduled_index = 0;
	float frames_between_entries;
	std::vector<std::shared_ptr<ScriptSchedule>> m_Schedules;
	std::string name;
public:
	Scripting(std::string _name = "") : name(_name) {}
	void SetStartFrame(UINT64 f) { start_frame = f; last_scheduled_index = 0; }
	void SetEndFrame(UINT64 f) { end_frame = f; }
	std::string& Name() { return name; }
	UINT AddScheduleItem(UINT64 scheduled_frame, XMFLOAT3 location, XMFLOAT3 rotation, XMFLOAT3 target, ScriptActionCode action = NoAction, std::string parameter = "")
	{
		std::shared_ptr<ScriptSchedule> s = std::make_shared<ScriptSchedule>(scheduled_frame, location, rotation, target, action, parameter);
		m_Schedules.push_back(s);
		frames_between_entries = (end_frame - start_frame) / static_cast<float>(m_Schedules.size());
		return (UINT)m_Schedules.size() - 1;
	}
	void LoadSchedule(UINT64 start_frame, UINT64 end_frame, std::vector<std::shared_ptr<ScriptSchedule>> schedule)
	{
		this->start_frame = start_frame;
		this->last_scheduled_index = 0;
		this->end_frame = end_frame;
		this->m_Schedules = schedule;
		frames_between_entries = (end_frame - start_frame) / static_cast<float>(m_Schedules.size());
	}
};

class Actor;
class Camera;
class Light;

class ActorScript : public Scripting
{
public:
	ActorScript(std::string _name = "") : Scripting(_name) {}
	void UpdateActor(std::shared_ptr<Actor> actor, UINT64 frameNumber);
};

class CameraScript : public Scripting
{
public:
	CameraScript(std::string _name = "") : Scripting(_name) {}
	void UpdateCamera(std::shared_ptr<Camera> camera, UINT64 frameNumber);
};

class LightingScript : public Scripting
{
public:
	LightingScript(std::string _name = "") : Scripting(_name) {}
	void UpdateLight(std::shared_ptr<Light> light, UINT64 frameNumber) {}
};
