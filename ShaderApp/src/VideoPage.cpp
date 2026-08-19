#include "pch.h"
#include "VideoPage.h"

BOOL CVideoPage::OnSetActive()
{
	CPropertyPage::OnSetActive();
	return TRUE;
}

void CVideoPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVideoPage, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CVideoPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
}