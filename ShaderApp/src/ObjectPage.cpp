#include "pch.h"
#include "ContentPage.h"
#include "resource.h"
#include <string>
#include "Utility.h"
#include "ControlBoard.h"

using namespace std;
BOOL CObjectPage::OnSetActive()
{
	CPropertyPage::OnSetActive();
	ObjectListBox.AddString(L"No Objects Yet");
	Pitch_Scroll.SetRange(0, 360);
	Roll_Scroll.SetRange(0, 360);
	Yaw_Scroll.SetRange(0, 360);
	Pitch_Scroll.SetPos(180);
	Roll_Scroll.SetPos(180);
	Yaw_Scroll.SetPos(180);
	return TRUE;
}

void CObjectPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OBJECT_LIST, ObjectListBox);
	DDX_Control(pDX, IDC_OBJECT_X_BOX, Object_X_Box);
	DDX_Control(pDX, IDC_OBJECT_Y_BOX, Object_Y_Box);
	DDX_Control(pDX, IDC_OBJECT_Z_BOX, Object_Z_Box);
	DDX_Control(pDX, IDC_PITCH_SCROLL, Pitch_Scroll);
	DDX_Control(pDX, IDC_ROLL_SCROLL, Roll_Scroll);
	DDX_Control(pDX, IDC_YAW_SCROLL, Yaw_Scroll);
}

BEGIN_MESSAGE_MAP(CObjectPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_OBJECT_LIST, &CObjectPage::OnCbnSelchangeObjectList)
	ON_BN_CLICKED(IDC_SET_X_BUTTON, &CObjectPage::OnBnClickedSetXButton)
	ON_BN_CLICKED(IDC_SET_Y_BUTTON, &CObjectPage::OnBnClickedSetYButton)
	ON_BN_CLICKED(IDC_SET_Z_BUTTON, &CObjectPage::OnBnClickedSetZButton)
END_MESSAGE_MAP()
#define WM_OBJECT_CHANGED						0
#define WM_OBJECT_X_CHANGED						1
#define WM_OBJECT_Y_CHANGED						2
#define WM_OBJECT_Z_CHANGED						3
#define WM_OBJECT_PITCH_CHANGED					4
#define WM_OBJECT_YAW_CHANGED					5
#define WM_OBJECT_ROLL_CHANGED					6

void CObjectPage::OnCbnSelchangeObjectList()
{
	CString str;
	int sel = ObjectListBox.GetCurSel();
	ObjectListBox.GetLBText(sel, str);
	string objectName = ConvertToStrStd(str);
	((CControlBoard*)GetParentSheet())->SendObjectMessage(WM_OBJECT_CHANGED, &objectName);
}

void CObjectPage::OnBnClickedSetXButton()
{
	CString s_value;
	Object_X_Box.GetWindowText(s_value);
	((CControlBoard*)GetParentSheet())->SendObjectMessage(WM_OBJECT_X_CHANGED, _ttoi(s_value));
}

void CObjectPage::OnBnClickedSetYButton()
{
	CString s_value;
	Object_Y_Box.GetWindowText(s_value);
	((CControlBoard*)GetParentSheet())->SendObjectMessage(WM_OBJECT_Y_CHANGED, _ttoi(s_value));
}

void CObjectPage::OnBnClickedSetZButton()
{
	CString s_value;
	Object_Z_Box.GetWindowText(s_value);
	((CControlBoard*)GetParentSheet())->SendObjectMessage(WM_OBJECT_Z_CHANGED, _ttoi(s_value));
}

void CObjectPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	WPARAM SliderID = pScrollBar->GetDlgCtrlID();
	int slideValue = (int)nPos;
	if (nSBCode == TB_THUMBTRACK)
	{
		switch (SliderID)
		{
		case IDC_PITCH_SCROLL:
		{
			int PitchTo = (slideValue - 180);
			((CControlBoard*)GetParentSheet())->SendObjectMessage(WM_OBJECT_PITCH_CHANGED, PitchTo);
		}
		break;
		case IDC_YAW_SCROLL:
		{
			int YawTo = (slideValue - 180);
			((CControlBoard*)GetParentSheet())->SendObjectMessage(WM_OBJECT_YAW_CHANGED, YawTo);
		}
		break;
		case IDC_ROLL_SCROLL:
		{
			int RollTo = (slideValue - 180);
			((CControlBoard*)GetParentSheet())->SendObjectMessage(WM_OBJECT_ROLL_CHANGED, RollTo);
		}
		break;
		}
	}
}

void CObjectPage::AddObjectName(string objName)
{
	CString obj_Name = string_to_wstring_mb(objName).c_str();
	ObjectListBox.AddString(obj_Name);
}
void CObjectPage::ResetObjectNameList()
{
	ObjectListBox.ResetContent();
}
void CObjectPage::SetSelectedObject(int value)
{
	ObjectListBox.SetCurSel(value);
}
void CObjectPage::UpdatePosition(XMFLOAT3 pos)
{
	CString s_value;
	s_value.Format(_T("%7.3f"), pos.x);
	Object_X_Box.SetWindowText(s_value);
	s_value.Format(_T("%7.3f"), pos.y);
	Object_Y_Box.SetWindowText(s_value);
	s_value.Format(_T("%7.3f"), pos.z);
	Object_Z_Box.SetWindowText(s_value);

}
void CObjectPage::UpdateRotation(XMFLOAT3 pos)
{
	Pitch_Scroll.SetPos((int)RadiansToDegrees(pos.x) + 180);
	Yaw_Scroll.SetPos((int)RadiansToDegrees(pos.y) + 180);
	Roll_Scroll.SetPos((int)RadiansToDegrees(pos.z) + 180);
}
