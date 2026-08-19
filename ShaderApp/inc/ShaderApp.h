
// ShaderApp.h : main header file for the PROJECT_NAME application
//

#pragma once

#include "framework.h"
#include <Device.h>
#include <SwapChain.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace dx12lib;



// CShaderApp:
// See ShaderApp.cpp for the implementation of this class
//

class CShaderApp : public CWinApp
{
public:
	CShaderApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
private:
	std::shared_ptr<Device>      m_Device;
	std::shared_ptr<SwapChain>   m_SwapChain;
	bool m_TearingSupported = false;
	static uint64_t ms_FrameCount;


public:
	bool CheckTearingSupport();
	bool IsTearingSupported() const { return m_TearingSupported; }
	uint64_t GetFrameCount() { return ms_FrameCount; }

    DECLARE_MESSAGE_MAP()
};

