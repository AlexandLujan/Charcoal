#pragma once
#include <afxdlgs.h>
#include <string>
#include "../resources.h"

using namespace std;

class CContentPage : public CPropertyPage
{
protected:
	virtual BOOL OnSetActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	CContentPage() :CPropertyPage(IDD_CONTENT_PAGE)
	{

	}
//	~CContentPage();
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONTENT_PAGE };
#endif
	CStatic FramesPerSecond;
	CSliderCtrl FrameRateSlider;
	CEdit fileNameBox;
	CComboBox RenderStyleList;
	CEdit FractalSeedValueBox;
	CButton Show_XY_Axis_CheckBox;
	CButton Show_Grid_CheckBox;
	CMFCColorButton ColorButton;

	bool show_Grid = true;
	bool show_XY_Axis = true;
	std::string json_file_name = "";
	int render_style = 0;
	COLORREF backgroundColor = 0;
	int frame_rate = 20;

	int& FrameRate() { return frame_rate; }
	COLORREF& BackgroundColor() { return backgroundColor; }
	std::string& JSON_FileName() { return json_file_name; }
	bool& GridToggle() { return show_Grid; }
	bool& AXISToggle() { return show_XY_Axis; }
	void UpdateSeedValue(uint32_t seed);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedShowGridCheck();
	afx_msg void OnBnClickedShowXyCheck();
	afx_msg void OnBnClickedReadJsonButton();
	afx_msg void OnCbnSelchangeRenderStyle();
	afx_msg void OnBnClickedColorbutton();
	afx_msg void OnBnClickedHexCorridorButton();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

