
// ShaderAppDlg.cpp : implementation file
//

#include "pch.h"
#include <d3dcompiler.h>
#include "Resource.h"
#include "afxdialogex.h"
#include "framework.h"
#include "ShaderApp.h"
#include "ShaderAppDlg.h"
#pragma comment(lib, "d3dcompiler.lib")
#include "CommandQueue.h"
#include "CommandList.h"
#include "Camera.h"
#include "Light.h"
#include "Object.h"
#include "Lighting.h"
#include "LightPage.h"
#include "CameraPage.h"
#include "ObjectPage.h"
#include "ContentPage.h"
#include "VideoPage.h"
#include "Utility.h"
#include <algorithm>
#include <exception>
#include <fstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace dx12lib;

extern CShaderApp theApp;
LONG CShaderAppDlg::m_windowStyle;
RECT CShaderAppDlg::m_windowRect;
// Vertex data for a colored cube.
FLOAT		Light::Nothing;
XMFLOAT4	Light::Nothing4;

CShaderAppDlg* parent;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CShaderAppDlg dialog



CShaderAppDlg::CShaderAppDlg(Device& device, CWnd* pParent /*=nullptr*/, bool vSync)
	: CDialogEx(IDD_SHADERAPP_DIALOG, pParent), m_Device(device), m_vSync(vSync)
{
	m_windowBounds = { 0,0,0,0 };
	m_aspectRatio = 0.0f;
	m_enableUI = true;
	m_adapterIDoverride = UINT_MAX;
	m_ScissorRect = { CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX) };
	m_FoV = 45.0f;
	m_ContentLoaded = false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CShaderAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CShaderAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SCRIPT_MESSAGE, OnScriptMessage)
	ON_WM_MOUSEWHEEL()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_CAMERA_MESSAGE, OnCameraMessage)
	ON_MESSAGE(WM_CONTENT_MESSAGE, OnContentMessage)
	ON_MESSAGE(WM_LIGHTING_MESSAGE, OnLightMessage)
	ON_MESSAGE(WM_OBJECT_MESSAGE, OnObjectMessage)
	ON_MESSAGE(WM_VIDEO_MESSAGE, OnVideoMessage)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CShaderAppDlg message handlers

BOOL CShaderAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	parent = this;

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	ShowWindow(SW_MAXIMIZE);
	GetWindowRect(&m_windowRect);
	screenWidth = SCREEN_WIDTH;
	screenHeight = SCREEN_HEIGHT;
	m_Viewport = { CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)) };

	controlBoard = new CControlBoard(_T("CONTROL BOARD"), this, 0);
	if (!controlBoard->Create(this))
	{
		delete controlBoard;
		controlBoard = NULL;
		return -1;
	}
	controlBoard->ShowWindow(SW_SHOW);

	shared_ptr<Camera> c = make_shared<Camera>("Default Camera");

	c->Position() = { 0, 0, -50 };
	c->Target() = { 0, 0, 0 };

	c->set_LookAt(
		c->CameraPositionVector(),
		c->TargetPositionVector(),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);

	pCurrentCamera = c;
	CameraList["Default Camera"] = c;

	// Register the hardcoded camera with the Control Board.
	controlBoard->ResetCameraNameList();
	controlBoard->UpdateAddCameraName("Default Camera");
	controlBoard->SetSelectedCamera(0);

	// Set the renderer's current camera name so OnCameraMessage() can find this camera in CameraList.
	cameraName = "Default Camera";

	// Synchronize the Camera Tab Controls with the camera's current values.
	controlBoard->UpdateCameraPosition(
		{ c->X(), c->Y(), c->Z() }
	);

	controlBoard->UpdateCameraTarget(
		{ c->TargetX(), c->TargetY(), c->TargetZ() }
	);

	auto defaultLight = std::make_shared<DirectionalLight>(
		XMFLOAT4{ 0.0f, -1.0f, 1.0f, 0.0f },
		XMFLOAT4{ 0.0f, -1.0f, 1.0f, 0.0f },
		"WHITE",
		0.2f,
		1.0f,
		1.0f
	);

	defaultLight->TurnOn();

	RegisterLight("Default Directional Light", defaultLight);

	LightName = "Default Directional Light";
	currentLight = defaultLight;

	controlBoard->ResetLightingNameList();
	controlBoard->UpdateAddLightingName("Default Directional Light");
	controlBoard->SetSelectedLighting(0);
	controlBoard->UpdateLightColor(defaultLight->Color());
	controlBoard->UpdateLightingAmbientColor(AmbientLightColor);
	controlBoard->UpdateLightType(defaultLight->LightType());

	g_IsInitialized = false;

	InitParticles();
	SetTimer(1, 16, nullptr);

	CWnd* pOk = GetDlgItem(IDOK);
	if (pOk) pOk->ShowWindow(SW_HIDE);

	CWnd* pCancel = GetDlgItem(IDCANCEL);
	if (pCancel) pCancel->ShowWindow(SW_HIDE);

	CWnd* pChild = GetWindow(GW_CHILD);
	while (pChild)
	{
		if (pChild != controlBoard)
		{
			pChild->ShowWindow(SW_HIDE);
		}
		pChild = pChild->GetNextWindow();
	}

	return TRUE;
}

void CShaderAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CShaderAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CShaderAppDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return 0;
}

void CShaderAppDlg::OnPaint()
{
	CPaintDC dc(this); // IMPORTANT: validates the paint message.

	static int paintLogCount = 0;
	paintLogCount++;

	if (paintLogCount <= 20)
	{
		std::ofstream log("sandbox_debug_log.txt", std::ios::app);
		log << "[Paint] OnPaint entered. g_IsInitialized = "
			<< (g_IsInitialized ? "true" : "false") << "\n";
		log.flush();
	}

	if (g_IsInitialized)
	{
		if (paintLogCount <= 20)
		{
			std::ofstream log("sandbox_debug_log.txt", std::ios::app);
			log << "[Paint] g_IsInitialized true. Paint validated, returning.\n";
			log.flush();
		}

		return;
	}

	CreateBackBuffer(&dc);

	CRect rect;
	GetClientRect(&rect);

	CBrush bgBrush(RGB(10, 10, 25));
	m_MemDC.FillRect(&rect, &bgBrush);

	CPen ringPen(PS_SOLID, 1, RGB(40, 40, 80));
	CPen* oldPen = m_MemDC.SelectObject(&ringPen);
	CBrush* oldNullBrush = (CBrush*)m_MemDC.SelectStockObject(NULL_BRUSH);

	int cx = rect.Width() / 2;
	int cy = rect.Height() / 2;

	for (int r = 60; r <= 220; r += 40)
	{
		m_MemDC.Ellipse(cx - r, cy - r, cx + r, cy + r);
	}

	m_MemDC.SelectObject(oldNullBrush);
	m_MemDC.SelectObject(oldPen);

	for (size_t i = 0; i < m_Particles.size(); i++)
	{
		for (size_t j = i + 1; j < m_Particles.size(); j++)
		{
			float dx = m_Particles[j].x - m_Particles[i].x;
			float dy = m_Particles[j].y - m_Particles[i].y;
			float distSq = dx * dx + dy * dy;

			if (distSq < 120.0f * 120.0f)
			{
				CPen linePen(PS_SOLID, 1, RGB(80, 80, 140));
				CPen* oldLinePen = m_MemDC.SelectObject(&linePen);

				m_MemDC.MoveTo((int)m_Particles[i].x, (int)m_Particles[i].y);
				m_MemDC.LineTo((int)m_Particles[j].x, (int)m_Particles[j].y);

				m_MemDC.SelectObject(oldLinePen);
			}
		}
	}

	for (const auto& p : m_Particles)
	{
		CBrush brush(p.color);
		CBrush* oldBrush = m_MemDC.SelectObject(&brush);
		CPen* oldParticlePen = (CPen*)m_MemDC.SelectStockObject(NULL_PEN);

		int left = static_cast<int>(p.x - p.size);
		int top = static_cast<int>(p.y - p.size);
		int right = static_cast<int>(p.x + p.size);
		int bottom = static_cast<int>(p.y + p.size);

		m_MemDC.Ellipse(left, top, right, bottom);

		m_MemDC.SelectObject(oldParticlePen);
		m_MemDC.SelectObject(oldBrush);
	}

	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &m_MemDC, 0, 0, SRCCOPY);
}

#if 1
LRESULT CShaderAppDlg::OnCameraMessage(WPARAM wParam, LPARAM lParam)
{
	shared_ptr<Camera> c = nullptr;
	switch (wParam)
	{
		case WM_CAMERA_CHANGED:
			cameraName = *(string*)lParam;
			c = CameraList[cameraName];
			if (c != nullptr)
			{
				controlBoard->UpdateCameraPosition({ c->X(), c->Y(), c->Z() });
				controlBoard->UpdateCameraTarget({ c->TargetX(), c->TargetY(), c->TargetZ() });
	//			c->Update();
				pCurrentCamera = c;
			}
			break;
		case WM_CAMERA_X_CHANGED:
		{
			c = CameraList[cameraName];
			if (c == nullptr) break;

			const int32_t value = static_cast<int32_t>(lParam);
			c->X() = static_cast<float>(value) / 10.0f;

			//		c->Update();
			//		needs_update = true;

			XMVECTOR eye = c->CameraPositionVector();
			XMVECTOR target = c->TargetPositionVector();

			if (!XMVector3Equal(eye, target))
			{
				XMVECTOR direction = XMVector3Normalize(
					XMVectorSubtract(target, eye)
				);

				XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

				float alignment = fabsf(
					XMVectorGetX(
						XMVector3Dot(direction, up)
					)
				);

				// If looking almost straight up/down,
				// use Z as the temporary up axis instead.
				if (alignment >= 0.999f)
				{
					up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
				}

				c->set_LookAt(eye, target, up);
			}
		}
			break;
		case WM_CAMERA_Y_CHANGED:
		{
			c = CameraList[cameraName];
			if (c == nullptr) break;

			const int32_t value = static_cast<int32_t>(lParam);
			c->Y() = static_cast<float>(value) / 10.0f;

			//		c->Update();
			//		needs_update = true;

			XMVECTOR eye = c->CameraPositionVector();
			XMVECTOR target = c->TargetPositionVector();

			if (!XMVector3Equal(eye, target))
			{
				XMVECTOR direction = XMVector3Normalize(
					XMVectorSubtract(target, eye)
				);

				XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

				float alignment = fabsf(
					XMVectorGetX(
						XMVector3Dot(direction, up)
					)
				);

				// If looking almost straight up/down,
				// use Z as the temporary up axis instead.
				if (alignment >= 0.999f)
				{
					up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
				}

				c->set_LookAt(eye, target, up);
			}
		}
			break;
		case WM_CAMERA_Z_CHANGED:
		{
			c = CameraList[cameraName];
			if (c == nullptr) break;

			const int32_t value = static_cast<int32_t>(lParam);
			c->Z() = static_cast<float>(value) / 10.0f;

			//		c->Update();
			//		needs_update = true;

			XMVECTOR eye = c->CameraPositionVector();
			XMVECTOR target = c->TargetPositionVector();

			if (!XMVector3Equal(eye, target))
			{
				XMVECTOR direction = XMVector3Normalize(
					XMVectorSubtract(target, eye)
				);

				XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

				float alignment = fabsf(
					XMVectorGetX(
						XMVector3Dot(direction, up)
					)
				);

				// If looking almost straight up/down,
				// use Z as the temporary up axis instead.
				if (alignment >= 0.999f)
				{
					up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
				}

				c->set_LookAt(eye, target, up);
			}
		}
			break;
		case WM_CAMERA_TARGET_X_CHANGED:
		{
			c = CameraList[cameraName];
			if (c == nullptr) break;

			const int32_t value = static_cast<int32_t>(lParam);
			c->TargetX() = static_cast<float>(value) / 10.0f;


			//		c->Update();
			//		needs_update = true;

			XMVECTOR eye = c->CameraPositionVector();
			XMVECTOR target = c->TargetPositionVector();

			if (!XMVector3Equal(eye, target))
			{
				XMVECTOR direction = XMVector3Normalize(
					XMVectorSubtract(target, eye)
				);

				XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

				float alignment = fabsf(
					XMVectorGetX(
						XMVector3Dot(direction, up)
					)
				);

				// If looking almost straight up/down,
				// use Z as the temporary up axis instead.
				if (alignment >= 0.999f)
				{
					up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
				}

				c->set_LookAt(eye, target, up);
			}
		}
			break;
		case WM_CAMERA_TARGET_Y_CHANGED:
		{
			c = CameraList[cameraName];
			if (c == nullptr) break;

			const int32_t value = static_cast<int32_t>(lParam);
			c->TargetY() = static_cast<float>(value) / 10.0f;


			//		c->Update();
			//		needs_update = true;

			XMVECTOR eye = c->CameraPositionVector();
			XMVECTOR target = c->TargetPositionVector();

			if (!XMVector3Equal(eye, target))
			{
				XMVECTOR direction = XMVector3Normalize(
					XMVectorSubtract(target, eye)
				);

				XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

				float alignment = fabsf(
					XMVectorGetX(
						XMVector3Dot(direction, up)
					)
				);

				// If looking almost straight up/down,
				// use Z as the temporary up axis instead.
				if (alignment >= 0.999f)
				{
					up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
				}

				c->set_LookAt(eye, target, up);
			}
		}
			break;
		case WM_CAMERA_TARGET_Z_CHANGED:
		{
			c = CameraList[cameraName];
			if (c == nullptr) break;

			const int32_t value = static_cast<int32_t>(lParam);
			c->TargetZ() = static_cast<float>(value) / 10.0f;


			//		c->Update();
			//		needs_update = true;

			XMVECTOR eye = c->CameraPositionVector();
			XMVECTOR target = c->TargetPositionVector();

			if (!XMVector3Equal(eye, target))
			{
				XMVECTOR direction = XMVector3Normalize(
					XMVectorSubtract(target, eye)
				);

				XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

				float alignment = fabsf(
					XMVectorGetX(
						XMVector3Dot(direction, up)
					)
				);

				// If looking almost straight up/down,
				// use Z as the temporary up axis instead.
				if (alignment >= 0.999f)
				{
					up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
				}

				c->set_LookAt(eye, target, up);
			}
		}
			break;
	}
	return NOERROR;
}
LRESULT CShaderAppDlg::OnContentMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case WM_CONTENT_FRAME_RATE_CHANGE:
			frame_rate = (uint32_t)lParam;
			SetTimer(1, frame_rate, NULL);
			break;
		case WM_CONTENT_BACKGROUND_COLOR_CHANGE:
			{
				XMFLOAT4 c = RGB2FRGB(colorswap((COLORREF)lParam));
				BackgroundColor[0] = c.x;
				BackgroundColor[1] = c.y;
				BackgroundColor[2] = c.z;
				BackgroundColor[3] = c.w;
				//		needs_update = true;
			}
			break;
		case WM_CONTENT_JSON_FILENAME:
			{
				std::ofstream log("sandbox_debug_log.txt", std::ios::app);

				log << "[JSON] WM_CONTENT_JSON_FILENAME received. g_IsInitialized = "
					<< (g_IsInitialized ? "true" : "false") << "\n";
				log.flush();

				if (g_IsInitialized)
				{
					log << "[JSON] WM_CONTENT_JSON_FILENAME ignored because app is already initialized\n";
					log.flush();
					break;
				}

				string str = *(string*)lParam;

				log << "[JSON] About to call ReadJson\n";
				log.flush();

				ReadJson(str);

				log << "[JSON] ReadJson finished\n";
				log.flush();

				if (m_ContentLoaded)
				{
					log << "[JSON] Content loaded. About to call Initialize\n";
					log.flush();

					Initialize();

					log << "[JSON] Initialize finished\n";
					log.flush();

					log << "[JSON] About to call OnResized\n";
					log.flush();

					OnResized(screenWidth, screenHeight);

					log << "[JSON] OnResized finished\n";
					log.flush();

					g_IsInitialized = true;

					log << "[JSON] g_IsInitialized set true\n";
					log.flush();

					Invalidate(FALSE);

					log << "[JSON] Invalidate called. Leaving WM_CONTENT_JSON_FILENAME handler\n";
					log.flush();
				}
				else
				{
					log << "[JSON] ReadJson failed. Initialize skipped.\n";
					log.flush();
				}
			}
			break;
		case WM_CONTENT_GRID_TOGGLE:
	//		show_Grid = lParam;
	//		needs_update = true;
			break;
		case WM_CONTENT_AXIS_TOGGLE:
	//		show_XY_Axis = lParam;
	//		needs_update = true;
			break;
		case WM_CONTENT_RENDER_STYLE_CHANGE:
			switch (lParam)
			{
				case 0: RenderStyle = WIREFRAME; break;
				case 1: RenderStyle = WIREFRAMEwPERSPECTIVE; break;
				case 2: RenderStyle = RASTERIZE; break;
				case 3: RenderStyle = RASTERIZEwPERSPECTIVE; break;
				case 4: RenderStyle = RAY_TRACE; break;
				case 5: RenderStyle = RAY_TRACE_Z; break;
			}
	//		needs_update = true;
			break;
		case WM_CONTENT_HEX_CORRIDOR:
		{
			if (g_IsInitialized) break;

			Initialize();
			OnResized(screenWidth, screenHeight);
			g_IsInitialized = true;
			Invalidate(FALSE);

			break;
		}
	}
	return NOERROR;
}
LRESULT CShaderAppDlg::OnLightMessage(WPARAM wParam, LPARAM lParam)
{
	shared_ptr<Light> l = currentLight;
	if ((currentLight == nullptr) and (wParam != WM_LIGHT_CHANGED))
	{
		l = LightList[LightName];
		if (l == nullptr) return 1;
		currentLight = l;
	}
	switch (wParam)
	{
		case WM_LIGHT_CHANGED:
			LightName = *(string*)lParam;
			l = LightList[LightName];
			if (l != nullptr)
			{
				controlBoard->UpdateLightPosition(l->Position());
				controlBoard->UpdateLightTarget(l->Target());
				controlBoard->UpdateLightColor(l->Color());
				controlBoard->UpdateLightAmbientIntensity(l->Ambient());
				controlBoard->UpdateLightType(l->LightType());
				controlBoard->UpdateLightStatus(l->Status());
				currentLight = l;
			}
			break;
		case WM_LIGHT_STATUS_TOGGLE:
			if ((bool)lParam)
				l->TurnOn();
			else
				l->TurnOff();
			break;
		case WM_LIGHT_X_CHANGED:
		{
			const int32_t value = static_cast<int32_t>(lParam);
			l->X() = static_cast<float>(value);
		}
			break;
		case WM_LIGHT_Y_CHANGED:
		{
			const int32_t value = static_cast<int32_t>(lParam);
			l->Y() = static_cast<float>(value);
		}
			break;
		case WM_LIGHT_Z_CHANGED:
		{
			const int32_t value = static_cast<int32_t>(lParam);
			l->Z() = static_cast<float>(value);
		}
			break;
		case WM_LIGHT_TARGET_X_CHANGED:
		{ 
			const int32_t value = static_cast<int32_t>(lParam); 
			l->TargetX() = static_cast<float>(value) / 100.0f;
		}
			break;
		case WM_LIGHT_TARGET_Y_CHANGED:
		{
			const int32_t value = static_cast<int32_t>(lParam);
			l->TargetY() = static_cast<float>(value) / 100.0f;
		}
			break;
		case WM_LIGHT_TARGET_Z_CHANGED:
		{ 
			const int32_t value = static_cast<int32_t>(lParam);
			l->TargetZ() = static_cast<float>(value) / 100.0f;
		}
			break;
		case WM_LIGHT_COLOR_CHANGED:
			{
				uint32_t c = (uint32_t)lParam;
				l->SetColor(RGB2FRGB(colorswap(c)));
			}
			break;
		case WM_LIGHT_INTENSITY_CHANGED:
			{
		//		FLOAT the_intensity = powf(10.0f, ((float)lParam / 100.0f));
		//		l->Intensity() = the_intensity;
			l->diffuseIntensity = static_cast<float>(lParam) / 100.0f;
			}
			break;
		case WM_LIGHT_AMBIENT_COLOR_CHANGED:
			{
				uint32_t c = static_cast<uint32_t>(lParam);
				AmbientLightColor = RGB2FRGB(colorswap(c));
			}
			break;
		case WM_LIGHT_AMBIENT_INTENSITY_CHANGED:
			{
				// FLOAT the_intensity = powf(10.0f, ((float)lParam / 100.0f));
				// l->Ambient() = the_intensity;
				l->Ambient() = static_cast<float>(lParam) / 100.0f;
			}
			break;
		case WM_LIGHT_BEAM_CONE_ANGLE_CHANGED:
			{
				INT beam_angle = (int32_t)lParam;
				l->Spread() = cosf(DegreesToRadians((float)beam_angle));
			}
			break;
	}
	if (m_LightingPSO)
	{
		m_LightingPSO->SetPointLights(PointLightList);
		m_LightingPSO->SetSpotLights(SpotLightList);
		m_LightingPSO->SetDirectionalLights(DirectionalLightList);
	}
	return NOERROR;
}

LRESULT CShaderAppDlg::OnObjectMessage(WPARAM wParam, LPARAM lParam)
{
	shared_ptr<Object> o = nullptr;
	switch (wParam)
	{
		case WM_OBJECT_CHANGED:
			objectName = *(string*)lParam;
			o = ObjectList[objectName];
			if (o != nullptr)
			{
				UpdateObjectRotation(o);
				UpdateObjectPositionBoxes(o);
			}
			break;
		case WM_OBJECT_X_CHANGED:
			o = ObjectList[objectName];
			if (o != nullptr)
			{
				o->MoveTo((float)lParam, o->Y(), o->Z());
			}
			break;
		case WM_OBJECT_Y_CHANGED:
			o = ObjectList[objectName];
			if (o != nullptr)
			{
				o->MoveTo(o->X(), (float)lParam, o->Z());
			}
			break;
		case WM_OBJECT_Z_CHANGED:
			o = ObjectList[objectName];
			if (o != nullptr)
			{
				o->MoveTo(o->X(), o->Y(), (float)lParam);
			}
			break;
		case WM_OBJECT_PITCH_CHANGED:
			o = ObjectList[objectName];
			if (o != nullptr)
			{
				o->PitchTo((float)lParam);
			}
			break;
		case WM_OBJECT_YAW_CHANGED:
			o = ObjectList[objectName];
			if (o != nullptr)
			{
				o->YawTo((float)lParam);
			}
			break;
		case WM_OBJECT_ROLL_CHANGED:
			o = ObjectList[objectName];
			if (o != nullptr)
			{
				o->RollTo((float)lParam);
			}
			break;
	}
	return NOERROR;
}
LRESULT CShaderAppDlg::OnVideoMessage(WPARAM wParam, LPARAM lParam)
{
	return NOERROR;
}
void CShaderAppDlg::UpdateObjectRotation(shared_ptr<Object> o)
{
	controlBoard->UpdateObjectRotation({ o->Pitch(),o->Yaw(),o->Roll() });
}

void CShaderAppDlg::UpdateObjectPositionBoxes(shared_ptr<Object> o)
{
	controlBoard->UpdateObjectPosition({ o->X(),o->Y(),o->Z() });
}

#else

LRESULT CShaderAppDlg::OnCameraMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case CAMERA_SELECTED:
			{
				string* cameraName = reinterpret_cast<string*>(lParam);
				currentCamera = *cameraName;
//				pCurrentCamera = CameraList[currentCamera];
			}
			break;
		case CAMERA_ZOOM_CHANGED:
			break;
	}
	return 0;
}

LRESULT CShaderAppDlg::OnLightMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case LIGHT_BACKGROUND_COLOR_CHANGED:
			{
				float r = (float)((lParam >> 16) & 0xFF) / 255.0f;
				float g = (float)((lParam >> 8) & 0xFF) / 255.0f;
				float b = (float)(lParam & 0xFF) / 255.0f;
				BackgroundColor = { 0,r,g,b };
			}
			break;
		case LIGHT_AMBIENT_COLOR_CHANGED:
			break;
		case LIGHT_0_COLOR_CHANGED:
			break;
		case LIGHT_1_COLOR_CHANGED:
			break;
		case LIGHT_2_COLOR_CHANGED:
			break;
		case LIGHT_0_POSITION_X_CHANGED:
			break;
		case LIGHT_0_POSITION_Y_CHANGED:
			break;
		case LIGHT_0_POSITION_Z_CHANGED:
			break;
		case LIGHT_1_POSITION_X_CHANGED:
			break;
		case LIGHT_1_POSITION_Y_CHANGED:
			break;
		case LIGHT_1_POSITION_Z_CHANGED:
			break;
		case LIGHT_2_POSITION_X_CHANGED:
			break;
		case LIGHT_2_POSITION_Y_CHANGED:
			break;
		case LIGHT_2_POSITION_Z_CHANGED:
			break;
		case LIGHT_0_TARGET_X_CHANGED:
			break;
		case LIGHT_0_TARGET_Y_CHANGED:
			break;
		case LIGHT_0_TARGET_Z_CHANGED:
			break;
		case LIGHT_1_TARGET_X_CHANGED:
			break;
		case LIGHT_1_TARGET_Y_CHANGED:
			break;
		case LIGHT_1_TARGET_Z_CHANGED:
			break;
		case LIGHT_2_TARGET_X_CHANGED:
			break;
		case LIGHT_2_TARGET_Y_CHANGED:
			break;
		case LIGHT_2_TARGET_Z_CHANGED:
			break;
		case LIGHT_0_ON_CHANGED:
			break;
		case LIGHT_1_ON_CHANGED:
			break;
		case LIGHT_2_ON_CHANGED:
			break;
	}
	return 0;
}
#endif

LRESULT CShaderAppDlg::OnScriptMessage(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

BOOL CShaderAppDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_FoV -= zDelta / 16;
	m_FoV = std::clamp(m_FoV, 12.0f, 90.0f);

//	char buffer[256];
//	sprintf_s(buffer, "FoV: %f\n", m_FoV);
//	OutputDebugStringA(buffer);
	return true;
}


void CShaderAppDlg::OnDestroy()
{
	if (m_BufferReady)
	{
		if (m_OldBitmap)
			m_MemDC.SelectObject(m_OldBitmap);

		m_BackBuffer.DeleteObject();
		m_MemDC.DeleteDC();
		m_BufferReady = false;
	}
}

void CShaderAppDlg::OnSizeChanged(UINT width, UINT height, bool minimized)
{
	if (width != Width() || height != Height())
	{
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
			static_cast<float>(width), static_cast<float>(height));
		if (g_IsInitialized) OnResized(width, height);
	}
}
void CShaderAppDlg::UpdateForSizeChange(UINT width, UINT height)
{
	Width() = width;
	Height() = height;
	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

// Screensaver
void CShaderAppDlg::InitParticles()
{
	if (m_ParticlesInitialized)
		return;

	m_ParticlesInitialized = true;
	m_Particles.clear();

	CRect rect;
	GetClientRect(&rect);

	const int particleCount = 120;

	for (int i = 0; i < particleCount; i++)
	{
		SimpleParticle p;
		p.x = static_cast<float>(rand() % std::max(1, rect.Width()));
		p.y = static_cast<float>(rand() % std::max(1, rect.Height()));

		p.vx = ((rand() % 200) - 100) / 60.0f;
		p.vy = ((rand() % 200) - 100) / 60.0f;

		p.size = 4 + (rand() % 6);

		int r = 100 + rand() % 156;
		int g = 100 + rand() % 156;
		int b = 100 + rand() % 156;
		p.color = RGB(r, g, b);

		m_Particles.push_back(p);
	}
}

// Screensaver.
void CShaderAppDlg::UpdateParticles()
{
	CRect rect;
	GetClientRect(&rect);

	const float centerX = rect.Width() * 0.5f;
	const float centerY = rect.Height() * 0.5f;

	for (size_t i = 0; i < m_Particles.size(); i++)
	{
		auto& p = m_Particles[i];

		float dx = centerX - p.x;
		float dy = centerY - p.y;

		p.vx += dx * 0.0005f;
		p.vy += dy * 0.0005f;

		float temp = p.vx;
		p.vx += -p.vy * 0.002f;
		p.vy += temp * 0.002f;

		p.x += p.vx;
		p.y += p.vy;

		if (p.x < 0.0f)
		{
			p.x = 0.0f;
			p.vx *= -1.0f;
		}
		if (p.x > rect.Width())
		{
			p.x = static_cast<float>(rect.Width());
			p.vx *= -1.0f;
		}
		if (p.y < 0.0f)
		{
			p.y = 0.0f;
			p.vy *= -1.0f;
		}
		if (p.y > rect.Height())
		{
			p.y = static_cast<float>(rect.Height());
			p.vy *= -1.0f;
		}
	}
}

void CShaderAppDlg::OnTimer(UINT_PTR nIDEvent)
{
	static int timerTick = 0;
	timerTick++;

	if (nIDEvent == 1)
	{
		if (timerTick <= 20)
		{
			std::ofstream log("sandbox_debug_log.txt", std::ios::app);

			log << "[Timer] Tick " << timerTick << "\n";
			log << "[Timer] g_IsInitialized = " << (g_IsInitialized ? "true" : "false") << "\n";
			log << "[Timer] pSwapChain = " << (pSwapChain != nullptr ? "valid" : "NULL") << "\n";
			log << "[Timer] pPipelineStateObject = " << (pPipelineStateObject != nullptr ? "valid" : "NULL") << "\n";
			log << "[Timer] pRootSignature = " << (pRootSignature != nullptr ? "valid" : "NULL") << "\n";
			log << "[Timer] pRegularVertexBuffer = "
				<< (pRegularVertexBuffer != nullptr ? "valid" : "NULL") << "\n";

			log << "[Timer] pRegularIndexBuffer = "
				<< (pRegularIndexBuffer != nullptr ? "valid" : "NULL") << "\n";

			log << "[Timer] pEmissiveVertexBuffer = "
				<< (pEmissiveVertexBuffer != nullptr ? "valid" : "NULL") << "\n";

			log << "[Timer] pEmissiveIndexBuffer = "
				<< (pEmissiveIndexBuffer != nullptr ? "valid" : "NULL") << "\n";

			log << "[Timer] m_LightingPSO = "
				<< (m_LightingPSO != nullptr ? "valid" : "NULL") << "\n";

			log << "[Timer] pRegularMaterial = "
				<< (pRegularMaterial != nullptr ? "valid" : "NULL") << "\n";

			log << "[Timer] pEmissiveMaterial = "
				<< (pEmissiveMaterial != nullptr ? "valid" : "NULL") << "\n";
			log.flush();
		}

		if (g_IsInitialized &&
			pSwapChain != nullptr &&
			pPipelineStateObject != nullptr &&
			pRootSignature != nullptr &&
			pRegularVertexBuffer != nullptr &&
			pRegularIndexBuffer != nullptr &&
			pEmissiveVertexBuffer != nullptr &&
			pEmissiveIndexBuffer != nullptr &&
			m_LightingPSO != nullptr &&
			pRegularMaterial != nullptr &&
			pEmissiveMaterial != nullptr)
		{
			if (timerTick <= 20)
			{
				std::ofstream log("sandbox_debug_log.txt", std::ios::app);
				log << "[Timer] Initialized branch entered\n";
				log << "[Timer] About to call OnUpdate\n";
				log.flush();
			}

			UpdateEventArgs ua(1.0 / 60.0, 0.0);
			OnUpdate(ua);

			if (timerTick <= 20)
			{
				std::ofstream log("sandbox_debug_log.txt", std::ios::app);
				log << "[Timer] OnUpdate finished\n";
				log.flush();
			}
		}
		else
		{
			if (timerTick <= 20)
			{
				std::ofstream log("sandbox_debug_log.txt", std::ios::app);
				log << "[Timer] Screensaver branch entered\n";
				log.flush();
			}

			UpdateParticles();
			Invalidate(FALSE);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

BOOL CShaderAppDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CShaderAppDlg::CreateBackBuffer(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	if (!m_BufferReady)
	{
		m_MemDC.CreateCompatibleDC(pDC);
		m_BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		m_OldBitmap = m_MemDC.SelectObject(&m_BackBuffer);
		m_BufferReady = true;
	}
	else
	{
		BITMAP bm = {};
		m_BackBuffer.GetBitmap(&bm);
		if (bm.bmWidth != rect.Width() || bm.bmHeight != rect.Height())
		{
			if (m_OldBitmap)
				m_MemDC.SelectObject(m_OldBitmap);

			m_BackBuffer.DeleteObject();
			m_BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
			m_OldBitmap = m_MemDC.SelectObject(&m_BackBuffer);
		}
	}
}