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
#include <cstdlib>
#include <d3dcommon.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <exception>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <sal.h>
#include <speechapi_cxx.h>
#include <speechapi_cxx_enums.h>
#include <speechapi_cxx_phrase_list_grammar.h>
#include <speechapi_cxx_recognition_result.h>
#include <speechapi_cxx_speech_config.h>
#include <speechapi_cxx_speech_recognizer.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <wchar.h>
#include <windows.h>
#include <wrl/client.h>
#include "Actor.h"
#include "Camera.h"
#include "d3d12.h"
#include "DXImage.h"
#include "DXImageHelper.h"
#include "Light.h"
#include "Object.h"
#include "Win32Application.h"

using namespace std;
using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::WRL;

unordered_map<std::string, std::shared_ptr<Actor>>				DXImage::ActorList;
unordered_map<std::string, std::shared_ptr<Object>>				DXImage::ObjectList;
unordered_map<std::string, std::shared_ptr<Camera>>				DXImage::CameraList;
unordered_map<std::string, std::shared_ptr<Light>>				DXImage::LightList;
unordered_map<std::string, std::shared_ptr<PointLight>>			DXImage::PointLightList;
unordered_map<std::string, std::shared_ptr<SpotLight>>			DXImage::SpotLightList;
unordered_map<std::string, std::shared_ptr<DirectionalLight>>	DXImage::DirectionalLightList;
//unordered_map<string, pair<UINT, UINT>>				DXImage::MeshlistDirectory; // name, meshlet start, meshlet count	

DXImage::DXImage(UINT width, UINT height, std::string name) :
	m_width(width),
	m_height(height),
	m_title(name),
	m_useWarpDevice(false)
{
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	m_assetsPath = assetsPath;

	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}
DXImage::~DXImage() {}

// Helper function for resolving the full path of assets.
std::wstring DXImage::GetAssetFullPath(LPCWSTR assetName)
{
	return m_assetsPath + assetName;
}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_
void DXImage::GetHardwareAdapter(
	IDXGIFactory1* pFactory,
	IDXGIAdapter1** ppAdapter,
	bool requestHighPerformanceAdapter)
{
	*ppAdapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter;

	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapterIndex = 0;
			DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&adapter));
				++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}
	else
	{
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter.Detach();
}

// Helper function for setting the window's title text.
void DXImage::SetCustomWindowText(string text)
{
	std::string windowText = m_title + ": " + text;
//	std::string s = wstring_to_string_mb(windowText);

	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}
\
// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void DXImage::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
	for (int i = 1; i < argc; ++i)
	{
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
			_wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
		{
			m_useWarpDevice = true;
			m_title = m_title + " (WARP)";
		}
		if (_wcsnicmp(argv[i], L"-json", wcslen(argv[i])) == 0 ||
			_wcsnicmp(argv[i], L"/json", wcslen(argv[i])) == 0)
		{
			wstring jsonFileName = argv[i + 1];
			ReadJson(jsonFileName);
		}

	}

	try
	{
		speechConfig = SpeechConfig::FromSubscription("8ztKspsrUCac9xZsyRjPZqsZk8767ca1NQ5txIQNdy933XhGvZDHJQQJ99CFACYeBjFXJ3w3AAAYACOGGE94", "eastus"); //OldGeekSpeechServices

		// Set the language (optional, defaults to en-US)
		speechConfig->SetSpeechRecognitionLanguage("en-US");

		// Create a recognizer using the default microphone
		speechRecognizer = SpeechRecognizer::FromConfig(speechConfig);

		auto phraseList = PhraseListGrammar::FromRecognizer(speechRecognizer);
		vector<string> Keywords = { "flaps", "landing", "gear", "landing gear", "front", "rear", "back", "canopy",
									"front canopy", "rear canoopy", "back canopy", "both", "canopies","both canopies","speed",
									"brakes", "speed brakes", "roll", "barrel", "barrel roll", "climb", "dive", "bank",
									"up", "down", "open", "close", "extend", "retract", "left", "right" };
		for (auto it = Keywords.begin() + 1; it != Keywords.end(); it++)
		{
			string word = *it;
			phraseList->AddPhrase(word);
		}
		for (auto [key, value] : ActorList)
		{
			phraseList->AddPhrase(key);
		}





	}
	catch (exception& e)
	{
		cout << "Exception caught: " << e.what() << "\n";
	}
}

void RecognizeSpeech()
{
// Replace with your Azure subscription key and service region
	auto config = SpeechConfig::FromSubscription("8ztKspsrUCac9xZsyRjPZqsZk8767ca1NQ5txIQNdy933XhGvZDHJQQJ99CFACYeBjFXJ3w3AAAYACOGGE94", "eastus"); //OldGeekSpeechServices

	// Set the language (optional, defaults to en-US)
	config->SetSpeechRecognitionLanguage("en-US");

	// Create a recognizer using the default microphone
	auto recognizer = SpeechRecognizer::FromConfig(config);

	cout << "Speak into your microphone...\n";

	// Start recognition (blocks until a single phrase is spoken)
	auto result = recognizer->RecognizeOnceAsync().get();

	// Check the result status
	if (result->Reason == ResultReason::RecognizedSpeech)
	{
		cout << "RECOGNIZED: " << result->Text << "\n";
	}
	else if (result->Reason == ResultReason::NoMatch)
	{
		cout << "NOMATCH: Speech could not be recognized.\n";
	}
	else if (result->Reason == ResultReason::Canceled)
	{
		auto cancellation = CancellationDetails::FromResult(result);
		cout << "CANCELED: Reason=" << (int)cancellation->Reason << "\n";
		if (cancellation->Reason == CancellationReason::Error)
		{
			cout << "CANCELED: ErrorCode=" << (int)cancellation->ErrorCode << "\n";
			cout << "CANCELED: ErrorDetails=" << cancellation->ErrorDetails << "\n";
		}
	}
}

