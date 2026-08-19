#pragma once
#include <afxdlgs.h>
#include "resource.h"

class CCameraPage : public CPropertyPage
{
protected:
	virtual BOOL OnSetActive();

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	CCameraPage() :CPropertyPage(IDD_CAMERA_PAGE)
	{

	}
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMERA_PAGE };
#endif
	void AddCameraName(std::string camName);
	void ResetCameraNameList();
	void SetSelectedCamera(int value);
	void UpdatePosition(XMINT3 pos);
	void UpdateTarget(XMINT3 pos);

	CComboBox CameraListBox;
	CSliderCtrl Camera_X_Slider;
	CSliderCtrl Camera_Y_Slider;
	CSliderCtrl Camera_Z_Slider;
	CSliderCtrl Target_X_Slider;
	CSliderCtrl Target_Y_Slider;
	CSliderCtrl Target_Z_Slider;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelchangeCameraList();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CStatic camera_target_x;
	CStatic camera_target_y;
	CStatic camera_target_z;
	CStatic camera_location_x;
	CStatic camera_location_y;
	CStatic camera_location_z;
};

