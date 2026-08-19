#pragma once
#include <afxdlgs.h>
#include "../resources.h"

class CVideoPage : public CPropertyPage
{
protected:
	virtual BOOL OnSetActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	CVideoPage() : CPropertyPage(IDD_VIDEO_PAGE)
	{

	}
	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

