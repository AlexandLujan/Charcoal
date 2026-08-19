#pragma once
#include <afxdlgs.h>
#include "../resources.h"

class CLightPage : public CPropertyPage
{
	COLORREF ambient_color = 0;
	COLORREF light_color = 0;
protected:
	virtual BOOL OnSetActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	CLightPage() :CPropertyPage(IDD_LIGHTING_PAGE)
	{

	}
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_LIGHTING_PAGE
	};
#endif
	CComboBox LightListBox;
	CSliderCtrl Light_X_Slider;
	CSliderCtrl Light_Y_Slider;
	CSliderCtrl Light_Z_Slider;
	CSliderCtrl Light_Target_X_Slider;
	CSliderCtrl Light_Target_Y_Slider;
	CSliderCtrl Light_Target_Z_Slider;
	CMFCColorButton ambient_color_button;
	CSliderCtrl ambient_intensity;
	CEdit light_beam_angle;
	CButton light_status;
	CMFCColorButton lighting_color_button;
	CSliderCtrl lighting_intensity;
	CComboBox lighting_style;
	CStatic light_target_x;
	CStatic light_target_y;
	CStatic light_target_z;
	CStatic light_location_x;
	CStatic light_location_y;
	CStatic light_location_z;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelchangeLightsList();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedLightStatus();
	afx_msg void OnBnClickedLightingColor();
	afx_msg void OnBnClickedAmbientColor();
public:
	void AddLightName(std::string lightName);
	void ResetLightNameList();
	void SetSelectedLighting(INT value);
	void UpdatePosition(XMFLOAT4 pos);
	void UpdateTarget(XMFLOAT4 pos);
	void UpdateAmbientColor(XMFLOAT4 color);
	void UpdateAmbientIntensity(FLOAT intensity);
	void UpdateLightIntensity(FLOAT intensity);
	void UpdateLightColor(std::string color);
	void UpdateLightType(std::string lightType);
	void UpdateLightStatus(std::string status);

};
