#include "pch.h"
#include <string>
#include "resource.h"
#include "ControlBoard.h"
#include "ContentPage.h"
#include "Utility.h"
#include "Const.h"

using namespace std;

BOOL CContentPage::OnSetActive()
{
	CPropertyPage::OnSetActive();
	FrameRateSlider.SetRange(1, 80);
	FrameRateSlider.SetPos(1);
	FramesPerSecond.SetWindowText(L"1");
	RenderStyleList.ResetContent();
	RenderStyleList.AddString(L"Wireframe");
	RenderStyleList.AddString(L"Wireframe w/ Perspective");
	RenderStyleList.AddString(L"Rasterize");
	RenderStyleList.AddString(L"Rasterize w/ Perspective");
	RenderStyleList.AddString(L"Ray Trace");
	RenderStyleList.AddString(L"Ray Trace Z");
	RenderStyleList.SetCurSel(0);
	FractalSeedValueBox.SetWindowTextW(L"Fractals Not Used");
	return TRUE;
}

void CContentPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAME_BOX, FramesPerSecond);
	DDX_Control(pDX, IDC_FRAMERATE, FrameRateSlider);
	DDX_Control(pDX, IDC_JSON_FILENAME_BOX, fileNameBox);
	DDX_Control(pDX, IDC_RENDER_STYLE, RenderStyleList);
	DDX_Control(pDX, IDC_FRACTAL_SEED_VALUE, FractalSeedValueBox);
	DDX_Control(pDX, IDC_SHOW_XY_CHECK, Show_XY_Axis_CheckBox);
	DDX_Control(pDX, IDC_SHOW_GRID_CHECK, Show_Grid_CheckBox);
	DDX_Control(pDX, IDC_COLORBUTTON, ColorButton);
}

BEGIN_MESSAGE_MAP(CContentPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_SHOW_GRID_CHECK, &CContentPage::OnBnClickedShowGridCheck)
	ON_BN_CLICKED(IDC_SHOW_XY_CHECK, &CContentPage::OnBnClickedShowXyCheck)
	ON_BN_CLICKED(IDC_READ_JSON_BUTTON, &CContentPage::OnBnClickedReadJsonButton)
	ON_CBN_SELCHANGE(IDC_RENDER_STYLE, &CContentPage::OnCbnSelchangeRenderStyle)
	ON_BN_CLICKED(IDC_COLORBUTTON, &CContentPage::OnBnClickedColorbutton)
	ON_BN_CLICKED(IDC_HEX_CORRIDOR_BUTTON, &CContentPage::OnBnClickedHexCorridorButton)
END_MESSAGE_MAP()

void CContentPage::OnBnClickedShowGridCheck()
{
	show_Grid = Show_Grid_CheckBox.GetCheck();
	((CControlBoard*)GetParentSheet())->SendContentMessage(WM_CONTENT_GRID_TOGGLE, show_Grid);
}

void CContentPage::OnBnClickedShowXyCheck()
{
	show_XY_Axis = Show_XY_Axis_CheckBox.GetCheck();
	((CControlBoard*)GetParentSheet())->SendContentMessage(WM_CONTENT_AXIS_TOGGLE, show_XY_Axis);
}


void CContentPage::OnBnClickedReadJsonButton()
{
	const TCHAR szFilter[] = _T("json Files (*.json)|*.json|All Files (*.*)|*.*||");
	CFileDialog fd(true, _T("json"), NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, szFilter, this);
	if (fd.DoModal() == IDOK)
	{
		CString sFilePath = fd.GetPathName();
		fileNameBox.SetWindowText(sFilePath);
		CT2CA pszConvertedAnsiString(sFilePath);
		string str(pszConvertedAnsiString);
		json_file_name = str;
		// send file name back to sandbox to read and process
		((CControlBoard*)GetParentSheet())->SendContentMessage(WM_CONTENT_JSON_FILENAME,&json_file_name);
	}
}

void CContentPage::OnCbnSelchangeRenderStyle()
{
	render_style = RenderStyleList.GetCurSel();
	((CControlBoard*)GetParentSheet())->SendContentMessage(WM_CONTENT_RENDER_STYLE_CHANGE, render_style);
}

// Charcoal ALPHA 0.30
void CContentPage::OnBnClickedHexCorridorButton()
{
	((CControlBoard*)GetParentSheet())->SendContentMessage(
		WM_CONTENT_HEX_CORRIDOR,
		static_cast<uint32_t>(0)
	);
}

void CContentPage::OnBnClickedColorbutton()
{
	backgroundColor = ColorButton.GetColor();

	((CControlBoard*)GetParentSheet())->SendContentMessage(WM_CONTENT_BACKGROUND_COLOR_CHANGE, backgroundColor);
}

void CContentPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	WPARAM SliderID = pScrollBar->GetDlgCtrlID();
	CString text;
	int slideValue = FrameRateSlider.GetPos();

	if (nSBCode == TB_THUMBTRACK)
	{
		if (SliderID == IDC_FRAMERATE)
		{
			frame_rate = 1000 / slideValue;
			CString frameText;
			frameText.Format(_T("%d"), slideValue);
			FramesPerSecond.SetWindowText(frameText);
			((CControlBoard*)GetParentSheet())->SendContentMessage(WM_CONTENT_FRAME_RATE_CHANGE, frame_rate);
		}
	}
}
void CContentPage::UpdateSeedValue(uint32_t seed)
{
	FractalSeedValueBox.SetWindowTextW(std::to_wstring(seed).c_str());
}



