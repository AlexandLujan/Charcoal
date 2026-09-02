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
#include <fstream>

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
	auto viewport = renderTarget.GetViewport();

	DirectX::XMMATRIX modelMatrix =
		DirectX::XMMatrixIdentity();

	DirectX::XMMATRIX viewMatrix =
		pCurrentCamera->get_ViewMatrix();

	float aspectRatio =
		viewport.Width / viewport.Height;

	pCurrentCamera->set_Projection(
		45.0f,
		aspectRatio,
		0.1f,
		1000.0f
	);

	DirectX::XMMATRIX projectionMatrix =
		pCurrentCamera->get_ProjectionMatrix();

	m_LightingPSO->SetWorldMatrix(modelMatrix);
	m_LightingPSO->SetViewMatrix(viewMatrix);
	m_LightingPSO->SetProjectionMatrix(projectionMatrix);
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
	//CommandList& cmdListRef = *commandList;
	const auto& renderTarget = m_SceneRenderTarget;
	//Camera& theCamera = *CameraList[currentCamera];

	/*
	EffectPSO& p_LightingPSO = *m_LightingPSO;
	EffectPSO& p_DecalPSO = *m_DecalPSO;
	EffectPSO& p_UnlitPSO = *m_UnlitPSO;

	SceneVisitor opaquePass(cmdListRef, theCamera, p_LightingPSO, false);
	SceneVisitor transparentPass(cmdListRef, theCamera, p_DecalPSO, true);
	SceneVisitor unlitPass(cmdListRef, theCamera, p_UnlitPSO, false);
	*/

	std::ofstream log("bloom_debug_log.txt", std::ios::app);

	log << "[OnRender] Begin\n";
	log.flush();

	// Clear the render targets.
	commandList->ClearTexture(renderTarget.GetTexture(AttachmentPoint::Color0), BackgroundColor);

	log << "[OnRender] HDR color cleared\n";
	log.flush();

	commandList->ClearDepthStencilTexture(
		renderTarget.GetTexture(AttachmentPoint::DepthStencil),
		D3D12_CLEAR_FLAG_DEPTH
	);

	log << "[OnRender] Depth cleared\n";
	log.flush();

	commandList->SetViewport(m_Viewport);
	commandList->SetScissorRect(m_ScissorRect);
	commandList->SetRenderTarget(m_SceneRenderTarget);

	//
	// HEX CORRIDOR - REGULAR TOP GEOMETRY
	//

	commandList->SetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	log << "[OnRender] Applying regular top material\n";
	log.flush();

	m_LightingPSO->SetMaterial(
		pRegularTopMaterial
	);

	m_LightingPSO->Apply(
		*commandList
	);

	log << "[OnRender] Regular top PSO applied\n";
	log.flush();

	commandList->SetVertexBuffer(
		0,
		pRegularTopVertexBuffer
	);

	commandList->SetIndexBuffer(
		pRegularTopIndexBuffer
	);

	commandList->DrawIndexed(
		static_cast<UINT>(
			pRegularTopIndexBuffer->GetNumIndicies()
			)
	);

	log << "[OnRender] Regular top draw recorded\n";
	log.flush();


	//
	// HEX CORRIDOR - REGULAR SIDES GEOMETRY
	//

	log << "[OnRender] Applying regular sides material\n";
	log.flush();

	m_LightingPSO->SetMaterial(
		pRegularSidesMaterial
	);

	m_LightingPSO->Apply(
		*commandList
	);

	log << "[OnRender] Regular sides PSO applied\n";
	log.flush();

	commandList->SetVertexBuffer(
		0,
		pRegularSidesVertexBuffer
	);

	commandList->SetIndexBuffer(
		pRegularSidesIndexBuffer
	);

	commandList->DrawIndexed(
		static_cast<UINT>(
			pRegularSidesIndexBuffer->GetNumIndicies()
			)
	);

	log << "[OnRender] Regular sides draw recorded\n";
	log.flush();

	//
	// HEX CORRIDOR - EMISSIVE GEOMETRY
	//

	log << "[OnRender] Applying emissive material\n";
	log.flush();

	m_LightingPSO->SetMaterial(pEmissiveMaterial);
	m_LightingPSO->Apply(*commandList);

	log << "[OnRender] Emissive PSO applied\n";
	log.flush();

	commandList->SetVertexBuffer(
		0,
		pEmissiveVertexBuffer
	);

	commandList->SetIndexBuffer(
		pEmissiveIndexBuffer
	);

	commandList->DrawIndexed(
		static_cast<UINT>(
			pEmissiveIndexBuffer->GetNumIndicies()
			)
	);

	
	/// BRIGHT PASS
	commandList->ClearTexture(
		m_BrightPassRenderTarget.GetTexture(AttachmentPoint::Color0),
		DirectX::Colors::Black
	);

	commandList->SetRenderTarget(
		m_BrightPassRenderTarget
	);

	commandList->SetViewport(
		m_BrightPassRenderTarget.GetViewport()
	);

	commandList->SetScissorRect(
		CD3DX12_RECT(
			0,
			0,
			LONG_MAX,
			LONG_MAX
		)
	);

	commandList->SetPipelineState(
		pBrightPassPSO
	);

	commandList->SetGraphicsRootSignature(
		pCompositeRootSignature
	);

	commandList->SetShaderResourceView(
		0,
		0,
		pSceneRenderTarget,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	commandList->SetShaderResourceView(
		0,
		1,
		pSceneRenderTarget,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	commandList->SetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	commandList->Draw(3);

	log << "[OnRender] Emissive draw recorded\n";
	log.flush();

	log << "[OnRender] Executing command list\n";
	log.flush();

	//commandQueue.ExecuteCommandList(commandList);

	log << "[OnRender] Command list executed\n";
	log.flush();

	// TheScene->Accept(opaquePass);
	// TheScene->Accept(transparentPass);

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

	//
	// HORIZONTAL BLUR PASS
	//

	commandList->ClearTexture(
		m_BlurHorizontalRenderTarget.GetTexture(
			AttachmentPoint::Color0
		),
		DirectX::Colors::Black
	);

	commandList->SetRenderTarget(
		m_BlurHorizontalRenderTarget
	);

	commandList->SetViewport(
		m_BlurHorizontalRenderTarget.GetViewport()
	);

	commandList->SetScissorRect(
		CD3DX12_RECT(
			0,
			0,
			LONG_MAX,
			LONG_MAX
		)
	);

	commandList->SetPipelineState(
		pBlurHorizontalPSO
	);

	commandList->SetGraphicsRootSignature(
		pCompositeRootSignature
	);

	commandList->SetShaderResourceView(
		0,
		0,
		pBrightPassRenderTarget,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	commandList->SetShaderResourceView(
		0,
		1,
		pBrightPassRenderTarget,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	commandList->SetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	commandList->Draw(3);


	//
	// VERTICAL BLUR PASS
	//

	commandList->ClearTexture(
		m_BlurVerticalRenderTarget.GetTexture(
			AttachmentPoint::Color0
		),
		DirectX::Colors::Black
	);

	commandList->SetRenderTarget(
		m_BlurVerticalRenderTarget
	);

	commandList->SetViewport(
		m_BlurVerticalRenderTarget.GetViewport()
	);

	commandList->SetScissorRect(
		CD3DX12_RECT(
			0,
			0,
			LONG_MAX,
			LONG_MAX
		)
	);

	commandList->SetPipelineState(
		pBlurVerticalPSO
	);

	commandList->SetGraphicsRootSignature(
		pCompositeRootSignature
	);

	commandList->SetShaderResourceView(
		0,
		0,
		pBlurHorizontalRenderTarget,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	commandList->SetShaderResourceView(
		0,
		1,
		pBlurHorizontalRenderTarget,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	commandList->SetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	commandList->Draw(3);
	
	//
	// COMPOSITE PASS
	//

	auto swapChainRenderTarget =
		pSwapChain->GetRenderTarget();

	// Switch output from HDR scene target to swap-chain back buffer.
	commandList->SetRenderTarget(
		swapChainRenderTarget
	);

	commandList->SetViewport(
		swapChainRenderTarget.GetViewport()
	);

	commandList->SetScissorRect(
		CD3DX12_RECT(
			0,
			0,
			LONG_MAX,
			LONG_MAX
		)
	);

	// Composite pipeline.
	commandList->SetPipelineState(
		pCompositePSO
	);

	commandList->SetGraphicsRootSignature(
		pCompositeRootSignature
	);

	// Bind the HDR scene texture to t0.
	commandList->SetShaderResourceView(
		0,
		0,
		pSceneRenderTarget,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	// Bind the final blurred bloom texture to t1.
	commandList->SetShaderResourceView(
		0,
		1,
		pBlurVerticalRenderTarget,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	commandList->SetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	// Fullscreen triangle.
	commandList->Draw(3);

	commandQueue.ExecuteCommandList(
		commandList
	);
	

	pSwapChain->Present();

	log << "[OnRender] Presented\n";
	log.flush();
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

	//
	// HDR SCENE TARGET
	//

	D3D12_CLEAR_VALUE sceneClearValue = {};
	sceneClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	sceneClearValue.Color[0] = 0.0f;
	sceneClearValue.Color[1] = 0.0f;
	sceneClearValue.Color[2] = 0.0f;
	sceneClearValue.Color[3] = 1.0f;

	auto sceneTextureDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			Width(),
			Height(),
			1,
			1,
			1,
			0,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		);

	pSceneRenderTarget =
		m_Device.CreateTexture(
			sceneTextureDesc,
			&sceneClearValue
		);

	//
	// REATTACH
	//

	m_SceneRenderTarget.AttachTexture(
		AttachmentPoint::Color0,
		pSceneRenderTarget
	);

	m_SceneRenderTarget.AttachTexture(
		AttachmentPoint::DepthStencil,
		pDepthTexture
	);

	//
	// BRIGHT PASS RENDER TARGET
	//

	D3D12_CLEAR_VALUE brightPassClearValue = {};
	brightPassClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	brightPassClearValue.Color[0] = 0.0f;
	brightPassClearValue.Color[1] = 0.0f;
	brightPassClearValue.Color[2] = 0.0f;
	brightPassClearValue.Color[3] = 1.0f;

	auto brightPassTextureDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			Width(),
			Height(),
			1,
			1,
			1,
			0,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		);

	pBrightPassRenderTarget =
		m_Device.CreateTexture(
			brightPassTextureDesc,
			&brightPassClearValue
		);

	m_BrightPassRenderTarget.AttachTexture(
		AttachmentPoint::Color0,
		pBrightPassRenderTarget
	);

	//
// HORIZONTAL BLUR RENDER TARGET
//

	D3D12_CLEAR_VALUE blurHorizontalClearValue = {};
	blurHorizontalClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	blurHorizontalClearValue.Color[0] = 0.0f;
	blurHorizontalClearValue.Color[1] = 0.0f;
	blurHorizontalClearValue.Color[2] = 0.0f;
	blurHorizontalClearValue.Color[3] = 1.0f;

	auto blurHorizontalTextureDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			Width(),
			Height(),
			1,
			1,
			1,
			0,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		);

	pBlurHorizontalRenderTarget =
		m_Device.CreateTexture(
			blurHorizontalTextureDesc,
			&blurHorizontalClearValue
		);

	m_BlurHorizontalRenderTarget.AttachTexture(
		AttachmentPoint::Color0,
		pBlurHorizontalRenderTarget
	);

	//
	// VERTICAL BLUR RENDER TARGET
	//

	D3D12_CLEAR_VALUE blurVerticalClearValue = {};
	blurVerticalClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	blurVerticalClearValue.Color[0] = 0.0f;
	blurVerticalClearValue.Color[1] = 0.0f;
	blurVerticalClearValue.Color[2] = 0.0f;
	blurVerticalClearValue.Color[3] = 1.0f;

	auto blurVerticalTextureDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			Width(),
			Height(),
			1,
			1,
			1,
			0,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		);

	pBlurVerticalRenderTarget =
		m_Device.CreateTexture(
			blurVerticalTextureDesc,
			&blurVerticalClearValue
		);

	m_BlurVerticalRenderTarget.AttachTexture(
		AttachmentPoint::Color0,
		pBlurVerticalRenderTarget
	);
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
	//eyePos.z += 10.0f * DeltaTime;
	XMFLOAT3 targetPos = pCurrentCamera->TargetLocation();
	//targetPos.z = eyePos.z + 1000.0f;
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

