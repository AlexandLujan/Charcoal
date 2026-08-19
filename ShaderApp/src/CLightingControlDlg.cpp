// CLightingControlDlg.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "CLightingControlDlg.h"
#include "Lighting.h"
#include "Utility.h"

map<string, Lighting*> LightingList;
vector<string> LightNames;
vector<uint32_t> LightColors;

// CLightingControlDlg dialog

IMPLEMENT_DYNAMIC(CLightingControlDlg, CDialogEx)

CLightingControlDlg::CLightingControlDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LIGHTINGCONTROL, nullptr)
{
	mainParent = (CWnd*)pParent;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CLightingControlDlg::~CLightingControlDlg()
{
}

void CLightingControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCCOLORBUTTON1, ColorButton);
	DDX_Control(pDX, IDC_LIGHT0_COLOR, Light_0_Color);
	DDX_Control(pDX, IDC_LIGHT1_COLOR, Light_1_Color);
	DDX_Control(pDX, IDC_LIGHT2_COLOR, Light_2_Color);

	DDX_Control(pDX, IDC_LIGHT0_X, Light_0_X);
	DDX_Control(pDX, IDC_LIGHT0_Y, Light_0_Y);
	DDX_Control(pDX, IDC_LIGHT0_Z, Light_0_Z);

	DDX_Control(pDX, IDC_LIGHT1_X, Light_1_X);
	DDX_Control(pDX, IDC_LIGHT1_Y, Light_1_Y);
	DDX_Control(pDX, IDC_LIGHT1_Z, Light_1_Z);

	DDX_Control(pDX, IDC_LIGHT2_X, Light_2_X);
	DDX_Control(pDX, IDC_LIGHT2_Y, Light_2_Y);
	DDX_Control(pDX, IDC_LIGHT2_Z, Light_2_Z);

	DDX_Control(pDX, IDC_TARGET0_X, Target_0_X);
	DDX_Control(pDX, IDC_TARGET0_Y, Target_0_Y);
	DDX_Control(pDX, IDC_TARGET0_Z, Target_0_Z);

	DDX_Control(pDX, IDC_TARGET1_X, Target_1_X);
	DDX_Control(pDX, IDC_TARGET1_Y, Target_1_Y);
	DDX_Control(pDX, IDC_TARGET1_Z, Target_1_Z);
	DDX_Control(pDX, IDC_TARGET2_X, Target_2_X);
	DDX_Control(pDX, IDC_TARGET2_Y, Target_2_Y);
	DDX_Control(pDX, IDC_TARGET2_Z, Target_2_Z);

	DDX_Control(pDX, IDC_LIGHT0_ON, Light_0_ON);
	DDX_Control(pDX, IDC_LIGHT1_ON, Light_1_ON);
	DDX_Control(pDX, IDC_LIGHT2_ON, Light_2_ON);
	DDX_Control(pDX, IDC_AMBIENT_BLUE, AmbientBlueSlider);
	DDX_Control(pDX, IDC_AMBIENT_GREEN, AmbientGreenSlider);
	DDX_Control(pDX, IDC_AMBIENT_RED, AmbientRedSlider);
	DDX_Control(pDX, IDC_LIGHT0_NAME, Light_0_Name);
	DDX_Control(pDX, IDC_LIGHT2_NAME, Light_2_Name);
	DDX_Control(pDX, IDC_LIGHT1_NAME, Light_1_Name);
	DDX_Control(pDX, IDC_AMBIENT_COLOR, Ambient_Color);

}


BEGIN_MESSAGE_MAP(CLightingControlDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_MFCCOLORBUTTON1, &CLightingControlDlg::OnBnClickedMfccolorbutton1)
	ON_BN_CLICKED(IDC_LIGHT0_COLOR, &CLightingControlDlg::OnBnClickedLight0Color)
	ON_BN_CLICKED(IDC_LIGHT1_COLOR, &CLightingControlDlg::OnBnClickedLight1Color)
	ON_BN_CLICKED(IDC_LIGHT2_COLOR, &CLightingControlDlg::OnBnClickedLight2Color)
	ON_BN_CLICKED(IDC_AMBIENT_COLOR, &CLightingControlDlg::OnBnClickedAmbientColor)
	ON_EN_CHANGE(IDC_LIGHT0_X, &CLightingControlDlg::OnEnChangeLight0_X)
	ON_EN_CHANGE(IDC_LIGHT1_X, &CLightingControlDlg::OnEnChangeLight1_X)
	ON_EN_CHANGE(IDC_LIGHT2_X, &CLightingControlDlg::OnEnChangeLight2_X)
	ON_EN_CHANGE(IDC_LIGHT0_Y, &CLightingControlDlg::OnEnChangeLight0_Y)
	ON_EN_CHANGE(IDC_LIGHT1_Y, &CLightingControlDlg::OnEnChangeLight1_Y)
	ON_EN_CHANGE(IDC_LIGHT2_Y, &CLightingControlDlg::OnEnChangeLight2_Y)
	ON_EN_CHANGE(IDC_LIGHT0_Z, &CLightingControlDlg::OnEnChangeLight0_Z)
	ON_EN_CHANGE(IDC_LIGHT1_Z, &CLightingControlDlg::OnEnChangeLight1_Z)
	ON_EN_CHANGE(IDC_LIGHT2_Z, &CLightingControlDlg::OnEnChangeLight2_Z)
	ON_EN_CHANGE(IDC_TARGET0_X, &CLightingControlDlg::OnEnChangeTarget0_X)
	ON_EN_CHANGE(IDC_TARGET1_X, &CLightingControlDlg::OnEnChangeTarget1_X)
	ON_EN_CHANGE(IDC_TARGET2_X, &CLightingControlDlg::OnEnChangeTarget2_X)
	ON_EN_CHANGE(IDC_TARGET0_Y, &CLightingControlDlg::OnEnChangeTarget0_Y)
	ON_EN_CHANGE(IDC_TARGET1_Y, &CLightingControlDlg::OnEnChangeTarget1_Y)
	ON_EN_CHANGE(IDC_TARGET2_Y, &CLightingControlDlg::OnEnChangeTarget2_Y)
	ON_EN_CHANGE(IDC_TARGET0_Z, &CLightingControlDlg::OnEnChangeTarget0_Z)
	ON_EN_CHANGE(IDC_TARGET1_Z, &CLightingControlDlg::OnEnChangeTarget1_Z)
	ON_EN_CHANGE(IDC_TARGET2_Z, &CLightingControlDlg::OnEnChangeTarget2_Z)
	ON_BN_CLICKED(IDC_LIGHT0_ON, &CLightingControlDlg::OnBnClickedLight0_On)
	ON_BN_CLICKED(IDC_LIGHT1_ON, &CLightingControlDlg::OnBnClickedLight1_On)
	ON_BN_CLICKED(IDC_LIGHT2_ON, &CLightingControlDlg::OnBnClickedLight2_On)

END_MESSAGE_MAP()

// CLightingControlDlg message handlers
CRect light_0_rect;
CRect light_1_rect;
CRect light_2_rect;
CRect ambient_rect;
//extern DWORD LightingControlTable[3][9];
//{
//	{ IDC_LIGHT0_COLOR,IDC_LIGHT0_NAME,IDC_LIGHT0_ON,IDC_LIGHT0_X,IDC_LIGHT0_Y,IDC_LIGHT0_Z,IDC_TARGET0_X,IDC_TARGET0_Y,IDC_TARGET0_Z},
//	{ IDC_LIGHT1_COLOR,IDC_LIGHT1_NAME,IDC_LIGHT1_ON,IDC_LIGHT1_X,IDC_LIGHT1_Y,IDC_LIGHT1_Z,IDC_TARGET1_X,IDC_TARGET1_Y,IDC_TARGET1_Z},
//	{ IDC_LIGHT2_COLOR,IDC_LIGHT2_NAME,IDC_LIGHT2_ON,IDC_LIGHT2_X,IDC_LIGHT2_Y,IDC_LIGHT2_Z,IDC_TARGET2_X,IDC_TARGET2_Y,IDC_TARGET2_Z}
//};

BOOL CLightingControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ShowWindow(SW_SHOWNORMAL);

	AmbientRedSlider.SetRange(0, 255, TRUE);
	AmbientGreenSlider.SetRange(0, 255, TRUE);
	AmbientBlueSlider.SetRange(0, 255, TRUE);
	Light_0_Color.GetWindowRect(&light_0_rect);
	Light_1_Color.GetWindowRect(&light_1_rect);
	Light_2_Color.GetWindowRect(&light_2_rect);
	Ambient_Color.GetWindowRect(&ambient_rect);
	Light_0_Color.ModifyStyle(0, BS_BITMAP);
	Light_1_Color.ModifyStyle(0, BS_BITMAP);
	Light_2_Color.ModifyStyle(0, BS_BITMAP);

	light_0_rect.bottom -= light_0_rect.top;
	light_0_rect.top = 0;
	light_0_rect.left -= light_0_rect.left;
	light_0_rect.left = 0;

	light_1_rect.bottom -= light_1_rect.top;
	light_1_rect.top = 0;
	light_1_rect.left -= light_1_rect.left;
	light_1_rect.left = 0;

	light_2_rect.bottom -= light_2_rect.top;
	light_2_rect.top = 0;
	light_2_rect.left -= light_2_rect.left;
	light_2_rect.left = 0;

	ambient_rect.bottom -= ambient_rect.top;
	ambient_rect.top = 0;
	ambient_rect.left -= ambient_rect.left;
	ambient_rect.left = 0;
	return TRUE;  // return TRUE  unless you set the focus to a control


}
void CLightingControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	CDialogEx::OnPaint();
}

HCURSOR CLightingControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLightingControlDlg::OnBnClickedMfccolorbutton1()
{
	COLORREF c = ColorButton.GetColor();
	backgroundColor = colorswap(c);
	ClearCanvas();
	Invalidate();
	::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE, LIGHT_BACKGROUND_COLOR_CHANGED, backgroundColor);
}

void CLightingControlDlg::OnClose()
{
	CDialog::OnClose();
}
void CLightingControlDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	DestroyWindow();
	delete this;
}

void setColorButton(CButton& colorButton, CRect& rect, uint32_t color);


void CLightingControlDlg::OnBnClickedLight0Color()
{
	CColorDialog cd;
	if (cd.DoModal() == IDOK)
	{
		COLORREF clr = cd.GetColor();
		setColorButton(Light_0_Color, light_0_rect, clr);
		vector<float> intensity = { (float)GetRValue(clr) / 255.0f,(float)GetGValue(clr) / 255.0f,(float)GetBValue(clr) / 255.0f };
		LightingList[LightNames[0]]->setIntensity(intensity);
		LightColors[0] = colorswap(clr);
		::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE, LIGHT_0_COLOR_CHANGED, LightColors[0]);
	}
}

void CLightingControlDlg::OnBnClickedLight1Color()
{
	CColorDialog cd;
	if (cd.DoModal() == IDOK)
	{
		COLORREF clr = cd.GetColor();
		setColorButton(Light_1_Color, light_1_rect, clr);
		vector<float> intensity = { (float)GetRValue(clr) / 255.0f,(float)GetGValue(clr) / 255.0f,(float)GetBValue(clr) / 255.0f };
		LightingList[LightNames[1]]->setIntensity(intensity);
		LightColors[1] = colorswap(clr);
		::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE, LIGHT_1_COLOR_CHANGED, LightColors[1]);
	}
}

void CLightingControlDlg::OnBnClickedLight2Color()
{
	CColorDialog cd;
	if (cd.DoModal() == IDOK)
	{
		COLORREF clr = cd.GetColor();
		setColorButton(Light_2_Color, light_2_rect, clr);
		vector<float> intensity = { (float)GetRValue(clr) / 255.0f,(float)GetGValue(clr) / 255.0f,(float)GetBValue(clr) / 255.0f };
		LightingList[LightNames[2]]->setIntensity(intensity);
		LightColors[2] = colorswap(clr);
		::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE, LIGHT_2_COLOR_CHANGED, LightColors[2]);
	}
}

void CLightingControlDlg::OnBnClickedAmbientColor()
{
	CColorDialog cd;
	if (cd.DoModal() == IDOK)
	{
		COLORREF clr = cd.GetColor();
		setColorButton(Ambient_Color, ambient_rect, clr);
		vector<float> intensity = { (float)GetRValue(clr) / 255.0f,(float)GetGValue(clr) / 255.0f,(float)GetBValue(clr) / 255.0f };
		LightingList["Ambient"]->setIntensity(intensity);
	}
}

void setColorButton(CButton& colorButton, CRect& rect, uint32_t color)
{
	CDC memDC;
	memDC.CreateCompatibleDC(colorButton.GetDC());
	CPen pen(PS_SOLID, 1, color); // Red pen, 1 pixel solid
	CBrush brush(color); // Blue brush
	CPen* pOldPen = memDC.SelectObject(&pen);
	CBrush* pOldBrush = memDC.SelectObject(&brush);
	CBitmap myBitmap;
	myBitmap.CreateBitmap(rect.Width(), rect.Height(), 1, 32, nullptr);
	CBitmap* pOldBitmap = memDC.SelectObject(&myBitmap);
	memDC.Rectangle(&rect);
	memDC.SelectObject(pOldPen);
	memDC.SelectObject(pOldBrush);
	memDC.SelectObject(pOldBitmap);

	colorButton.SetBitmap(myBitmap.operator HBITMAP()); // .operator HBITMAP());
}


void CLightingControlDlg::OnEnChangeLight0_X()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_0_POSITION_X_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeLight1_X()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_1_POSITION_X_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeLight2_X()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_2_POSITION_X_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeLight0_Y()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_0_POSITION_Y_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeLight1_Y()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_1_POSITION_Y_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeLight2_Y()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_2_POSITION_Y_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeLight0_Z()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_0_POSITION_Z_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeLight1_Z()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_1_POSITION_Z_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeLight2_Z()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_2_POSITION_Z_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget0_X()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_0_TARGET_X_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget1_X()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_1_TARGET_X_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget2_X()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_2_TARGET_X_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget0_Y()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_0_TARGET_Y_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget1_Y()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_1_TARGET_Y_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget2_Y()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_2_TARGET_Y_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget0_Z()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_0_TARGET_Z_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget1_Z()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_1_TARGET_Z_CHANGED, 0);
}
void CLightingControlDlg::OnEnChangeTarget2_Z()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_2_TARGET_Z_CHANGED, 0);
}
void CLightingControlDlg::OnBnClickedLight0_On()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_0_ON_CHANGED, 0);
}
void CLightingControlDlg::OnBnClickedLight1_On()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_1_ON_CHANGED, 0);
}
void CLightingControlDlg::OnBnClickedLight2_On()
{
::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE,LIGHT_2_ON_CHANGED, 0);
}

void CLightingControlDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	WPARAM SliderID = pScrollBar->GetDlgCtrlID();
	CString text;
	text.Format(_T("%d"), (int)nPos);
	if (nSBCode == TB_THUMBTRACK)
	{
		switch (SliderID)
		{
			case IDC_AMBIENT_BLUE: //AmbientBlueSlider.GetDlgCtrlID())
			{
				AmbientBlueValue = nPos;
				COLORREF clr = windowsColor(AmbientRedValue, AmbientGreenValue, AmbientBlueValue);
				setColorButton(Ambient_Color, ambient_rect, clr);
				::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE, LIGHT_AMBIENT_COLOR_CHANGED, clr);
			}
			break;
			case IDC_AMBIENT_GREEN: //AmbientGreenSlider.GetDlgCtrlID())
			{
				AmbientGreenValue = nPos;
				COLORREF clr = windowsColor(AmbientRedValue, AmbientGreenValue, AmbientBlueValue);
				setColorButton(Ambient_Color, ambient_rect, clr);
				::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE, LIGHT_AMBIENT_COLOR_CHANGED, clr);
			}
			break;
			case IDC_AMBIENT_RED: //AmbientRedSlider.GetDlgCtrlID())
			{
				AmbientRedValue = nPos;
				COLORREF clr = windowsColor(AmbientRedValue, AmbientGreenValue, AmbientBlueValue);
				setColorButton(Ambient_Color, ambient_rect, clr);
				::PostMessage(mainParent->m_hWnd, WM_LIGHT_MESSAGE, LIGHT_AMBIENT_COLOR_CHANGED, clr);
			}
			break;
		}
	}
}

