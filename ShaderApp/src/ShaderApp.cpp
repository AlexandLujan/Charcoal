
// ShaderApp.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "ShaderApp.h"
#include "ShaderAppDlg.h"
//#include "afxwinappex.h"
//#include "afxdialogex.h"
//#include "Helpers.h"

#include <fstream>
#include <Windows.h>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

uint64_t CShaderApp::ms_FrameCount;

// CShaderApp

BEGIN_MESSAGE_MAP(CShaderApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CShaderApp construction

CShaderApp::CShaderApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CShaderApp object

CShaderApp theApp;

// String Debug Module
static void DebugTrace(const char* message)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");

    std::ofstream log(
        "charcoal_app_debug_log.txt",
        std::ios::app
    );

    if (log.is_open())
    {
        log << message << std::endl;
    }
}

// CShaderApp initialization

BOOL CShaderApp::InitInstance()
{
    MessageBoxA(NULL, "ShaderApp InitInstance reached", "DEBUG TEST", MB_OK);

    DebugTrace("================ APP START ================");
    DebugTrace("[App] InitInstance start");

    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);

    std::string dirMessage = std::string("[App] Current working directory: ") + currentDir;
    DebugTrace(dirMessage.c_str());

    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles. Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);

    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;

    DebugTrace("[App] Before InitCommonControlsEx");
    InitCommonControlsEx(&InitCtrls);
    DebugTrace("[App] After InitCommonControlsEx");

    DebugTrace("[App] Before CWinApp::InitInstance");
    CWinApp::InitInstance();
    DebugTrace("[App] After CWinApp::InitInstance");

    // DIRECTX12 Initialization

    DebugTrace("[App] Before XMVerifyCPUSupport");

    if (!DirectX::XMVerifyCPUSupport())
    {
        DebugTrace("[App] XMVerifyCPUSupport FAILED");

        MessageBoxA(
            NULL,
            "Failed to verify DirectX Math library support.",
            "Error",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

    DebugTrace("[App] After XMVerifyCPUSupport");

    DebugTrace("[App] Before SetThreadDpiAwarenessContext");
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    DebugTrace("[App] After SetThreadDpiAwarenessContext");

    // Create DirectX 12 Device

    DebugTrace("[App] Before Device::EnableDebugLayer");

#if defined(_DEBUG)
    Device::EnableDebugLayer();
#endif

    DebugTrace("[App] After Device::EnableDebugLayer");

    DebugTrace("[App] Before Device::Create");
    m_Device = Device::Create(nullptr);
    DebugTrace("[App] After Device::Create");


    // Create the shell manager, in case the dialog contains
    // any shell tree view or shell list view controls.

    DebugTrace("[App] Before creating CShellManager");
    CShellManager* pShellManager = new CShellManager;
    DebugTrace("[App] After creating CShellManager");


    // Activate "Windows Native" visual manager for enabling themes in MFC controls

    DebugTrace("[App] Before CMFCVisualManager::SetDefaultManager");
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
    DebugTrace("[App] After CMFCVisualManager::SetDefaultManager");


    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need.
    // Change the registry key under which our settings are stored.

    DebugTrace("[App] Before SetRegistryKey");
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));
    DebugTrace("[App] After SetRegistryKey");


    DebugTrace("[App] Before creating CShaderAppDlg");

    CShaderAppDlg dlg = { *m_Device, nullptr, true };
    m_pMainWnd = &dlg;

    DebugTrace("[App] After creating CShaderAppDlg");

    DebugTrace("[App] Before dlg.DoModal");

    INT_PTR nResponse = dlg.DoModal();

    DebugTrace("[App] After dlg.DoModal");


    if (nResponse == IDOK)
    {
        DebugTrace("[App] Dialog closed with IDOK");

        // TODO: Place code here to handle when the dialog is
        // dismissed with OK
    }
    else if (nResponse == IDCANCEL)
    {
        DebugTrace("[App] Dialog closed with IDCANCEL");

        // TODO: Place code here to handle when the dialog is
        // dismissed with Cancel
    }
    else if (nResponse == -1)
    {
        DebugTrace("[App] Dialog creation failed. nResponse = -1");

        TRACE(
            traceAppMsg,
            0,
            "Warning: dialog creation failed, so application is terminating unexpectedly.\n"
        );

        TRACE(
            traceAppMsg,
            0,
            "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n"
        );
    }


    // Delete the shell manager created above.

    DebugTrace("[App] Before deleting CShellManager");

    if (pShellManager != nullptr)
    {
        delete pShellManager;
    }

    DebugTrace("[App] After deleting CShellManager");


#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)

    DebugTrace("[App] Before ControlBarCleanUp");
    ControlBarCleanUp();
    DebugTrace("[App] After ControlBarCleanUp");

#endif

    // Since the dialog has been closed, return FALSE so that we exit the
    // application, rather than start the application's message pump.

    DebugTrace("[App] InitInstance finished. Returning FALSE");

    return FALSE;
}


bool CShaderApp::CheckTearingSupport()
{
	BOOL allowTearing = FALSE;

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
	// graphics debugging tools which will not support the 1.5 factory interface 
	// until a future update.
	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing, sizeof(allowTearing));
		}
	}

	return allowTearing == TRUE;
}

