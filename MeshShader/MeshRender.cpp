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
#include "stdafx.h"
#include <afx.h>
#include <afxstr.h>
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstringt.h>
#include <d3dcommon.h>
#include <DirectXMath.h>
#include <DirectXMathConvert.inl>
#include <DirectXMathMatrix.inl>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgicommon.h>
#include <dxgiformat.h>
#include <exception>
#include <iostream>
#include <memory>
#include <queue>
#include <speechapi_cxx.h>
#include <speechapi_cxx_enums.h>
#include <speechapi_cxx_recognition_result.h>
#include <speechapi_cxx_speech_config.h>
#include <speechapi_cxx_speech_recognizer.h>
#include <string>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <wrl/client.h>
#include "Actor.h"
#include "Camera.h"
#include "d3d12.h"
#include "d3d12sdklayers.h"
#include "d3dx12.h"
#include "DXImage.h"
#include "DXImageHelper.h"
#include "MeshRender.h"
#include "Model.h"
#include "steptimer.h"
#include "Utility.h"
#include "Win32Application.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Microsoft::CognitiveServices::Speech;
using namespace std;

extern std::unordered_map<std::string, std::shared_ptr<Actor>> ActorList;

//const wchar_t* MeshRender::c_meshFilename = L"Dragon_LOD0.bin";
//const wchar_t* D3D12MeshletRender::c_meshFilename = L"..\\Assets\\Dragon_LOD0.bin";

//const wchar_t* MeshRender::c_meshShaderFilename = L"MeshletMS.cso";
//const wchar_t* MeshRender::c_pixelShaderFilename = L"MeshletPS.cso";
const wchar_t* MeshRender::c_meshShaderFilename = L"InstanceMeshShaderMS.cso";
const wchar_t* MeshRender::c_pixelShaderFilename = L"InstanceMeshShaderPS.cso";
const wchar_t* MeshRender::c_ampShaderFilename = L"InstanceMeshShaderAS.cso";
MeshRender::MeshRender(UINT width, UINT height, std::string name)
	: DXImage(width, height, name)
	, m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
	, m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
	, m_rtvDescriptorSize(0)
	, m_dsvDescriptorSize(0)
	, m_constantBufferData{}
	, m_cbvDataBegin(nullptr)
	, m_frameIndex(0)
	, m_frameCounter(0)
	, m_fenceEvent{}
	, m_fenceValues{}
{}

void MeshRender::OnInit()
{
	m_camera.ResetView();
	m_camera.Position() = { 0, 0, -10, 1 };
//	m_camera.SetMoveSpeed(0); // 150.0f);
	LoadPipeline();
	m_pipelineState_solid = CreateMeshShaderPipelineState(FALSE);
	m_pipelineState_transparent = CreateMeshShaderPipelineState(TRUE);

	LoadAssets();
}

// Load the rendering pipeline dependencies.
void MeshRender::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_12_2,
			IID_PPV_ARGS(&m_device)
		));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter, true);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_12_2,
			IID_PPV_ARGS(&m_device)
		));
	}

	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
		|| (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
	{
		OutputDebugStringA("ERROR: Shader Model 6.5 is not supported\n");
		throw std::exception("Shader Model 6.5 is not supported");
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features)))
		|| (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
	{
		OutputDebugStringA("ERROR: Mesh Shaders aren't supported!\n");
		throw std::exception("Mesh Shaders aren't supported!");
	}
#ifdef _DEBUGx
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		// Configure the info queue to fail or break on severe errors if desired
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

		// Define the specific message ID you want to suppress
		D3D12_MESSAGE_ID denyIDs[] = {
			D3D12_MESSAGE_ID_CREATE_COMMANDLIST12,
			D3D12_MESSAGE_ID_DESTROY_COMMANDLIST12,
			D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED // Prevents Warning #1328
		};
//		#586: DESTROY_COMMANDLIST12]
//		D3D12 INFO : Create ID3D12GraphicsCommandList : Addr = 0x000001D7FFB51D70, ExtRef = 1, IntRef = 0[STATE_CREATION INFO #560
		// Populate the filter description structure
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = _countof(denyIDs);
		filter.DenyList.pIDList = denyIDs;

		// Push the filter into the storage filter stack
		infoQueue->PushStorageFilter(&filter);
	}
#endif

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Describe and create a depth stencil view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

		m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);

			ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
		}
	}

	// Create the depth stencil view.
//	if (false)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC depthStencilTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&depthStencilHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilTextureDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencil)
		));

		NAME_D3D12_OBJECT(m_depthStencil);

		m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Create the constant buffer.
	{
		const UINT64 constantBufferSize = sizeof(SceneConstantBuffer) * FrameCount;

		const CD3DX12_HEAP_PROPERTIES constantBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&constantBufferHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&constantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin)));
	}
}

HRESULT MeshRender::LoadShaderCSO(LPCWSTR ShaderByteCode_Filename, ByteCode& shaderBytecode)
{
	HRESULT status = ReadDataFromFile(GetAssetFullPath(ShaderByteCode_Filename).c_str(),
									  &shaderBytecode.data,
									  &shaderBytecode.size);
	if (FAILED(status))
	{
		OutputDebugStringA("ERROR: Failed to load shader bytecode from file.\n");
		return status;
	}
	return S_OK;
}


ComPtr<ID3D12PipelineState>  MeshRender::CreateMeshShaderPipelineState(BOOL transparent)
{
	struct alignas(void*) MeshShaderPipelineStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK SampleMask;
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
//		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_AS AS;  // Amplification Shader subobject
		CD3DX12_PIPELINE_STATE_STREAM_MS MS;  // Mesh Shader subobject
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;  // Pixel Shader subobject
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DepthStencilFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS	RenderTargetFormats;
	} pipelineStream;

	pipelineStream.SampleDesc = DefaultSampleDesc();
	pipelineStream.SampleMask = UINT_MAX;
	DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStream.DepthStencilFormat = depthStencilFormat;
	D3D12_RT_FORMAT_ARRAY renderTargetFormat = { {DXGI_FORMAT_R8G8B8A8_UNORM} ,1 }; // second parameter is NumRenderTargets
	pipelineStream.RenderTargetFormats = renderTargetFormat;


	const D3D12_INPUT_ELEMENT_DESC inputElementDescs[5] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32_UINT,        0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "USES",     0, DXGI_FORMAT_R32_UINT,        0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	//	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	ByteCode shaderBytecode;
//	const wchar_t* MeshRender::c_meshShaderFilename = L"bin/x64/Debug/InstanceMeshShaderMS.cso";
//	const wchar_t* MeshRender::c_pixelShaderFilename = L"bin/x64/Debug/InstanceMeshShaderPS.cso";
//	const wchar_t* MeshRender::c_ampShaderFilename = L"bin/x64/Debug/InstanceMeshShaderAS.cso";
	if (LoadShaderCSO(c_pixelShaderFilename, shaderBytecode) != S_OK)
	{
		OutputDebugStringA("ERROR: Failed to load pixel shader bytecode.\n");
		throw std::exception("Failed to load pixel shader bytecode.");
	}
	pipelineStream.PS = { shaderBytecode.data, shaderBytecode.size };

	if (LoadShaderCSO(c_meshShaderFilename, shaderBytecode) != S_OK)
	{
		OutputDebugStringA("ERROR: Failed to load mesh shader bytecode.\n");
		throw std::exception("Failed to load mesh shader bytecode.");
	}
	pipelineStream.MS = { shaderBytecode.data, shaderBytecode.size };

	if (LoadShaderCSO(c_ampShaderFilename, shaderBytecode) != S_OK)
	{
		OutputDebugStringA("ERROR: Failed to load amplification shader bytecode.\n");
		throw std::exception("Failed to load amplification shader bytecode.");
	}
	pipelineStream.AS = { shaderBytecode.data, shaderBytecode.size };

	ThrowIfFailed(m_device->CreateRootSignature(0, shaderBytecode.data, shaderBytecode.size, IID_PPV_ARGS(&m_rootSignature)));
	pipelineStream.pRootSignature = m_rootSignature.Get();

//	D3D12_INPUT_LAYOUT_DESC inputLayout = { inputElementDescs, _countof(inputElementDescs) };
//	pipelineStream.InputLayout = inputLayout;

	pipelineStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	if (transparent)
	{
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDesc.StencilEnable = FALSE;
	}
	pipelineStream.DepthStencilState = depthStencilDesc;

	CD3DX12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);// 
//	RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; // Wireframe mode for easier meshlet visualization.
	RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
//	RasterizerState.FrontCounterClockwise = true;
	RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	pipelineStream.RasterizerState = RasterizerState;

	CD3DX12_BLEND_DESC BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	BlendState.AlphaToCoverageEnable = FALSE;
	BlendState.IndependentBlendEnable = FALSE;
	if (transparent)
	{
		BlendState.RenderTarget[0].BlendEnable = TRUE;
		BlendState.RenderTarget[0].LogicOpEnable = FALSE;
		BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_MAX; // D3D12_BLEND_OP_ADD;
		BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD; // D3D12_BLEND_OP_ADD;
		BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA; // D3D12_BLEND_ONE;
		BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA; // D3D12_BLEND_ZERO;
		BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	else
	{
		BlendState.RenderTarget[0].BlendEnable = FALSE;
		BlendState.RenderTarget[0].LogicOpEnable = FALSE;
		BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	pipelineStream.BlendState = BlendState;

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
	streamDesc.pPipelineStateSubobjectStream = &pipelineStream;
	streamDesc.SizeInBytes = sizeof(pipelineStream);
	ComPtr<ID3D12PipelineState> pipeline_state;
	ThrowIfFailed(m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pipeline_state))); // m_pipelineState)));
	return pipeline_state;

}


// Load the sample assets.
void MeshRender::LoadAssets()
{
	// Create the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState_solid.Get(), IID_PPV_ARGS(&m_commandList)));
	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	ThrowIfFailed(m_commandList->Close());

	// *************************************************************************
	// *********************  create mesh here *********************************
	// *************************************************************************
	m_model.LoadFromObjectList();
	m_model.UploadGpuResources(m_device.Get(), m_commandQueue.Get(), m_commandAllocators[m_frameIndex].Get(), m_commandList.Get());
	// *************************************************************************
	// *************************************************************************
	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValues[m_frameIndex]++;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForGpu();
	}
}

// Update frame-based values.
UINT64 lastTime = 0;
UINT64 frameCounter = 0;
UINT64 UpdateCount = 0;
UINT64 lastFrame = 0;

void MeshRender::OnUpdate()
{
	UpdateCount++;
	m_timer.Tick(NULL);

	if (m_frameCounter % 36 == 0)
	{
		// Update window text with FPS value.
		char fps[64];
		sprintf_s(fps, "%llu frames at %u fps", frameCounter, m_timer.GetFramesPerSecond());
		SetCustomWindowText(fps);
//		CString st;
//		st.Format("%u fps", m_timer.GetFramesPerSecond());
//		OutputDebugString(st);
	}

	UINT64 t = m_timer.GetTotalTicks();
//  if ((t - lastTime) < (StepTimer::TicksPerSecond / 45)) return;
	lastTime = m_timer.GetTotalTicks();
	frameCounter++;

	m_camera.Update(frameCounter);

	XMMATRIX world = XMMatrixIdentity(); // (g_XMIdentityR0, g_XMIdentityR1, g_XMIdentityR2, g_XMIdentityR3);
	XMMATRIX view = m_camera.get_ViewMatrix();
	m_camera.set_Projection(60.0f, m_aspectRatio, 0.1f, 1000.0f);
	XMMATRIX proj = m_camera.get_ProjectionMatrix();
	// shaders need transposed matrices	
	XMStoreFloat4x4(&m_constantBufferData.World, XMMatrixTranspose(world));
	XMStoreFloat4x4(&m_constantBufferData.WorldView, XMMatrixTranspose(world * view));
	XMStoreFloat4x4(&m_constantBufferData.WorldViewProj, XMMatrixTranspose(world * view * proj));
	m_constantBufferData.camera_position = m_camera.Position();
	m_constantBufferData.camera_direction = m_camera.TargetVector();
	m_constantBufferData.ambient = ambient;
	m_constantBufferData.DrawMeshlets = true;
	memcpy(m_cbvDataBegin + sizeof(SceneConstantBuffer) * m_frameIndex, &m_constantBufferData, sizeof(m_constantBufferData));
	UpdateScripts(frameCounter);

	// handle voice command actions
	while (not requestList.empty())
	{
		auto item = requestList.front();
		shared_ptr<Actor> actor = item.actor;
		// process request
		if (not (actor == nullptr)) actor->HandleAction(item.actionCode, item.parameter);
		requestList.pop();
	}

	m_model.UpdateActors(frameCounter);
}

// Render the scene.
void MeshRender::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	InitializeRenderCommandList();
	PopulateSolidCommandList();
	if (m_model.m_transparentinstance_buffer.size() > 0)
	{
		PopulateTransparentCommandList();
	}
	FinalizeRenderCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));
	WaitForGpu();

#if 1
	if (video_output_started and !video_output_finished and video_recording_in_progress)
	{
		if (frameCounter > lastFrame)
		{
			ID3D12Resource* swapChainBackBuffer = nullptr;
			UINT backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
			m_swapChain->GetBuffer(backBufferIndex, IID_PPV_ARGS(&swapChainBackBuffer));
			VideoEncoder(swapChainBackBuffer);
			lastFrame = frameCounter;
		}
	}
#endif

	MoveToNextFrame();
}

void MeshRender::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForGpu();

	CloseHandle(m_fenceEvent);
}


void MeshRender::OnKeyUp(UINT8 key)
{}

//	D3D12_RESOURCE_STATE_COMMON

void MeshRender::InitializeRenderCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState_solid.Get()));

	// Set necessary state.
//	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// update the amp shader instance structure with updated transformations
	D3D12_RESOURCE_BARRIER postCopyBarrier;
	m_commandList->CopyResource(m_model.SolidInstanceResource.Get(), m_model.solidInstanceUpload.Get());
	postCopyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_model.SolidInstanceResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &postCopyBarrier);

	// Indicate that the back buffer will be used as a render target.
	const auto toRenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &toRenderTargetBarrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

}
void MeshRender::FinalizeRenderCommandList()
{
	// Indicate that the back buffer will now be used to present.
	const auto toPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &toPresentBarrier);
	WaitForGpu();
	ThrowIfFailed(m_commandList->Close());
}

void MeshRender::PopulateSolidCommandList()
{
	m_commandList->SetPipelineState(m_pipelineState_solid.Get());
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + sizeof(SceneConstantBuffer) * m_frameIndex);
	m_commandList->SetGraphicsRoot32BitConstant(1, sizeof(UINT), 0);
	m_commandList->SetGraphicsRootShaderResourceView(2, m_model.VertexResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(3, m_model.MeshletResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(4, m_model.UniqueVertexIndexResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(5, m_model.PrimitiveIndexResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(6, m_model.MaterialResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(7, m_model.CullDataResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(8, m_model.LightDataResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(9, m_model.SolidInstanceResource->GetGPUVirtualAddress());
	m_commandList->DispatchMesh(static_cast<UINT>(m_model.m_solidinstance_buffer.size()), 1, 1);

}
void MeshRender::PopulateTransparentCommandList()
{
	m_commandList->SetPipelineState(m_pipelineState_transparent.Get());
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->SetGraphicsRoot32BitConstant(1, sizeof(UINT), 0);
	m_commandList->SetGraphicsRootShaderResourceView(2, m_model.VertexResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(3, m_model.MeshletResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(4, m_model.UniqueVertexIndexResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(5, m_model.PrimitiveIndexResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(6, m_model.MaterialResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(7, m_model.CullDataResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(8, m_model.LightDataResource->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootShaderResourceView(9, m_model.TransparentInstanceResource->GetGPUVirtualAddress());
	m_commandList->DispatchMesh(static_cast<UINT>(m_model.m_transparentinstance_buffer.size()), 1, 1);
}


// Wait for pending GPU work to complete.
void MeshRender::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void MeshRender::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}
