// ControlPanel.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "CCameraControlDlg.h"
#include "Camera.h"
#include <map>
#include <string>

map<string, std::shared_ptr<Camera>> CameraList;

vector<string> CameraNames;
map<string, Shape*> ComponentList;

// ControlPanel dialog

IMPLEMENT_DYNAMIC(CCameraControlDlg, CDialogEx)

CCameraControlDlg::CCameraControlDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CAMERACONTROL, nullptr)
{
	mainParent = pParent;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CCameraControlDlg::~CCameraControlDlg()
{
}

void CCameraControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CAMERA_X_LOCATION, CameraXValueBox);
	DDX_Control(pDX, IDC_CAMERA_Y_LOCATION, CameraYValueBox);
	DDX_Control(pDX, IDC_CAMERA_Z_LOCATION, CameraZValueBox);
	DDX_Control(pDX, IDC_VECTOR_X, VectorXValueBox);
	DDX_Control(pDX, IDC_VECTOR_Y, VectorYValueBox);
	DDX_Control(pDX, IDC_VECTOR_Z, VectorZValueBox);
	DDX_Control(pDX, IDC_CAMERA_TRACKING, CameraTrackingList);
	DDX_Control(pDX, IDC_SURFACE_X, SurfaceXValueBox);
	DDX_Control(pDX, IDC_SURFACE_Y, SurfaceYValueBox);
	DDX_Control(pDX, IDC_SURFACE_Z, SurfaceZValueBox);
	DDX_Control(pDX, IDC_TRACKING_OBJECTS, TrackingObjectsList);
	DDX_Control(pDX, IDC_CAMERA_X, Camera_X_Contol);
	DDX_Control(pDX, IDC_CAMERA_Y, Camera_Y_Contol);
	DDX_Control(pDX, IDC_CAMERA_Z, Camera_Z_Contol);
	DDX_Control(pDX, IDC_CAMERA_TARGET_X, Camera_Target_X_Contol);
	DDX_Control(pDX, IDC_CAMERA_TARGET_Y, Camera_Target_Y_Contol);
	DDX_Control(pDX, IDC_CAMERA_TARGET_Z, Camera_Target_Z_Contol);
	DDX_Control(pDX, IDC_CAMERA_ZOOM, ZoomControl);
	DDX_Control(pDX, IDC_ZOOM_VALUE, ZoomValueBox);
	DDX_Control(pDX, IDC_CAMERA_LIST, Camera_List);
}


BEGIN_MESSAGE_MAP(CCameraControlDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_CAMERA_TRACKING, &CCameraControlDlg::OnCbnSelchangeCameraTracking)
	ON_CBN_SELCHANGE(IDC_CAMERA_LIST, &CCameraControlDlg::OnCbnSelchangeCameraList)
	ON_CBN_SELCHANGE(IDC_TRACKING_OBJECTS, &CCameraControlDlg::OnCbnSelchangeTrackingObjects)
END_MESSAGE_MAP()


// ControlPanel message handlers

BOOL CCameraControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ShowWindow(SW_SHOWNORMAL);
//	ClearCanvas();
//	Invalidate();
//	bm.CreateBitmap(PICTURE_WIDTH, PICTURE_HEIGHT, 1, 32, picture[0]);

	Camera_X_Contol.SetRange(-1000, 1000, TRUE);
	Camera_Y_Contol.SetRange(-1000, 1000, TRUE);
	Camera_Z_Contol.SetRange(-1000, 1000, TRUE);
	Camera_Target_Y_Contol.SetRange(-1000, 1000, TRUE);
	Camera_Target_X_Contol.SetRange(-1000, 1000, TRUE);
	Camera_Target_Z_Contol.SetRange(-1000, 1000, TRUE);
	ZoomControl.SetRange(5, 95, TRUE);
	Camera_Target_X_Contol.SetPos(0);
	Camera_Target_Y_Contol.SetPos(0);
	Camera_Target_Z_Contol.SetPos(0);
	ZoomControl.SetPos(50);
	Camera_X_Contol.SetPos(0);
	Camera_Y_Contol.SetPos(0);
	Camera_Z_Contol.SetPos(0);
	CameraXValueBox.SetWindowText(L"0");
	CameraYValueBox.SetWindowText(L"0");
	CameraZValueBox.SetWindowText(L"0");
	CameraTrackingList.AddString(L"Do Not Track");
	CameraTrackingList.AddString(L"Track Set Location");
	CameraTrackingList.AddString(L"Track An Object");
	CameraTrackingList.AddString(L"Track Location 0,0");
	CameraTrackingList.SetCurSel(3);
	trackingStyle = allStyles[3];

	TrackingObjectsList.AddString(L"No Objects"); // load with camera after json build
	TrackingObjectsList.SetCurSel(0);
	shared_ptr<Camera> new_camera = make_shared<Camera>();
	new_camera->X() = CameraXContolValue;
	new_camera->Y() = CameraYContolValue;
	new_camera->Z() = CameraZContolValue;
	new_camera->TargetX() = CameraTargetXContolValue;
	new_camera->TargetY() = CameraTargetYContolValue;
	new_camera->TargetZ() = CameraTargetZContolValue;
	new_camera->Zoom() = ZoomControlValue;
	new_camera->Tracking() = trackingStyle;
	CameraNames.push_back("Default Camera");
	CameraList.insert_or_assign("Default Camera", new_camera);
	if (CalculateCameraLocation(new_camera, CameraXContolValue, CameraYContolValue, CameraZContolValue))
	{
		UpdateCameraVector(new_camera,trackingStyle);
		UpdateCameraValues(new_camera);
	}
	Camera_List.ResetContent();
	Camera_List.AddString(L"Default Camera");
	Camera_List.SetCurSel(0);

	if (CalculateCameraLocation(new_camera,CameraXContolValue, CameraYContolValue, CameraZContolValue))
	{
		UpdateCameraVector(new_camera,trackingStyle);
		UpdateCameraValues(new_camera);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCameraControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	CDialogEx::OnPaint();
}

HCURSOR CCameraControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CCameraControlDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	DestroyWindow();
	delete this;
}

void CCameraControlDlg::UpdateSurfaceValues(shared_ptr<Camera> c)
{
	CString surfacexText;
	CString surfaceyText;
	CString surfacezText;
	XMFLOAT3 location = c->TargetLocation();
	surfacexText.Format(_T("%7.2f"), location.x);
	surfaceyText.Format(_T("%7.2f"), location.y);
	surfacezText.Format(_T("%7.2f"), location.z);
	SurfaceXValueBox.SetWindowText(surfacexText);
	SurfaceYValueBox.SetWindowText(surfaceyText);
	SurfaceZValueBox.SetWindowText(surfacezText);
}

void CCameraControlDlg::UpdateCameraValues(shared_ptr<Camera> c)
{
	CString cameraxText;
	CString camerayText;
	CString camerazText;
	XMFLOAT3 location = c->CameraLocation();
	cameraxText.Format(_T("%7.2f"), location.x);
	camerayText.Format(_T("%7.2f"), location.y);
	camerazText.Format(_T("%7.2f"), location.z);
	CameraXValueBox.SetWindowText(cameraxText);
	CameraYValueBox.SetWindowText(camerayText);
	CameraZValueBox.SetWindowText(camerazText);
}

void CCameraControlDlg::UpdateCameraVectorValues()
{
	CString vectorxText;
	CString vectoryText;
	CString vectorzText;
	vectorxText.Format(_T("%7.2f"), ViewVector.x);
	vectoryText.Format(_T("%7.2f"), ViewVector.y);
	vectorzText.Format(_T("%7.2f"), ViewVector.z);
	VectorXValueBox.SetWindowText(vectorxText);
	VectorYValueBox.SetWindowText(vectoryText);
	VectorZValueBox.SetWindowText(vectorzText);
}

void CCameraControlDlg::UpdateSurfaceLocation()
{
}

void CCameraControlDlg::UpdateVectorControls()
{
	CString vectorxText;
	CString vectoryText;
	CString vectorzText;
	vectorxText.Format(_T("%8.5f"), ViewVector.x);
	vectoryText.Format(_T("%8.5f"), ViewVector.y);
	vectorzText.Format(_T("%8.5f"), ViewVector.z);
	VectorXValueBox.SetWindowText(vectorxText);
	VectorYValueBox.SetWindowText(vectoryText);
	VectorZValueBox.SetWindowText(vectorzText);
}
#include <atlconv.h>
void CCameraControlDlg::UpdateCameraVector(shared_ptr<Camera> c, TrackingStyle s)
{
	float dx = 0;
	float dy = 0;
	float dz = 0;
	float vx = 0;
	float vy = 0;
	float vz = 0;
	float distance = 0;
	XMFLOAT3 position = c->CameraLocation();
	XMFLOAT3 surface = c->TargetLocation();
	switch (s)
	{
	case DoNotTrack:
		dx = -position.x;
		dy = -position.y;
		dz = -position.z;
		break;
	case TrackToSetLocation:
		dx = surface.x - position.x;
		dy = surface.y - position.y;
		dz = surface.z - position.z;
		distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
		if (distance != 0)
		{
			ViewVector.x = dx / distance;
			ViewVector.y = dy / distance;
			ViewVector.z = dz / distance;
		}
		UpdateVectorControls();
		break;
	case TrackToObject:
	{
		USES_CONVERSION;
		CString strSelectedItemText;
		TrackingObjectsList.GetLBText(TrackingObjectsList.GetCurSel(), strSelectedItemText);
		string object_name = T2A(strSelectedItemText);
		Shape* track_object = ComponentList[object_name];

		dx = track_object->Last().x - position.x;
		dy = track_object->Last().y - position.y;
		dz = track_object->Last().z - position.z;
		distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
		if (distance != 0)
		{
			ViewVector.x = dx / distance;
			ViewVector.y = dy / distance;
			ViewVector.z = dz / distance;
		}
		UpdateVectorControls();
	}
	break;
	case TrackToOrigin:
		dx = -position.x;
		dy = -position.y;
		dz = -position.z;
		distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
		ViewVector.x = dx / distance;
		ViewVector.y = dy / distance;
		ViewVector.z = dz / distance;
		surface.x = 0;
		surface.y = 0;
		surface.z = 0;
		UpdateVectorControls();
		break;
	}

	// calculate new surface coordinates
	if (dz != 0)
	{
		float t = -position.z / dz;
		surface.x = position.x + t * dx;
		surface.y = position.y + t * dy;
		surface.z = position.z + t * dz;
		// update the windows
		UpdateCameraVectorValues();
		UpdateSurfaceValues(c);
		FXMVECTOR eye = { position.x, position.y, position.z, 1 };
		FXMVECTOR target = { surface.x, surface.y, surface.z, 1 };
		FXMVECTOR up = { 0, 1, 0, 1 };
		c->set_LookAt(eye, target, up);
	}
}
// declination rotation around x axis
// azmuth rotation around Y axis
// tilt rotation around Z axis
bool CCameraControlDlg::CalculateCameraLocation(shared_ptr<Camera> c,int32_t x, int32_t y, int32_t z)
{
	c->X() = x;
	c->Y() = y;
	c->Z() = z;
	return true;
}


void CCameraControlDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	WPARAM SliderID = pScrollBar->GetDlgCtrlID();
	CString text;
	text.Format(_T("%d"), (int)nPos);
	int sel = Camera_List.GetCurSel();
	shared_ptr<Camera> c = CameraList[CameraNames[sel]];

	if (nSBCode == TB_THUMBTRACK)
	{
		switch (SliderID)
		{
		case IDC_CAMERA_X: //AzimuthContol.GetDlgCtrlID())
		{
			CameraXContolValue = (int)nPos;
			CameraXValueBox.SetWindowText(text);
			c->X() = (int)nPos;
			if (CalculateCameraLocation(c,CameraXContolValue, CameraYContolValue, CameraZContolValue))
			{
				UpdateCameraVector(c,trackingStyle);
				UpdateCameraValues(c);
//				::PostMessage(mainParent->m_hWnd, WM_CAMERA_MESSAGE, CAMERA_X_CHANGED, sel);
			}

		}
		break;
		case IDC_CAMERA_Y: //ElevationContol.GetDlgCtrlID())
		{
			CameraYContolValue = (int)nPos;
			CameraYValueBox.SetWindowText(text);
			c->Y() = (int)nPos;
			if (CalculateCameraLocation(c,CameraXContolValue, CameraYContolValue, CameraZContolValue))
			{
				UpdateCameraVector(c,trackingStyle);
				UpdateCameraValues(c);
//				::PostMessage(GetParent()->m_hWnd, WM_CAMERA_MESSAGE, CAMERA_Y_CHANGED, sel);
			}
		}
		break;
		case IDC_CAMERA_Z: //RadiusContol.GetDlgCtrlID())
		{
			CameraZContolValue = (int)nPos;
			CameraZValueBox.SetWindowText(text);
			c->Z() = (int)nPos;
			if (CalculateCameraLocation(c,CameraXContolValue, CameraYContolValue, CameraZContolValue))
			{
				UpdateCameraVector(c,trackingStyle);
				UpdateCameraValues(c);
//				::PostMessage(GetParent()->m_hWnd, WM_CAMERA_MESSAGE, CAMERA_Z_CHANGED, sel);
			}
		}
		break;
		case IDC_CAMERA_ZOOM: //ZoomControl.GetDlgCtrlID())
		{
			ZoomControlValue = (int)nPos;
			ZoomValueBox.SetWindowText(text);
			c->Zoom() = (int)nPos;
		}
		break;
		case IDC_CAMERA_TARGET_X: //TwistContol.GetDlgCtrlID())
		{
			CameraTargetXContolValue = (int)nPos;
			SurfaceXValueBox.SetWindowText(text);
			c->TargetX() = (int)nPos;
			UpdateCameraVector(c, trackingStyle);
			UpdateCameraVectorValues();
			UpdateSurfaceValues(c);
		}
		break;
		case IDC_CAMERA_TARGET_Y: //TiltContol.GetDlgCtrlID())
		{
			CameraTargetYContolValue = (int)nPos;
			SurfaceYValueBox.SetWindowText(text);
			c->TargetY() = (int)nPos;
			UpdateCameraVector(c,trackingStyle);
			UpdateCameraVectorValues();
			UpdateSurfaceValues(c);
		}
		break;
		case IDC_CAMERA_TARGET_Z: //PanContol.GetDlgCtrlID())
		{
			CameraTargetZContolValue = (int)nPos;
			SurfaceZValueBox.SetWindowText(text);
			c->TargetZ() = (int)nPos;
			UpdateCameraVector(c, trackingStyle);
			UpdateCameraVectorValues();
			UpdateSurfaceValues(c);
		}
		break;
		}
	}
}

float CCameraControlDlg::getValueFromControl(DWORD id)
{
	CString str;
	CEdit* editBox = (CEdit*)GetDlgItem(id);
	editBox->GetWindowText(str);
	return static_cast<float>(_ttof(str));
}

void CCameraControlDlg::OnCbnSelchangeCameraTracking()
{
	int csel = Camera_List.GetCurSel();
	shared_ptr<Camera> c = CameraList[CameraNames[csel]];
	int sel = CameraTrackingList.GetCurSel();
	trackingStyle = allStyles[sel];
	UpdateCameraVector(c,trackingStyle);
}
void CCameraControlDlg::OnCbnSelchangeTrackingObjects()
{
	int sel = Camera_List.GetCurSel();
	shared_ptr<Camera> c = CameraList[CameraNames[sel]];
	UpdateCameraVector(c,trackingStyle);
}

void CCameraControlDlg::OnCbnSelchangeCameraList()
{
	int sel = Camera_List.GetCurSel();
	shared_ptr<Camera> c = CameraList[CameraNames[sel]];

	CString AzimuthText;
	CString ElevationText;
	CString RadiusText;
	CString TiltText;
	CString TwistText;
	CString PanText;
	CString ZoomText;

	AzimuthText.Format(_T("%5d"), c->X());
	ElevationText.Format(_T("%5d"), c->Y());
	RadiusText.Format(_T("%6d"), c->Z());
	TiltText.Format(_T("%5d"), c->TargetX());
	TwistText.Format(_T("%5d"), c->TargetY());
	PanText.Format(_T("%5d"), c->TargetZ());
	ZoomText.Format(_T("%5d"), c->Zoom());

	Camera_X_Contol.SetPos(c->X());
	Camera_Y_Contol.SetPos(c->Y());
	Camera_Z_Contol.SetPos(c->Z());
	Camera_Target_X_Contol.SetPos(c->TargetX());
	Camera_Target_Y_Contol.SetPos(c->TargetY());
	Camera_Target_Z_Contol.SetPos(c->TargetZ());
	ZoomControl.SetPos(c->Zoom());


	CameraXValueBox.SetWindowText(AzimuthText);
	CameraYValueBox.SetWindowText(ElevationText);
	CameraZValueBox.SetWindowText(RadiusText);
	//	CameraTargetXValueBox.SetWindowText(TwistText);
	//	CameraTargetYValueBox.SetWindowText(TiltText);
	//	CameraTargetZValueBox.SetWindowText(PanText);
	ZoomValueBox.SetWindowText(ZoomText);
	CameraTrackingList.SetCurSel(c->Tracking());

	if (CalculateCameraLocation(c,CameraXContolValue, CameraYContolValue, CameraZContolValue))
	{
		UpdateCameraVector(c,trackingStyle);
		UpdateCameraValues(c);
		UpdateSurfaceValues(c);
	}
	::PostMessage(GetParent()->m_hWnd, WM_CAMERA_MESSAGE, CAMERA_SELECTED, (LPARAM) &CameraNames[sel]);
}

void CCameraControlDlg::OnClose()
{
#if 0
	for (auto& [key, value] : ComponentList)
	{
		auto p = value;
		delete p;
	}
	ComponentList.clear();

	for (auto& [key, value] : CameraList)
	{
		auto p = value;
		delete p;
	}
	CameraList.clear();

	for (auto& [key, value] : LightingList)
	{
		auto p = value;
		delete p;
	}
	LightingList.clear();

	for (auto& [key, value] : textureList)
	{
		auto p = value;
		delete p;
	}
	textureList.clear();

	for (Shape* s : LayoutList)
	{
		delete s;
	}
	LayoutList.clear();

	LightNames.clear();
	LightColors.clear();
	CameraNames.clear();

	Triangle::ClearTrianglePlacedList();
#endif
	CDialog::OnClose();
}

