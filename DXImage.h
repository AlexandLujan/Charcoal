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

#include <cstdint>
#include <d3d12.h>
#include <dxgi.h>
#include <map>
#include <memory>
#include <opencv2/videoio.hpp>
#include <sal.h>
#include <speechapi_cxx.h>
#include <speechapi_cxx_enums.h>
#include <speechapi_cxx_recognition_result.h>
#include <speechapi_cxx_speech_config.h>
#include <speechapi_cxx_speech_recognizer.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <windows.h>
#include "Actor.h"
#include "Camera.h"
#include "Light.h"
#include "nlohmann/json.hpp"
#include "Object.h"
#include "Scripting.h"


using json = nlohmann::json;

class DXImage
{
public:
	DXImage(UINT width, UINT height, std::string name);
	virtual ~DXImage();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;

	// Samples override the event handlers to handle specific messages.
	virtual void OnKeyDown(UINT8 /*key*/) {}
	virtual void OnKeyUp(UINT8 /*key*/) {}

	// Accessors.
	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	const std::string& GetTitle() const { return m_title; }

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

	static std::unordered_map<std::string, std::shared_ptr<Actor>>				ActorList;
	static std::unordered_map<std::string, std::shared_ptr<Object>>				ObjectList;
	static std::unordered_map<std::string, std::shared_ptr<Camera>>				CameraList;
	static std::unordered_map<std::string, std::shared_ptr<Light>>				LightList;
	static std::unordered_map<std::string, std::shared_ptr<PointLight>>			PointLightList;
	static std::unordered_map<std::string, std::shared_ptr<SpotLight>>			SpotLightList;
	static std::unordered_map<std::string, std::shared_ptr<DirectionalLight>>	DirectionalLightList;
	static std::unordered_map<std::string, std::pair<UINT, UINT>>				MeshlistDirectory; // name, meshlet start, meshlet count	

protected:
	std::wstring GetAssetFullPath(LPCWSTR assetName);

	void GetHardwareAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter = false);

	void SetCustomWindowText(std::string text);

	// Viewport dimensions.
	UINT m_width;
	UINT m_height;
	float m_aspectRatio;

	// Adapter info.
	bool m_useWarpDevice;

//private:
	// Root assets path.
	std::wstring m_assetsPath;

	// Window title.
	std::string m_title;

//	std::map<std::string, std::shared_ptr<Actor>>			  ActorList;
//	std::map<std::string, std::shared_ptr<Assembly>>			  ObjectList;
//	std::map<std::string, std::shared_ptr<Camera>>			  CameraList;
//	std::map<std::string, std::shared_ptr<Light>>			  LightList;
//	std::map<std::string, std::shared_ptr<PointLight>>		  PointLightList;
//	std::map<std::string, std::shared_ptr<SpotLight>>		  SpotLightList;
//	std::map<std::string, std::shared_ptr<DirectionalLight>>  DirectionalLightList;
	float ambient = 0.01f;
		// JSON 
	json document;
	json actors;
	json objects;
	json setup;
	json cameras;
	json lights;
	json script;
	json object_script;
	json camera_script;
	json lights_script;

	void BuildJSON(std::string filename);
	void BuildActors(json& actors);
	void BuildObjects(json& objects);
	void ObjectSetUps(json& setup);
	void BuildCameras(json& cameras);
	void BuildLights(json& lights);
	void BuildScript(json& actions);

	std::shared_ptr<Object> BuildText(json& element, std::string name);
	std::shared_ptr<Object> BuildMesh(json& element, std::string name);
	std::shared_ptr<Object> BuildOBJFileMesh(json& element, std::string name);
	std::shared_ptr<Object> BuildTemplateReference(json& element, std::string name);
	std::shared_ptr<Object> BuildFractal(json& element, std::string name);
	std::shared_ptr<Object> BuildBox(json& element, std::string name);
	std::shared_ptr<Object> BuildTunnel(json& element, std::string name);
	std::shared_ptr<Object> BuildRoundTunnel(json& element, std::string name);
	std::shared_ptr<Object> BuildTriangleTunnel(json& element, std::string name);
	std::shared_ptr<Object> BuildStarShip(json& element, std::string name);
//	void UpdateObjectRotation(std::shared_ptr<Object> o);
//	void UpdateObjectPositionBoxes(std::shared_ptr<Object> o);

//	std::shared_ptr<Object> GetTemplate(std::string name) { return TemplateList[name]; }
	void UpdateScripts(UINT64 frameNumber);
	std::vector<std::shared_ptr<CameraScript>> cameraScripts;
	std::vector<std::shared_ptr<ActorScript>> actorScripts;
	void BuildCameraScript(json& camera_script);
	void BuildActorScript(json& actor_script);
	std::vector<std::shared_ptr<ScriptSchedule>> BuildSchedule(json& a_schedule);
	void ReadJson(std::wstring filename);

	std::string movie_filename = "";
	UINT movie_start_frame = 0;
	UINT movie_end_frame = 0;
	double movie_fps = 30.0;
	UINT movie_width = 1920;
	UINT movie_height = 1080;
	bool video_recording_in_progress = true;
	bool video_output_started = true;
	bool video_output_finished = false;
	cv::VideoWriter video;

	int StartVideoOutput(int w, int h, double fps, std::string out_file);
	void EncodeFrame(uint8_t* bgra_data, int linesize);
	void FinishVideo();
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig> speechConfig = nullptr;
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognizer> speechRecognizer = nullptr;

};
