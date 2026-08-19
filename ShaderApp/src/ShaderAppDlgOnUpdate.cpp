#include "pch.h"
#include <d3dcompiler.h>
#include "Resource.h"
#include "afxdialogex.h"
#include "framework.h"
#include "ShaderApp.h"
#include "ShaderAppDlg.h"
#pragma comment(lib, "d3dcompiler.lib")
#include "CommandQueue.h"
#include "CommandList.h"
#include <Material.h>
#include <Visitor.h>
#include <Mesh.h>
#include "Camera.h"
//#include "Lighting.h"
#include "Light.h"
#include "SceneVisitor.h"
#include "EffectPSO.h"
#include <RootSignature.h>
#include <Scene.h>
#include <SceneNode.h>
#include <SwapChain.h>
#include <Texture.h>
#include <functional>
#include <DirectXColors.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace dx12lib;


XMMATRIX XM_CALLCONV LookAtMatrix(FXMVECTOR Position, FXMVECTOR Direction, FXMVECTOR Up)
{
	assert(!XMVector3Equal(Direction, XMVectorZero()));
	assert(!XMVector3IsInfinite(Direction));
	assert(!XMVector3Equal(Up, XMVectorZero()));
	assert(!XMVector3IsInfinite(Up));

	XMVECTOR R2 = XMVector3Normalize(Direction);

	XMVECTOR R0 = XMVector3Cross(Up, R2);
	R0 = XMVector3Normalize(R0);

	XMVECTOR R1 = XMVector3Cross(R2, R0);

	XMMATRIX M(R0, R1, R2, Position);

	return M;
}






void CShaderAppDlg::OnUpdate(UpdateEventArgs ua)
{
	static double totalTime = 0.0;
	totalTime += ua.ElapsedTime;

	auto renderTarget = pSwapChain->GetRenderTarget();
	renderTarget.AttachTexture(AttachmentPoint::DepthStencil, pDepthTexture);

	auto viewport = renderTarget.GetViewport();

	DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixIdentity();
	// Rotation Toggle
	/*
	float angle = static_cast<float>(totalTime * 45.0);
	const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 0, 0);
	DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));
	*/

	/*
	DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(0, 0, -5, 1);
	DirectX::XMVECTOR focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
	const DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0, 1, 0, 0);

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

	float aspectRatio = viewport.Width / viewport.Height;
	DirectX::XMMATRIX projectionMatrix =
		DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.0f), aspectRatio, 0.1f, 100.0f);
	*/

	DirectX:XMMATRIX viewMatrix = pCurrentCamera->get_ViewMatrix();

	float aspectRatio = viewport.Width / viewport.Height;

	pCurrentCamera->set_Projection(
		45.0f,
		aspectRatio,
		0.1f,
		100.0f
	);

	DirectX::XMMATRIX projectionMatrix = pCurrentCamera->get_ProjectionMatrix();

	DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixTranspose(modelMatrix * viewMatrix * projectionMatrix);

	auto& commandQueue = m_Device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue.GetCommandList();

	switch (RenderStyle)
	{
	case WIREFRAME:
	case WIREFRAMEwPERSPECTIVE:
		commandList->SetPipelineState(pWireframePipelineStateObject);
		break;

	case RASTERIZE:
	case RASTERIZEwPERSPECTIVE:
	default:
		commandList->SetPipelineState(pPipelineStateObject);
		break;
	}
	commandList->SetGraphicsRootSignature(pRootSignature);
	commandList->SetGraphics32BitConstants(0, mvpMatrix);

	/*
	* The old clear background color, gonna add this later as the default.
	const FLOAT clearColor[] =
	{
		0.5f + 0.5f * sinf(t),
		0.1f,
		0.5f + 0.5f * cosf(t),
		1.0f
	};
	*/

	commandList->ClearTexture(renderTarget.GetTexture(AttachmentPoint::Color0), BackgroundColor);

	if (pDepthTexture)
		commandList->ClearDepthStencilTexture(pDepthTexture, D3D12_CLEAR_FLAG_DEPTH);

	commandList->SetRenderTarget(renderTarget);
	commandList->SetViewport(renderTarget.GetViewport());
	commandList->SetScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX));

	commandList->SetVertexBuffer(0, pVertexBuffer);
	commandList->SetIndexBuffer(pIndexBuffer);
	commandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawIndexed((UINT)pIndexBuffer->GetNumIndicies());

	commandQueue.ExecuteCommandList(commandList);
	pSwapChain->Present();
}

void CShaderAppDlg::UpdateSceneNode(shared_ptr<SceneNode> node, uint64_t frameCount)
{
	//std::string name = node->GetName(); // name of the main object
	//shared_ptr<Object> obj = ObjectList[name];
	//node->SetLocalTransform(obj->World());

	//if (obj->Iam() == "MULTIPLE MESH")
	//{
	//	shared_ptr<MultiMeshObject> m = std::static_pointer_cast<MultiMeshObject>(obj);

	//	for (auto& [key, mesh] : m->MeshList())
	//	{
	//		mesh->IsDirty() = mesh->Update(frameCount, GetCurrentCamera()->get_ViewMatrix());
	//	}

	//	for (auto& M : node->m_Meshes)
	//	{
	//		std::string meshName = M->Name();
	//		shared_ptr<MeshObject> mesh = m->MeshList()[meshName];

	//		if (mesh->IsDirty())
	//		{
	//			// Temporarily disable emissive material update to avoid linker issues.
	//			// XMFLOAT4 color = mesh->Color();
	//			// std::shared_ptr<Material> mat = M->GetMaterial();
	//			// mat->SetEmissiveColor(color);
	//		}
	//	}
	//}

	//if (obj->IsDirty())
	//{
	//	if (obj->Iam() == "MULTIPLE MESH")
	//	{
	//	}
	//	else if (obj->Iam() == "MESH")
	//	{
	//	}

	//	obj->IsDirty() = false;
	//}

	//XMMATRIX localTransform = node->GetLocalTransform();
	//XMMATRIX worldTransform = localTransform * obj->World();
	//node->SetLocalTransform(worldTransform);

	//for (auto& child : node->m_Children)
	//{
	//	UpdateSceneNode(child, frameCount);
	//}

	// Temporarily Disabled.
}

//void XM_CALLCONV ComputeMatrices(FXMMATRIX model, CXMMATRIX view, CXMMATRIX viewProjection, Mat& mat)
//{
 //   mat.ModelMatrix = model;
 //   mat.ModelViewMatrix = model * view;
 //   mat.InverseTransposeModelViewMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, mat.ModelViewMatrix));
 //   mat.ModelViewProjectionMatrix = model * viewProjection;
//}

void CShaderAppDlg::OnRender()
{
	auto& commandQueue = m_Device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	shared_ptr<CommandList> commandList = commandQueue.GetCommandList();
	CommandList& cmdListRef = *commandList;
	const auto& renderTarget = m_RenderTarget;
	Camera& theCamera = *CameraList[currentCamera];
	EffectPSO& p_LightingPSO = *m_LightingPSO;
	EffectPSO& p_DecalPSO = *m_DecalPSO;
	EffectPSO& p_UnlitPSO = *m_UnlitPSO;

	SceneVisitor opaquePass(cmdListRef, theCamera, p_LightingPSO, false);
	SceneVisitor transparentPass(cmdListRef, theCamera, p_DecalPSO, true);
	SceneVisitor unlitPass(cmdListRef, theCamera, p_UnlitPSO, false);

	// Clear the render targets.
	commandList->ClearTexture(renderTarget.GetTexture(AttachmentPoint::Color0), BackgroundColor);
	commandList->ClearDepthStencilTexture(
		renderTarget.GetTexture(AttachmentPoint::DepthStencil),
		D3D12_CLEAR_FLAG_DEPTH
	);

	commandList->SetViewport(m_Viewport);
	commandList->SetScissorRect(m_ScissorRect);
	commandList->SetRenderTarget(m_RenderTarget);

	TheScene->Accept(opaquePass);
	TheScene->Accept(transparentPass);

	// Temporarily disable material-based light visualization to avoid linker issues.
	/*
	auto lightMaterial = Material::findMaterial("White");
	for (const auto& [key, l] : PointLightList)
	{
		lightMaterial->Emissive = l->FRGB();
		lightMaterial->Ambient = { ambient,ambient,ambient,1 };
		auto lightPos = XMLoadFloat4(&l->PositionWS);
		auto worldMatrix = XMMatrixTranslationFromVector(lightPos);
	}

	for (const auto& [key, l] : SpotLightList)
	{
		lightMaterial->Emissive = l->FRGB();
		lightMaterial->Ambient = { ambient,ambient,ambient,1 };
		XMVECTOR lightPos = XMLoadFloat4(&l->PositionWS);
		XMVECTOR lightDir = XMLoadFloat4(&l->DirectionWS);
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);

		auto rotationMatrix = XMMatrixRotationX(XMConvertToRadians(0.0f));
		auto worldMatrix = rotationMatrix * LookAtMatrix(lightPos, lightDir, up);
	}
	*/

	// Resolve the MSAA render target to the swapchain's backbuffer.
	auto swapChainBackBuffer = pSwapChain->GetRenderTarget().GetTexture(AttachmentPoint::Color0);
	auto msaaRenderTarget = m_RenderTarget.GetTexture(AttachmentPoint::Color0);

	commandList->ResolveSubresource(swapChainBackBuffer, msaaRenderTarget);
	commandQueue.ExecuteCommandList(commandList);
	pSwapChain->Present();
}

void CShaderAppDlg::OnResized(UINT width, UINT height)
{
	m_Device.Flush();

	Width() = std::max(1U, width);
	Height() = std::max(1U, height);

	pSwapChain->Resize(Width(), Height());

	auto depthTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width(), Height());
	depthTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	pDepthTexture = m_Device.CreateTexture(depthTextureDesc, &optimizedClearValue);
}

void CShaderAppDlg::UpdateCamera(float DeltaTime)
{
#if 1
	// Update the camera.
	float speedMultipler = 0.1f;
//	if (pCurrentCamera == nullptr) pCurrentCamera = CameraList["Default Camera"];
//	XMVECTOR position = pCurrentCamera->CameraPositionVector();
//	XMVECTOR target = pCurrentCamera->TargetPositionVector();
//	XMVECTOR up = { 0,1,0,0 };
//	pCurrentCamera->set_LookAt(position, target, up);
//	XMVECTOR cameraTranslate = XMVectorSet(position.x, position.y, position.z + 10, 1.0f);
	XMVECTOR cameraTranslate = XMVectorSet(0.0f, 0.0f, 10.0f * speedMultipler * static_cast<float>(DeltaTime), 1.0f);
//	XMVECTOR cameraPan = XMVectorSet(0.0f, m_Up - m_Down, 0.0f, 1.0f) * speedMultipler * static_cast<float>(DeltaTime);
	pCurrentCamera->set_Translation(cameraTranslate);
//	pCurrentCamera->Translate(cameraPan, Space::Local);
//	XMVECTORF32 translation = { X, Y, Z };
//	m_Camera.Translate(translation, Space::World);
//	XMVECTOR cameraRotation = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pCurrentCamera->m_Pitch), XMConvertToRadians(pCurrentCamera->m_Yaw), 0.0f);
//	pCurrentCamera->set_Rotation(cameraRotation);

#else
//	float z = DeltaTime * 10.0f;
//	XMVECTOR eyePosition = XMVectorSet(0, 0, z, 1);
//	XMVECTOR focusPoint = XMVectorSet(0, 0, z + 1000, 1);
	const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);

	XMFLOAT3 eyePos = pCurrentCamera->CameraLocation();
	eyePos.z += 10.0f * DeltaTime;
	XMFLOAT3 targetPos = pCurrentCamera->TargetLocation();
	targetPos.z = eyePos.z + 1000.0f;
	pCurrentCamera->set_LookAt(XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 1), XMVectorSet(targetPos.x, targetPos.y, targetPos.z, 1), upDirection);


	XMFLOAT4 ToTaget = VECTOR({ targetPos.x, targetPos.y, targetPos.z, 1 },
							  { eyePos.x, eyePos.y, eyePos.z, 1 });
	XMVECTOR eyePosition = XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 1);
	XMVECTOR focusPoint = XMVectorSet(targetPos.x, targetPos.y, targetPos.z, 1);
	XMVECTOR eye2target = XMVectorSet(ToTaget.x, ToTaget.y, ToTaget.z, 1);
	m_FoV = pCurrentCamera->ZoomFloat();
	m_FoV = std::clamp(m_FoV, 12.0f, 90.0f);

	XMMATRIX       viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	pCurrentCamera->Translate(eye2target, Space::World);
//	pCurrentCamera->Translate(viewMatrix, Space::Local);
// Update the projection matrix.
	float    aspectRatio = static_cast<float>(Width()) / static_cast<float>(Height());

//	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);
//	XMMATRIX mvpMatrix = XMMatrixMultiply(modelMatrix, viewMatrix);
//	mvpMatrix = XMMatrixMultiply(mvpMatrix, projectionMatrix);




#endif
}

