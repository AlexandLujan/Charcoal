#include "pch.h"
#include "ContentPage.h"
#include "resource.h"
#include <string>
#include "Utility.h"
#include "ControlBoard.h"
#include "LightPage.h"
#include "Const.h"


BOOL CLightPage::OnSetActive()
{
	CPropertyPage::OnSetActive();
	// LightListBox.AddString(L"No Lights Yet");
	Light_Target_X_Slider.SetRange(-100, 100);
	Light_Target_Y_Slider.SetRange(-100, 100);
	Light_Target_Z_Slider.SetRange(-100, 100);
	Light_X_Slider.SetRange(0, SCREEN_WIDTH);
	Light_Y_Slider.SetRange(0, SCREEN_HEIGHT);
	Light_Z_Slider.SetRange(0, SCREEN_WIDTH);
	Light_Target_X_Slider.SetPos(0);
	Light_Target_Y_Slider.SetPos(0);
	Light_Target_Z_Slider.SetPos(0);
	Light_X_Slider.SetPos(SCREEN_WIDTH / 2);
	Light_Y_Slider.SetPos(SCREEN_HEIGHT / 2);
	Light_Z_Slider.SetPos(SCREEN_WIDTH / 2);
	lighting_intensity.SetRange(1,500);
	lighting_intensity.SetPos(100);
	ambient_intensity.SetRange(1, 500);
	ambient_intensity.SetPos(100);

	return TRUE;
}


void CLightPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIGHT_TARGET_X_SLIDER, Light_Target_X_Slider);
	DDX_Control(pDX, IDC_LIGHT_TARGET_Y_SLIDER, Light_Target_Y_Slider);
	DDX_Control(pDX, IDC_LIGHT_TARGET_Z_SLIDER, Light_Target_Z_Slider);
	DDX_Control(pDX, IDC_LIGHT_X_SLIDER, Light_X_Slider);
	DDX_Control(pDX, IDC_LIGHT_Y_SLIDER, Light_Y_Slider);
	DDX_Control(pDX, IDC_LIGHT_Z_SLIDER, Light_Z_Slider);
	DDX_Control(pDX, IDC_LIGHTS_LIST, LightListBox);
	DDX_Control(pDX, IDC_AMBIENT_COLOR, ambient_color_button);
	DDX_Control(pDX, IDC_AMBIENT_INTENSITY, ambient_intensity);
	DDX_Control(pDX, IDC_BEAM_ANGLE, light_beam_angle);
	DDX_Control(pDX, IDC_LIGHT_STATUS, light_status);
	DDX_Control(pDX, IDC_LIGHTING_COLOR, lighting_color_button);
	DDX_Control(pDX, IDC_LIGHTING_INTENSITY, lighting_intensity);
	DDX_Control(pDX, IDC_LIGHTING_STYLE, lighting_style);
	DDX_Control(pDX, IDC_LIGHT_TARGET_X, light_target_x);
	DDX_Control(pDX, IDC_LIGHT_TARGET_Y, light_target_y);
	DDX_Control(pDX, IDC_LIGHT_TARGET_Z, light_target_z);
	DDX_Control(pDX, IDC_LIGHT_X, light_location_x);
	DDX_Control(pDX, IDC_LIGHT_Y, light_location_y);
	DDX_Control(pDX, IDC_LIGHT_Z, light_location_z);
}

BEGIN_MESSAGE_MAP(CLightPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_LIGHTS_LIST, &CLightPage::OnCbnSelchangeLightsList)
	ON_BN_CLICKED(IDC_LIGHT_STATUS, &CLightPage::OnBnClickedLightStatus)
	ON_BN_CLICKED(IDC_LIGHTING_COLOR, &CLightPage::OnBnClickedLightingColor)
	ON_BN_CLICKED(IDC_AMBIENT_COLOR, &CLightPage::OnBnClickedAmbientColor)
END_MESSAGE_MAP()

#define WM_LIGHT_CHANGED						0
#define WM_LIGHT_STATUS_TOGGLE					1
#define WM_LIGHT_X_CHANGED						2
#define WM_LIGHT_Y_CHANGED						3
#define WM_LIGHT_Z_CHANGED						4
#define WM_LIGHT_TARGET_X_CHANGED				5
#define WM_LIGHT_TARGET_Y_CHANGED				6
#define WM_LIGHT_TARGET_Z_CHANGED				7
#define WM_LIGHT_COLOR_CHANGED					8
#define WM_LIGHT_INTENSITY_CHANGED				9
#define WM_LIGHT_AMBIENT_COLOR_CHANGED			10
#define WM_LIGHT_AMBIENT_INTENSITY_CHANGED		11
#define WM_LIGHT_BEAM_CONE_ANGLE_CHANGED		12

void CLightPage::OnCbnSelchangeLightsList()
{
	CString str;
	int sel = LightListBox.GetCurSel();
	LightListBox.GetLBText(sel, str);
	string lightName = ConvertToStrStd(str);
	((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_CHANGED, &lightName);
}

void CLightPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	WPARAM SliderID = pScrollBar->GetDlgCtrlID();
	CString text;
	int slideValue = (int)nPos;
	if (nSBCode == TB_THUMBTRACK)
	{
		switch (SliderID)
		{
		case IDC_LIGHT_X_SLIDER:
			((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_X_CHANGED, (slideValue - (SCREEN_WIDTH / 2)));
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_WIDTH / 2)));
			light_location_x.SetWindowTextW(text);
			break;
		case IDC_LIGHT_Y_SLIDER:
			((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_Y_CHANGED, (slideValue - (SCREEN_HEIGHT / 2)));
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_HEIGHT / 2)));
			light_location_y.SetWindowTextW(text);
			break;
		case IDC_LIGHT_Z_SLIDER:
			((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_Z_CHANGED, (slideValue - (SCREEN_WIDTH / 2)));
			text.Format((LPCWSTR)L"%d", (slideValue - (SCREEN_WIDTH / 2)));
			light_location_z.SetWindowTextW(text);
			break;
		case IDC_LIGHT_TARGET_X_SLIDER:
			((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_TARGET_X_CHANGED, slideValue);
			text.Format((LPCWSTR)L"%d", slideValue);
			light_target_x.SetWindowTextW(text);
			break;
		case IDC_LIGHT_TARGET_Y_SLIDER:
			((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_TARGET_Y_CHANGED, slideValue);
			text.Format((LPCWSTR)L"%d", slideValue);
			light_target_y.SetWindowTextW(text);
			break;
		case IDC_LIGHT_TARGET_Z_SLIDER:
			((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_TARGET_Z_CHANGED, slideValue);
			text.Format((LPCWSTR)L"%d", slideValue);
			light_target_z.SetWindowTextW(text);
			break;
		case IDC_AMBIENT_INTENSITY:
			((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_AMBIENT_INTENSITY_CHANGED, slideValue);
			break;
		case IDC_LIGHTING_INTENSITY:
			((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_INTENSITY_CHANGED, slideValue);
			break;
		}
	}
}
void CLightPage::OnBnClickedLightStatus()
{
	((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_STATUS_TOGGLE, light_status.GetCheck() == BST_CHECKED);
}

void CLightPage::OnBnClickedLightingColor()
{
	ambient_color = ambient_color_button.GetColor();

	((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_COLOR_CHANGED, ambient_color);
}

void CLightPage::OnBnClickedAmbientColor()
{
	light_color = lighting_color_button.GetColor();

	((CControlBoard*)GetParentSheet())->SendLightMessage(WM_LIGHT_AMBIENT_COLOR_CHANGED, light_color);
}
void CLightPage::AddLightName(string lightName) 
{
	CString light_Name = string_to_wstring_mb(lightName).c_str();
	LightListBox.AddString(light_Name);
}
void CLightPage::ResetLightNameList() 
{
	LightListBox.ResetContent();
}
void CLightPage::SetSelectedLighting(INT value) 
{
	LightListBox.SetCurSel(value);
}
void CLightPage::UpdatePosition(XMFLOAT4 pos) 
{
	Light_X_Slider.SetPos((int)pos.x + (SCREEN_WIDTH / 2));
	Light_Y_Slider.SetPos((int)pos.y + (SCREEN_HEIGHT / 2));
	Light_Z_Slider.SetPos((int)pos.z + (SCREEN_WIDTH / 2));
	CString text;
	text.Format((LPCWSTR)L"%d", (int)pos.x);
	light_location_x.SetWindowTextW(text);
	text.Format((LPCWSTR)L"%d", (int)pos.y);
	light_location_y.SetWindowTextW(text);
	text.Format((LPCWSTR)L"%d", (int)pos.z);
	light_location_z.SetWindowTextW(text);
}
void CLightPage::UpdateTarget(XMFLOAT4 pos) 
{
	Light_Target_X_Slider.SetPos((int)pos.x + (SCREEN_WIDTH / 2));
	Light_Target_Y_Slider.SetPos((int)pos.y + (SCREEN_HEIGHT / 2));
	Light_Target_Z_Slider.SetPos((int)pos.z + (SCREEN_WIDTH / 2));
	CString text;
	text.Format((LPCWSTR)L"%d", (int)pos.x);
	light_target_x.SetWindowTextW(text);
	text.Format((LPCWSTR)L"%d", (int)pos.y);
	light_target_y.SetWindowTextW(text);
	text.Format((LPCWSTR)L"%d", (int)pos.z);
	light_target_z.SetWindowTextW(text);
}
void CLightPage::UpdateAmbientColor(XMFLOAT4 color) 
{
	uint32_t rgb = FRGB2RGB(color);
	ambient_color_button.SetColor(colorswap(rgb));
}
void CLightPage::UpdateAmbientIntensity(FLOAT intensity) 
{
	//FLOAT log_intensity = logf(intensity);
	//ambient_intensity.SetPos((int)(log_intensity * 100.0f));

	ambient_intensity.SetPos(static_cast<int>(intensity * 100.0f));
}
void CLightPage::UpdateLightIntensity(FLOAT intensity) 
{
	//FLOAT log_intensity = logf(intensity);
	//lighting_intensity.SetPos((int)(log_intensity * 100.0f));
	lighting_intensity.SetPos(static_cast<int>(intensity * 100.0f));
}
void CLightPage::UpdateLightColor(string color) 
{
	uint32_t rgb = FRGB2RGB(FindColor(color));
	lighting_color_button.SetColor(colorswap(rgb));
}
void CLightPage::UpdateLightType(string lightType)
{
//	lighting_style.SetCurSel(value);
}
void CLightPage::UpdateLightStatus(string status)
{
	light_status.SetCheck((status == "ON") ? BST_CHECKED : BST_UNCHECKED);
}
