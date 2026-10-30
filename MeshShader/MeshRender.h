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
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgi1_4.h>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h>
#include "Camera.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "DXImage.h"
#include "Light.h"
#include "Model.h"
#include "nlohmann/json.hpp"
#include "Object.h"
#include "Scripting.h"
#include "StepTimer.h"

using json = nlohmann::json;

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class MeshRender : public DXImage
{
public:
	MeshRender(UINT width, UINT height, std::string name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual void OnKeyDown(UINT8 key);
	virtual void OnKeyUp(UINT8 key);

private:
	static const UINT FrameCount = 2;

	_declspec(align(256u)) struct SceneConstantBuffer
	{
		XMFLOAT4X4 World;
		XMFLOAT4X4 WorldView;
		XMFLOAT4X4 WorldViewProj;
		XMFLOAT4 camera_position;
		XMFLOAT4 camera_direction;
		FLOAT    ambient;
		uint32_t DrawMeshlets;
	};

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device2> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12Resource> m_depthStencil;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState_solid;
	ComPtr<ID3D12PipelineState> m_pipelineState_transparent;
	ComPtr<ID3D12Resource> m_constantBuffer;
	UINT m_rtvDescriptorSize;
	UINT m_dsvDescriptorSize;

	ComPtr<ID3D12GraphicsCommandList6> m_commandList;
	SceneConstantBuffer m_constantBufferData;
	UINT8* m_cbvDataBegin;

	StepTimer m_timer;
	Camera m_camera;
	Model m_model;

	// Synchronization objects.
	UINT m_frameIndex;
	UINT m_frameCounter;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount];

	void LoadPipeline();
	struct ByteCode
	{
		uint8_t* data;
		uint32_t size;
	};
	HRESULT LoadShaderCSO(LPCWSTR ShaderByteCode_Filename, ByteCode& shaderBytecode);
	ComPtr<ID3D12PipelineState>   CreateMeshShaderPipelineState(BOOL transparent);
	void LoadAssets();
	void InitializeRenderCommandList();
	void FinalizeRenderCommandList();

	void PopulateSolidCommandList();
	void PopulateTransparentCommandList();
	void MoveToNextFrame();
	void WaitForGpu();

private:
	static const wchar_t* c_meshFilename;
	static const wchar_t* c_ampShaderFilename;
	static const wchar_t* c_meshShaderFilename;
	static const wchar_t* c_pixelShaderFilename;

	void VideoEncoder(ID3D12Resource* outputBuffer);
	enum VoiceCommandStates
	{
		GatheringActorName,
		GatheringComponentName,
		GatheringCommand,
		GatheringCommandParameters
	};
	vector<string> componentKeywords = { "not set", "flaps", "landing", "gear", "landing gear", "front", "rear", "back", "canopy",
		"front canopy", "rear canoopy", "back canopy", "both", "canopies", "both canopies", "speed",
		"brakes", "speed brakes", "roll", "barrel", "barrel roll", "climb", "dive", "bank" };
	enum VoiceComponents
	{
		component_Not_Set,
		Component_Flaps,
		Component_Landing_Gear,
		Component_Front_Canopy,
		Component_Rear_Canopy,
		Component_Both_Canopies,
		Component_Speed_Brakes,
		Aircraft_Action_Aileron_Roll,
		Aircraft_Action_Barrel_Roll,
		Aircraft_Action_Climb,
		Aircraft_Action_Dive,
		Aircraft_Action_Bank,
		Aircraft_Action_Last
	};
	vector<VoiceComponents> componentID = {
		component_Not_Set,
		Component_Flaps,
		Component_Landing_Gear,
		Component_Landing_Gear,
		Component_Landing_Gear,
		Component_Front_Canopy,
		Component_Rear_Canopy,
		Component_Rear_Canopy,
		Component_Front_Canopy,
		Component_Front_Canopy,
		Component_Rear_Canopy,
		Component_Rear_Canopy,
		Component_Both_Canopies,
		Component_Both_Canopies,
		Component_Both_Canopies,
		Component_Speed_Brakes,
		Component_Speed_Brakes,
		Component_Speed_Brakes,
		Aircraft_Action_Aileron_Roll,
		Aircraft_Action_Barrel_Roll,
		Aircraft_Action_Barrel_Roll,
		Aircraft_Action_Climb,
		Aircraft_Action_Dive,
		Aircraft_Action_Bank
	};
	vector<string> commandKeywords = { "not set", "up", "down", "open", "close", "extend", "retract", "left", "right" };
	enum VoiceCommands
	{
		Command_Not_Set,
		Command_Up,
		Command_Down,
		Command_Open,
		Command_Close,
		Command_Extend,
		Command_Retract,
		Command_Left,
		Command_Right,
		Command_Last
	};
	vector<VoiceCommands> commandID = {
		Command_Not_Set,
		Command_Up,
		Command_Down,
		Command_Open,
		Command_Close,
		Command_Extend,
		Command_Retract,
		Command_Left,
		Command_Right
	};

	bool wordIsKeyWord(string word);
	bool wordIsCommand(string word);
	void ParseVoiceCommand(std::string command);
	void HandleFlaps(std::shared_ptr<Actor> actor, VoiceCommands cmd, std::string percentage);
	void HandleFrontCanopy(std::shared_ptr<Actor> actor, VoiceCommands cmd);
	void HandleRearCanopy(std::shared_ptr<Actor> actor, VoiceCommands cmd);
	void HandleLandingGear(std::shared_ptr<Actor> actor, VoiceCommands cmd);
	void HandleSpeedBrakes(std::shared_ptr<Actor> actor, VoiceCommands cmd);
	struct RemoteActionRequest
	{
		shared_ptr<Actor>	actor;
		ScriptActionCode	actionCode;
		std::string			parameter;
	};
	std::queue<RemoteActionRequest> requestList = {};

//public:
//	Model& GetModel() { return m_model; }
};
