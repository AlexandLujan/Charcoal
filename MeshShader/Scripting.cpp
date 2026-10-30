#include "stdafx.h"
#include <DirectXMath.h>
#include <DirectXMathVector.inl>
#include <memory>
#include <vector>
#include <windows.h>
#include "Actor.h"
#include "Camera.h"
#include "DXImage.h"
#include "Scripting.h"

using namespace DirectX;
using namespace std;


void DXImage::UpdateScripts(UINT64 frameNumber)
{
	// movie managment
	if (movie_start_frame != 0)
	{
		if (frameNumber == movie_start_frame)
		{
			video_recording_in_progress = true;
			video_output_started = true;
			video_output_finished = false;
		}
		else if (frameNumber == movie_end_frame)
		{
			FinishVideo();
			video_recording_in_progress = false;
			video_output_finished = true;
		}
	}

	for (auto& camera : cameraScripts)
	{
//		shared_ptr<Camera> pCamera = CameraList[camera->Name()];
		camera->UpdateCamera(CameraList[camera->Name()], frameNumber);
	}
	for (auto& actor : actorScripts)
	{
//		shared_ptr<Actor> pActor = ActorList[actor->Name()];
		actor->UpdateActor(ActorList[actor->Name()], frameNumber);
	}
}

void CameraScript::UpdateCamera(std::shared_ptr<Camera> pCamera, UINT64 frameNumber)
{
#if 0
	if (frameNumber < start_frame || frameNumber > end_frame) return; // not active
	// is this a scheduled frame?
	UINT current_index = 0;
	for (current_index = 0; current_index < m_Schedules.size(); current_index++)
	{
		if (m_Schedules[current_index]->scheduled_frame == frameNumber)
		{
			// snap to the scheduled position/rotation/target
			pCamera->set_Rotation(XMLoadFloat3(&m_Schedules[current_index]->rotation));
			pCamera->set_Translation(XMLoadFloat3(&m_Schedules[current_index]->location));
			pCamera->set_FocalPoint(XMLoadFloat3(&m_Schedules[current_index]->target));
			pCamera->SetLookAt(m_Schedules[current_index]->location,
								m_Schedules[current_index]->target, { 0,1,0 });
			last_scheduled_index = current_index;
			return;
		}
		if (m_Schedules[current_index]->scheduled_frame > frameNumber)
		{
			current_index--; // step back to the last scheduled frame
			break;
		}
	}
	float relative_frame = static_cast<float>(frameNumber - m_Schedules[current_index]->scheduled_frame);
	float frames_between_entries = static_cast<float>(m_Schedules[current_index + 1]->scheduled_frame -
													  m_Schedules[current_index]->scheduled_frame);
	XMFLOAT3 delta_location = { (m_Schedules[current_index + 1]->location.x - m_Schedules[current_index]->location.x) / frames_between_entries,
								(m_Schedules[current_index + 1]->location.y - m_Schedules[current_index]->location.y) / frames_between_entries,
								(m_Schedules[current_index + 1]->location.z - m_Schedules[current_index]->location.z) / frames_between_entries };
	XMFLOAT3 delta_rotation = { (m_Schedules[current_index + 1]->rotation.x - m_Schedules[current_index]->rotation.x) / frames_between_entries,
								(m_Schedules[current_index + 1]->rotation.y - m_Schedules[current_index]->rotation.y) / frames_between_entries,
								(m_Schedules[current_index + 1]->rotation.z - m_Schedules[current_index]->rotation.z) / frames_between_entries };
	XMFLOAT3 delta_target = { (m_Schedules[current_index + 1]->target.x - m_Schedules[current_index]->target.x) / frames_between_entries,
								(m_Schedules[current_index + 1]->target.y - m_Schedules[current_index]->target.y) / frames_between_entries,
								(m_Schedules[current_index + 1]->target.z - m_Schedules[current_index]->target.z) / frames_between_entries };

	XMVECTOR current_location = XMVectorSet(m_Schedules[current_index]->location.x + delta_location.x * relative_frame,
											m_Schedules[current_index]->location.y + delta_location.y * relative_frame,
											m_Schedules[current_index]->location.z + delta_location.z * relative_frame, 1.0f);
	XMVECTOR current_rotation = XMVectorSet(m_Schedules[current_index]->rotation.x + delta_rotation.x * relative_frame,
											m_Schedules[current_index]->rotation.y + delta_rotation.y * relative_frame,
											m_Schedules[current_index]->rotation.z + delta_rotation.z * relative_frame, 1.0f);
	XMVECTOR current_target = XMVectorSet(m_Schedules[current_index]->target.x + delta_target.x * relative_frame,
											m_Schedules[current_index]->target.y + delta_target.y * relative_frame,
											m_Schedules[current_index]->target.z + delta_target.z * relative_frame, 1.0f);


	pCamera->set_Rotation(current_rotation);
	pCamera->set_Translation(current_location);
	pCamera->set_FocalPoint(current_target);
	pCamera->SetLookAt(m_Schedules[current_index]->location, m_Schedules[current_index]->target, { 0,1,0 });
#endif
}

void ActorScript::UpdateActor(std::shared_ptr<Actor> the_actor, UINT64 frameNumber)
{
	if (the_actor == nullptr) return;
	if (frameNumber < start_frame || frameNumber > end_frame) return; // not active
	// is this a scheduled frame?
	UINT current_index = 0;
	for (current_index = 0; current_index < m_Schedules.size(); current_index++)
	{
		ScriptSchedule& schedule = *m_Schedules[current_index];
		if (schedule.scheduled_frame == frameNumber)
		{
			// snap to the scheduled position/rotation
			the_actor->MoveTo(schedule.location.x, schedule.location.y, schedule.location.z);
			the_actor->RotateTo(schedule.rotation.x, schedule.rotation.y, schedule.rotation.z); //degrees
			the_actor->HandleAction(schedule.actionCode, schedule.parameter);
			last_scheduled_index = current_index;
			return;
		}
		if (m_Schedules[current_index]->scheduled_frame > frameNumber)
		{
			current_index--; // step back to the last scheduled frame
			break;
		}
	}
	ScriptSchedule& schedule = *m_Schedules[current_index];
	ScriptSchedule& next_schedule = *m_Schedules[current_index + 1];
	float relative_frame = static_cast<float>(frameNumber - schedule.scheduled_frame);
	float frames_between_entries = static_cast<float>(next_schedule.scheduled_frame - schedule.scheduled_frame);

	XMFLOAT3 delta_location = { (next_schedule.location.x - schedule.location.x) / frames_between_entries,
								(next_schedule.location.y - schedule.location.y) / frames_between_entries,
								(next_schedule.location.z - schedule.location.z) / frames_between_entries };
	XMFLOAT3 delta_rotation = { (next_schedule.rotation.x - schedule.rotation.x) / frames_between_entries,
								(next_schedule.rotation.y - schedule.rotation.y) / frames_between_entries,
								(next_schedule.rotation.z - schedule.rotation.z) / frames_between_entries };

	the_actor->MoveBy(delta_location.x * relative_frame, delta_location.y * relative_frame, delta_location.z * relative_frame);
	the_actor->RotateBy(delta_rotation.x * relative_frame, delta_rotation.y * relative_frame, delta_rotation.z * relative_frame);
}
