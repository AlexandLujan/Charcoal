#include "pch.h"
#include "ContentPage.h"
#include "resource.h"
#include <string>
#include "Utility.h"
#include "ControlBoard.h"
#include "Const.h"
using namespace std;
BOOL CCameraPage::OnSetActive()
{
	CPropertyPage::OnSetActive();
	if (CameraListBox.GetCount() == 0) { CameraListBox.AddString(L"No cameras Yet"); }
	Target_X_Slider.SetRange(0, SCREEN_WIDTH);
	Target_Y_Slider.SetRange(0, SCREEN_HEIGHT);
	Target_Z_Slider.SetRange(0, SCREEN_WIDTH);
	Camera_X_Slider.SetRange(0, SCREEN_WIDTH);
	Camera_Y_Slider.SetRange(0, SCREEN_HEIGHT);
	Camera_Z_Slider.SetRange(0, SCREEN_WIDTH);
	Target_X_Slider.SetPos(SCREEN_WIDTH / 2);
	Target_Y_Slider.SetPos(SCREEN_HEIGHT / 2);
	Target_Z_Slider.SetPos(SCREEN_WIDTH / 2);
	Camera_X_Slider.SetPos(SCREEN_WIDTH / 2);
	Camera_Y_Slider.SetPos(SCREEN_HEIGHT / 2);
	Camera_Z_Slider.SetPos(SCREEN_WIDTH / 2);
	return TRUE;
}

void CCameraPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TARGET_X_SLIDER, Target_X_Slider);
	DDX_Control(pDX, IDC_TARGET_Y_SLIDER, Target_Y_Slider);
	DDX_Control(pDX, IDC_TARGET_Z_SLIDER, Target_Z_Slider);
	DDX_Control(pDX, IDC_CAMERA_X_SLIDER, Camera_X_Slider);
	DDX_Control(pDX, IDC_CAMERA_Y_SLIDER, Camera_Y_Slider);
	DDX_Control(pDX, IDC_CAMERA_Z_SLIDER, Camera_Z_Slider);
	DDX_Control(pDX, IDC_CAMERA_LIST, CameraListBox);
	DDX_Control(pDX, IDC_CAMERA_TARGET_X, camera_target_x);
	DDX_Control(pDX, IDC_CAMERA_TARGET_Y, camera_target_y);
	DDX_Control(pDX, IDC_CAMERA_TARGET_Z, camera_target_z);
	DDX_Control(pDX, IDC_CAMERA_X, camera_location_x);
	DDX_Control(pDX, IDC_CAMERA_Y, camera_location_y);
	DDX_Control(pDX, IDC_CAMERA_Z, camera_location_z);
}

BEGIN_MESSAGE_MAP(CCameraPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_CAMERA_LIST, &CCameraPage::OnCbnSelchangeCameraList)
END_MESSAGE_MAP()

#define WM_CAMERA_CHANGED						0
#define WM_CAMERA_X_CHANGED						1
#define WM_CAMERA_Y_CHANGED						2
#define WM_CAMERA_Z_CHANGED						3
#define WM_CAMERA_TARGET_X_CHANGED				4
#define WM_CAMERA_TARGET_Y_CHANGED				5
#define WM_CAMERA_TARGET_Z_CHANGED				6

void CCameraPage::OnCbnSelchangeCameraList()
{
	CString str;
	int sel = CameraListBox.GetCurSel();
	CameraListBox.GetLBText(sel, str);
	string cameraName = ConvertToStrStd(str);
	((CControlBoard*)GetParentSheet())->SendCameraMessage(WM_CAMERA_CHANGED, &cameraName);
}

void CCameraPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	WPARAM SliderID = pScrollBar->GetDlgCtrlID();
	CString text;
	int slideValue = (int)nPos;
	if (nSBCode == TB_THUMBTRACK)
	{
		switch (SliderID)
		{
		case IDC_CAMERA_X_SLIDER:
			((CControlBoard*)GetParentSheet())->SendCameraMessage(WM_CAMERA_X_CHANGED, (slideValue - (SCREEN_WIDTH / 2)) / 5);
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_WIDTH / 2)) / 10);
			camera_location_x.SetWindowTextW(text);
			break;
		case IDC_CAMERA_Y_SLIDER:
			((CControlBoard*)GetParentSheet())->SendCameraMessage(WM_CAMERA_Y_CHANGED, (slideValue - (SCREEN_HEIGHT / 2)) / 5);
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_HEIGHT / 2)) / 10);
			camera_location_y.SetWindowTextW(text);
			break;
		case IDC_CAMERA_Z_SLIDER:
			((CControlBoard*)GetParentSheet())->SendCameraMessage(WM_CAMERA_Z_CHANGED, (slideValue - (SCREEN_WIDTH / 2)) / 5);
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_WIDTH / 2))/ 10);
			camera_location_z.SetWindowTextW(text);
			break;
		case IDC_TARGET_X_SLIDER:
			((CControlBoard*)GetParentSheet())->SendCameraMessage(WM_CAMERA_TARGET_X_CHANGED, (slideValue - (SCREEN_WIDTH / 2)));
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_WIDTH / 2)) / 10);
			camera_target_x.SetWindowTextW(text);
			break;
		case IDC_TARGET_Y_SLIDER:
			((CControlBoard*)GetParentSheet())->SendCameraMessage(WM_CAMERA_TARGET_Y_CHANGED, (slideValue - (SCREEN_HEIGHT / 2)));
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_HEIGHT / 2)) / 10);
			camera_target_y.SetWindowTextW(text);
			break;
		case IDC_TARGET_Z_SLIDER:
			((CControlBoard*)GetParentSheet())->SendCameraMessage(WM_CAMERA_TARGET_Z_CHANGED, (slideValue - (SCREEN_WIDTH / 2)));
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_WIDTH / 2)) / 10);
			camera_target_z.SetWindowTextW(text);
			break;
		}
	}
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);

}
void CCameraPage::AddCameraName(string camName)
{
	CString cam_Name = string_to_wstring_mb(camName).c_str();
	CameraListBox.AddString(cam_Name);
}
void CCameraPage::ResetCameraNameList()
{
	CameraListBox.ResetContent();
}
void CCameraPage::SetSelectedCamera(int value)
{
	CameraListBox.SetCurSel(value);
}
void CCameraPage::UpdatePosition(XMINT3 pos)
{
	Camera_X_Slider.SetPos(pos.x + (SCREEN_WIDTH / 2));
	Camera_Y_Slider.SetPos(pos.y + (SCREEN_HEIGHT / 2));
	Camera_Z_Slider.SetPos(pos.z + (SCREEN_WIDTH / 2));
	CString text;
	text.Format((LPCWSTR)L"%d", pos.x);
	camera_location_x.SetWindowTextW(text);
	text.Format((LPCWSTR)L"%d", pos.y);
	camera_location_y.SetWindowTextW(text);
	text.Format((LPCWSTR)L"%d", pos.z);
	camera_location_z.SetWindowTextW(text);
}

void CCameraPage::UpdateTarget(XMINT3 pos)
{
	Target_X_Slider.SetPos(pos.x + (SCREEN_WIDTH / 2));
	Target_Y_Slider.SetPos(pos.y + (SCREEN_HEIGHT / 2));
	Target_Z_Slider.SetPos(pos.z + (SCREEN_WIDTH / 2));
	CString text;
	text.Format((LPCWSTR)L"%d", pos.x);
	camera_target_x.SetWindowTextW(text);
	text.Format((LPCWSTR)L"%d", pos.y);
	camera_target_y.SetWindowTextW(text);
	text.Format((LPCWSTR)L"%d", pos.z);
	camera_target_z.SetWindowTextW(text);
}
