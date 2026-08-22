
// ShaderAppDlg.h : header file
//

/*
* NOTE: Yeah, this and its respective .cpp file are going to be
* the bane of my existence.
*/

#pragma once
#include "pch.h"
#include <vector>
#include <Device.h>
#include <IndexBuffer.h> 
#include <PipelineStateObject.h>
#include <RenderTarget.h>
#include <RootSignature.h>
#include <Scene.h>
#include <SceneNode.h>
#include <SwapChain.h>
#include <Texture.h>   
#include <VertexBuffer.h>
#include "resources.h"
#include "Const.h"
#include "ControlBoard.h"
#include "EffectPSO.h"
#include "Events.h"
#include "HighResolutionTimer.h"
#include "Light.h"
#include "nlohmann/json.hpp"
#include "Object.h"
#include "Utility.h"

using namespace dx12lib;
using namespace DirectX;
using json = nlohmann::json;

class EffectPSO;

// Something I added for the screensaver.
struct SimpleParticle
{
	float x;
	float y;
	float vx;
	float vy;
	COLORREF color;
	int size;
};

struct DirectionalLightCB
{
	DirectX::XMFLOAT4 Direction;
	DirectX::XMFLOAT4 Color;

	float Ambient;
	float DiffuseIntensity;
	DirectX::XMFLOAT2 Padding;
};

// CShaderAppDlg dialog
class CShaderAppDlg : public CDialogEx
{
	// Construction
public:
	CShaderAppDlg(dx12lib::Device& device, CWnd* pParent = nullptr, bool vSync = false);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHADERAPP_DIALOG };
#endif

	static const UINT BufferCount = 3;

private:
	Device& m_Device;
	INT screenWidth;
	INT screenHeight;
	bool g_IsInitialized = false;
	static LONG m_windowStyle;
	static RECT m_windowRect;
	float m_aspectRatio = 0;
	float m_curRotationAngleRad = 0;
	bool m_vSync = false;
	bool m_enableUI = 0;
	UINT m_adapterIDoverride = 0;
	RECT m_windowBounds = {};
	float m_FoV;
	static const UINT FrameCount = 3;
	CDC m_MemDC;
	CBitmap m_BackBuffer;
	CBitmap* m_OldBitmap = nullptr;
	bool m_BufferReady = false;

	void CreateBackBuffer(CDC* pDC);

public:
	INT& Width() { return screenWidth; }
	INT& Height() { return screenHeight; }
	Device& GetDevice() { return m_Device; }
	shared_ptr<Camera> GetCurrentCamera() { return pCurrentCamera; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	// Implementation
protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnDestroy();
	afx_msg void OnSizeChanged(UINT width, UINT height, bool minimized);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnScriptMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCameraMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnContentMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLightMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnVideoMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnObjectMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()

	void OnUpdate(UpdateEventArgs ua);
	void UpdateForSizeChange(UINT width, UINT height);
	BOOL Initialize();
	void OnResized(UINT width, UINT height);

private:
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	DirectX::XMMATRIX m_ModelMatrix;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjectionMatrix;

	bool m_ContentLoaded;
	bool m_IsTearingSupported = false;

	std::shared_ptr<SwapChain>           pSwapChain = nullptr;
	std::shared_ptr<Texture>             pDepthTexture = nullptr;
	std::shared_ptr<VertexBuffer>        pVertexBuffer = nullptr;
	std::shared_ptr<IndexBuffer>         pIndexBuffer = nullptr;
	std::shared_ptr<RootSignature>       pRootSignature = nullptr;
	std::shared_ptr<PipelineStateObject> pPipelineStateObject = nullptr;
	std::shared_ptr<PipelineStateObject> pWireframePipelineStateObject = nullptr;
	HighResolutionTimer m_Timer;

	FLOAT BackgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	string currentCamera = "Default Camera";
	shared_ptr<Camera> pCurrentCamera = nullptr;
	void UpdateCamera(float frameNumber);

	shared_ptr<EffectPSO> m_LightingPSO;
	shared_ptr<EffectPSO> m_DecalPSO;
	shared_ptr<EffectPSO> m_UnlitPSO;

	map<string, shared_ptr<PointLight>>		 PointLightList;
	map<string, shared_ptr<SpotLight>>		 SpotLightList;
	map<string, shared_ptr<DirectionalLight>> DirectionalLightList;

	bool m_AnimateLights;
	FLOAT ambient = 0.2f;

	RenderTarget m_RenderTarget;
	void OnRender();

	shared_ptr<Scene> m_Scene;

	map<string, shared_ptr<Scene>> Thing;

	class g_Object;

	json document;
	json viewpoint;
	json templates;
	json objects;
	json setup;
	json cameras;
	json lights;
	json actions;

	void ReadJson(string filename);

	void BuildJSON(string filename);
	void BuildViewPoint(json& viewpoint);
	void BuildTemplates(json& templates);
	void BuildObjects(json& objects);
	void ObjectActions(json& setup);
	void BuildCameras(json& cameras);
	void BuildLights(json& lights);
	void BuildActions(json& actions);

	void RegisterLight(const std::string& name, const std::shared_ptr<Light>& light);

	shared_ptr<Object> BuildCircle(json& element, string name);
	shared_ptr<Object> BuildSquare(json& element, string name);
	shared_ptr<Object> BuildRectangle(json& element, string name);
	shared_ptr<Object> BuildPolygon(json& element, string name);
	shared_ptr<Object> BuildText(json& element, string name);
	shared_ptr<Object> BuildMesh(json& element, string name);
	shared_ptr<Object> BuildOBJFileMesh(json& element, string name);
	shared_ptr<Object> BuildTemplateReference(json& element, string name);
	shared_ptr<Object> BuildFractal(json& element, string name);
	shared_ptr<Object> BuildBox(json& element, string name);
	shared_ptr<Object> BuildTunnel(json& element, string name);
	shared_ptr<Object> BuildRoundTunnel(json& element, string name);
	shared_ptr<Object> BuildTriangleTunnel(json& element, string name);

	map<string, shared_ptr<Object>>   TemplateList;
	map<string, shared_ptr<Object>>   ObjectList;
	map<string, shared_ptr<Camera>>   CameraList;
	map<string, shared_ptr<Light>>    LightList;
	map<string, shared_ptr<Lighting>> LightingList;

	void UpdateObjectRotation(shared_ptr<Object> o);
	void UpdateObjectPositionBoxes(shared_ptr<Object> o);

	shared_ptr<Object> GetTemplate(string name) { return TemplateList[name]; }

	RenderStyles RenderStyle = WIREFRAME;
	CControlBoard* controlBoard = nullptr;
	string objectName = "";
	string cameraName = "";
	string LightName = "";
	string LightingName = "";
	int frame_rate = 1;
	shared_ptr<Light> currentLight = nullptr;

	void TurnOnLight(string name) { LightList[name]->TurnOn(); }
	void TurnOffLight(string name) { LightList[name]->TurnOff(); }

	XMFLOAT4 eyeLocation = { 0,0,0,0 };

	shared_ptr<SceneNode> CreateSceneMesh(string objectName, shared_ptr<CommandList> commandList);
	shared_ptr<SceneNode> CreateSceneMultipleMesh(string objectName, shared_ptr<CommandList> commandList);

	XMMATRIX identity = XMMatrixIdentity();
	shared_ptr<Scene> TheScene = std::make_shared<Scene>();
	shared_ptr<SceneNode> RootNode = std::make_shared<SceneNode>();
	std::mt19937 gen{ static_cast<unsigned int>(std::time(0)) };
	std::string RandomColor();
	void UpdateSceneNode(shared_ptr<SceneNode> node, uint64_t frameCount);

	// Simple particle system
	std::vector<SimpleParticle> m_Particles;
	bool m_ParticlesInitialized = false;
	void InitParticles();
	void UpdateParticles();
};