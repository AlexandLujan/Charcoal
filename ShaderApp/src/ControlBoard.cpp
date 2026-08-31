#include "pch.h"
#include "afxdialogex.h"
#include "ControlBoard.h"
#include "resource.h"
#include "Light.h"
#include "Utility.h"
#include "ShaderAppDlg.h"
#include "Const.h"

CControlBoard::CControlBoard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	parent = (CShaderAppDlg*) pParentWnd;
	AddPage(&contentPage);
	AddPage(&objectPage); // Add the second page
	AddPage(&cameraPage); // Add the second page
	AddPage(&lightPage); // Add the second page
	AddPage(&videoPage); // Add the second page
}

CControlBoard::~CControlBoard()
{
}

BOOL CControlBoard::OnInitDialog()
{
	CPropertySheet::OnInitDialog();
	SetActivePage(&objectPage);   // force control initialization
	SetActivePage(&cameraPage);   // force control initialization
	SetActivePage(&lightPage); // force control initialization
	SetActivePage(&videoPage);    // force control initialization
	SetActivePage(&contentPage);  // init and show
	return TRUE;
}

void CControlBoard::DoDataExchange(CDataExchange* pDX)
{
	CPropertySheet::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CControlBoard, CPropertySheet)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


void CControlBoard::SendContentMessage(UINT MessageID, uint32_t content)
{
	::PostMessage(parent->m_hWnd, WM_CONTENT_MESSAGE, MessageID, content);
}
void CControlBoard::SendContentMessage(UINT MessageID, string* content)
{
	::PostMessage(parent->m_hWnd, WM_CONTENT_MESSAGE, MessageID, (LPARAM) content);
}
void CControlBoard::SendCameraMessage(UINT MessageID, uint32_t content)
{
	::PostMessage(parent->m_hWnd, WM_CAMERA_MESSAGE, MessageID, content);
}
void CControlBoard::SendCameraMessage(UINT MessageID, string* content)
{
	::SendMessage(parent->m_hWnd, WM_CAMERA_MESSAGE, MessageID, (LPARAM)content);
	// Gonna refactor this to preserve asynchronous camera controls later, it'll require some
	// code review and modification of ownership of selection string.
}
void CControlBoard::SendLightMessage(UINT MessageID, uint32_t content)
{
	::PostMessage(parent->m_hWnd, WM_LIGHTING_MESSAGE, MessageID, static_cast<LPARAM>(content));
}
void CControlBoard::SendLightMessage(UINT MessageID, string* content)
{
	::SendMessage(parent->m_hWnd, WM_LIGHTING_MESSAGE, MessageID, reinterpret_cast<LPARAM>(content));
}
void CControlBoard::UpdateLightColor(XMFLOAT4 color)
{
	lightPage.UpdateLightColor(color);
}
void CControlBoard::SendObjectMessage(UINT MessageID, uint32_t content)
{
	::PostMessage(parent->m_hWnd, WM_OBJECT_MESSAGE, MessageID, content);
}
void CControlBoard::SendObjectMessage(UINT MessageID, string* content)
{
	::PostMessage(parent->m_hWnd, WM_OBJECT_MESSAGE, MessageID, (LPARAM)content);
}
void CControlBoard::SendVideoMessage(UINT MessageID, uint32_t content)
{
	::PostMessage(parent->m_hWnd, WM_VIDEO_MESSAGE, MessageID, content);
}
void CControlBoard::SendVideoMessage(UINT MessageID, string* content)
{
	::PostMessage(parent->m_hWnd, WM_VIDEO_MESSAGE, MessageID, (LPARAM)content);
}
