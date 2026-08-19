#pragma once
#include <afxdlgs.h>
#include "../resources.h"

class CObjectPage : public CPropertyPage
{
protected:
	virtual BOOL OnSetActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	CObjectPage() :CPropertyPage(IDD_OBJECT_PAGE)
	{

	}
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OBJECT_PAGE };
#endif
	void AddObjectName(string objName);
	void ResetObjectNameList();
	void SetSelectedObject(int value);
	void UpdatePosition(XMFLOAT3 pos);
	void UpdateRotation(XMFLOAT3 pos);

	CComboBox ObjectListBox;
	CEdit Object_X_Box;
	CEdit Object_Y_Box;
	CEdit Object_Z_Box;
	CSliderCtrl Pitch_Scroll;
	CSliderCtrl Roll_Scroll;
	CSliderCtrl Yaw_Scroll;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelchangeObjectList();
	afx_msg void OnBnClickedSetXButton();
	afx_msg void OnBnClickedSetYButton();
	afx_msg void OnBnClickedSetZButton();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

