#pragma once
#include "pch.h"
#include <afxdialogex.h>
#include "afxcolorbutton.h"

#include "../resources.h"
#include "CameraPage.h"
#include "ContentPage.h"
#include "LightPage.h"
#include "ObjectPage.h"
#include "Utility.h"
#include "VideoPage.h"

class CShaderAppDlg;

class CControlBoard : public CPropertySheet
{
public:
	CControlBoard(LPCTSTR pszCaption = NULL, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CControlBoard();
protected:
	CShaderAppDlg* parent;

	CContentPage	contentPage;
	CObjectPage		objectPage;
	CCameraPage		cameraPage;
	CLightPage		lightPage;
	CVideoPage		videoPage;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_CONTROL_BOARD
	};
#endif

protected:
	virtual BOOL OnInitDialog();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
//	HICON m_hIcon;
	CWnd* mainParent = nullptr;

public:
	void SendContentMessage(UINT MessageID, uint32_t content);
	void SendContentMessage(UINT MessageID, std::string* content);
	void SendCameraMessage(UINT MessageID, uint32_t content);
	void SendCameraMessage(UINT MessageID, std::string* content);
	void SendLightMessage(UINT MessageID, uint32_t content);
	void SendLightMessage(UINT MessageID, std::string* content);
	void SendObjectMessage(UINT MessageID, uint32_t content);
	void SendObjectMessage(UINT MessageID, std::string* content);
	void SendVideoMessage(UINT MessageID, uint32_t content);
	void SendVideoMessage(UINT MessageID, std::string* content);
	void UpdateContentSeedValue(uint32_t seed)
	{
		contentPage.UpdateSeedValue(seed);
	}
	void UpdateAddObjectName(std::string objName)
	{
		objectPage.AddObjectName(objName);
	}
	void ResetObjectNameList()
	{
		objectPage.ResetObjectNameList();
	}
	void SetSelectedObject(int value)
	{
		objectPage.SetSelectedObject(value);
	}
	void UpdateObjectPosition(XMFLOAT3 pos)
	{
		objectPage.UpdatePosition(pos);

	}
	void UpdateObjectRotation(XMFLOAT3 pos)
	{
		objectPage.UpdateRotation(pos);
	}

	void UpdateAddCameraName(std::string camName)
	{
		cameraPage.AddCameraName(camName);
	}
	void ResetCameraNameList()
	{
		cameraPage.ResetCameraNameList();
	}
	void SetSelectedCamera(int value)
	{
		cameraPage.SetSelectedCamera(value);
	}
	void UpdateCameraPosition(XMINT3 pos)
	{
		cameraPage.UpdatePosition(pos);

	}
	void UpdateCameraTarget(XMINT3 pos)
	{
		cameraPage.UpdateTarget(pos);
	}
	void UpdateAddLightingName(std::string lightName)
	{
		lightPage.AddLightName(lightName);
	}
	void ResetLightingNameList()
	{
		lightPage.ResetLightNameList();
	}
	void SetSelectedLighting(int value)
	{
		lightPage.SetSelectedLighting(value);
	}
	void UpdateLightPosition(XMFLOAT4 pos)
	{
		lightPage.UpdatePosition(pos);
	}
	void UpdateLightTarget(XMFLOAT4 pos)
	{
		lightPage.UpdateTarget(pos);
	}
	void UpdateLightingAmbientColor(XMFLOAT4 pos)
	{
		lightPage.UpdateAmbientColor(pos);
	}
	void UpdateLightingAmbientIntensity(FLOAT pos)
	{
		lightPage.UpdateAmbientIntensity(pos);
	}
	void UpdateLightColor(std::string color)
	{
		lightPage.UpdateLightColor(color);
	}
	void UpdateLightAmbientIntensity(FLOAT pos)
	{
		lightPage.UpdateLightIntensity(pos);
	}
	void UpdateLightType(std::string lightType)
	{
		lightPage.UpdateLightType(lightType);
	}

	void UpdateLightStatus(std::string status)
	{
		lightPage.UpdateLightStatus(status);
	}
};

