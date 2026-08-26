#include "pch.h"
#include <d3dcompiler.h>
#include "Resource.h"
#include "SceneGeometry.h"
#include "scenes/HexCorridorScene.h" // Remove in Charcoal ALPHA 0.60
#include "afxdialogex.h"
#include "framework.h"
#include "ShaderApp.h"
#include "ShaderAppDlg.h"
#pragma comment(lib, "d3dcompiler.lib")
#include "CommandQueue.h"
#include "CommandList.h"
#include "Camera.h"
#include "Lighting.h"
//#include "OBJ_3D_Model.h"
#include <VertexTypes.h>
#include <Helpers.h>
#include <fstream>
#include <filesystem>
#include <windows.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace dx12lib;

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace dx12lib;

std::filesystem::path GetExecutableDirectory()
{
    wchar_t path[MAX_PATH];

    GetModuleFileNameW(nullptr, path, MAX_PATH);

    return std::filesystem::path(path).parent_path();
}

// Clamp a value between a min and max range.
template<typename T>
constexpr const T& clamp(const T& val, const T& min, const T& max)
{
	return val < min ? min : val > max ? max : val;
}
#if 1
//map<string, uint32_t> sectionList = { {"one",12},{"two",13} };
struct VertexPosColor
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Color;
};

static VertexPosColor g_Vertices[8] =
{
	{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) },
	{ DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
	{ DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) },
	{ DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
	{ DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) },
	{ DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) },
	{ DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) },
	{ DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) }
};

static UINT g_Indicies[36] =
{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7
};
#else
#include "Camry.h"
#endif

//OBJ_3D_Model TheModel;

BOOL CShaderAppDlg::Initialize()
{
    std::ofstream log("sandbox_debug_log.txt", std::ios::app);

    log << "[Initialize] Start\n";
    log.flush();

    log << "[Initialize] Creating swap chain\n";
    log.flush();

    pSwapChain = m_Device.CreateSwapChain(m_hWnd, DXGI_FORMAT_R8G8B8A8_UNORM);
    pSwapChain->SetVSync(false);

    log << "[Initialize] Swap chain created\n";
    log.flush();

    log << "[Initialize] Getting COPY command queue\n";
    log.flush();

    auto& commandQueue = m_Device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList = commandQueue.GetCommandList();

    HexCorridorGeometry scene = BuildHexCorridorScene();

    //
    // REGULAR GEOMETRY
    //

    log << "[Initialize] Copying regular vertex buffer\n";
    log.flush();

    pRegularVertexBuffer = commandList->CopyVertexBuffer(
        scene.Regular.Vertices.size(),
        sizeof(SceneVertex),
        scene.Regular.Vertices.data()
    );

    log << "[Initialize] Regular vertex buffer copied\n";
    log.flush();

    log << "[Initialize] Copying regular index buffer\n";
    log.flush();

    pRegularIndexBuffer = commandList->CopyIndexBuffer(
        scene.Regular.Indices.size(),
        DXGI_FORMAT_R32_UINT,
        scene.Regular.Indices.data()
    );

    log << "[Initialize] Regular index buffer copied\n";
    log.flush();


    //
    // EMISSIVE GEOMETRY
    //

    log << "[Initialize] Copying emissive vertex buffer\n";
    log.flush();

    pEmissiveVertexBuffer = commandList->CopyVertexBuffer(
        scene.Emissive.Vertices.size(),
        sizeof(SceneVertex),
        scene.Emissive.Vertices.data()
    );

    log << "[Initialize] Emissive vertex buffer copied\n";
    log.flush();

    log << "[Initialize] Copying emissive index buffer\n";
    log.flush();

    pEmissiveIndexBuffer = commandList->CopyIndexBuffer(
        scene.Emissive.Indices.size(),
        DXGI_FORMAT_R32_UINT,
        scene.Emissive.Indices.data()
    );

    log << "[Initialize] Emissive index buffer copied\n";
    log.flush();

    //
// MATERIALS
//

    log << "[Initialize] Creating regular material\n";
    log.flush();

    pRegularMaterial = std::make_shared<Material>();

    pRegularMaterial->SetDiffuseColor(
        { 1.0f, 1.0f, 1.0f, 1.0f });

    pRegularMaterial->SetAmbientColor(
        { 0.22f, 0.22f, 0.22f, 1.0f });

    pRegularMaterial->SetSpecularColor(
        { 0.15f, 0.15f, 0.15f, 1.0f });

    pRegularMaterial->SetEmissiveColor(
        { 0.0f, 0.0f, 0.0f, 1.0f });

    pRegularMaterial->SetBumpIntensity(
        { 2.0f }
    );

    log << "[Initialize] Regular material created\n";
    log.flush();


    log << "[Initialize] Creating emissive material\n";
    log.flush();

    pEmissiveMaterial = std::make_shared<Material>();

    pEmissiveMaterial->SetDiffuseColor(
        { 0.0f, 0.0f, 0.0f, 1.0f });

    pEmissiveMaterial->SetAmbientColor(
        { 0.0f, 0.0f, 0.0f, 1.0f });

    pEmissiveMaterial->SetSpecularColor(
        { 0.0f, 0.0f, 0.0f, 1.0f });

    pEmissiveMaterial->SetEmissiveColor(
        { 1.0f, 0.03f, 0.0f, 1.0f });

    log << "[Initialize] Emissive material created\n";
    log.flush();


    //
    // EXECUTE COPY COMMAND LIST
    //

    log << "[Initialize] Executing copy command list\n";
    log.flush();

    commandQueue.ExecuteCommandList(commandList);

    log << "[Initialize] Copy command list executed\n";
    log.flush();

    auto& textureCommandQueue =
        m_Device.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    auto textureCommandList =
        textureCommandQueue.GetCommandList();

    const auto texturePath =
        GetExecutableDirectory()
        / L".."
        / L".."
        / L"ShaderApp"
        / L"Textures"
        / L"VolcanicRock"
        / L"volcanic-rock1-albedo.png";

    const auto normalTexturePath =
        GetExecutableDirectory()
        / L".."
        / L".."
        / L"ShaderApp"
        / L"Textures"
        / L"VolcanicRock"
        / L"volcanic-rock1-normal-ogl.png";

    const auto heightTexturePath =
        GetExecutableDirectory()
        / L".."
        / L".."
        / L"ShaderApp"
        / L"Textures"
        / L"VolcanicRock"
        / L"volcanic-rock1-height.png";

    auto regularTexture =
        textureCommandList->LoadTextureFromFile(
            texturePath.wstring(),
            true
        );

    pRegularMaterial->SetTexture(
        Material::TextureType::Diffuse,
        regularTexture
    );

    auto normalTexture =
        textureCommandList->LoadTextureFromFile(
            normalTexturePath.wstring(),
            false
        );

    // pRegularMaterial->SetTexture(
//     Material::TextureType::Normal,
//     normalTexture
// );

    auto heightTexture =
        textureCommandList->LoadTextureFromFile(
            heightTexturePath.wstring(),
            false
        );

    pRegularMaterial->SetTexture(
        Material::TextureType::Bump,
        heightTexture
    );

    textureCommandQueue.ExecuteCommandList(
        textureCommandList
    );

    textureCommandQueue.Flush();

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    CD3DX12_ROOT_PARAMETER1 rootParameters[2]{};
    rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    log << "[Initialize] Creating root signature\n";
    log.flush();

    pRootSignature = m_Device.CreateRootSignature(rootSignatureDescription.Desc_1_1);

    log << "[Initialize] Root signature created\n";
    log.flush();

    const std::filesystem::path executableDirectory = GetExecutableDirectory();

    const std::filesystem::path vertexShaderPath =
        executableDirectory / L"VertexShader.cso";

    const std::filesystem::path pixelShaderPath =
        executableDirectory / L"PixelShader.cso";

    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;

    log << "[Initialize] Loading VertexShader.cso from: "
        << vertexShaderPath.string() << "\n";
    log.flush();

    ThrowIfFailed(
        D3DReadFileToBlob(vertexShaderPath.c_str(), &vertexShaderBlob)
    );

    log << "[Initialize] VertexShader.cso loaded\n";
    log.flush();

    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;

    log << "[Initialize] Loading PixelShader.cso from: "
        << pixelShaderPath.string() << "\n";
    log.flush();

    ThrowIfFailed(
        D3DReadFileToBlob(pixelShaderPath.c_str(), &pixelShaderBlob)
    );

    log << "[Initialize] PixelShader.cso loaded\n";
    log.flush();

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencilState;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK           SampleMask;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream = {};

    pipelineStateStream.pRootSignature = pRootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);
    rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pipelineStateStream.RasterizerState = rasterizerState;

    pipelineStateStream.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineStateStream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pipelineStateStream.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
    pipelineStateStream.SampleMask = UINT_MAX;

    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    auto rtvFormats = pSwapChain->GetRenderTarget().GetRenderTargetFormats();

    log << "[Initialize] RTV NumRenderTargets = " << rtvFormats.NumRenderTargets << "\n";

    for (UINT i = 0; i < rtvFormats.NumRenderTargets; ++i)
    {
        log << "[Initialize] RTV Formats[" << i << "] = " << rtvFormats.RTFormats[i] << "\n";
    }
    log.flush();

    pipelineStateStream.RTVFormats = rtvFormats;

    log << "[Initialize] Creating pipeline state object\n";
    log.flush();

    try {
        pPipelineStateObject = m_Device.CreatePipelineStateObject(pipelineStateStream);
    }
    catch (const std::exception& e)
    {
        log << "[Initialize] CreatePipelineStateObject threw std::exception: " << e.what() << "\n";
        log.flush();
        return FALSE;
    }
    catch (...)
    {
        log << "[Initialize] CreatePipelineStateObject threw unknown exception\n";
        log.flush();
        return FALSE;
    }

    log << "[Initialize] Pipeline state object created\n";
    log.flush();

    // Wireframe PSO
    log << "[Initialize] Creating wireframe pipeline state object\n";
    log.flush();

    CD3DX12_RASTERIZER_DESC wireframeRasterizer(D3D12_DEFAULT);
    wireframeRasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;

    pipelineStateStream.RasterizerState = wireframeRasterizer;

    try
    {
        pWireframePipelineStateObject = m_Device.CreatePipelineStateObject(pipelineStateStream);
    }
    catch (const std::exception& e)
    {
        log << "[Initialize] Wireframe CreatePipelineStateObject threw std::exception: " << e.what() << "\n";
        log.flush();
        return FALSE;
    }
    catch (...)
    {
        log << "[Initialize] Wireframe CreatePipelineStateObject threw unknown exception\n";
        log.flush();
        return FALSE;
    }

    log << "[Initialize] Wireframe pipeline state object created\n";
    log.flush();

    log << "[Initialize] Creating depth texture\n";
    log.flush();

    auto depthTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width(), Height());
    depthTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil = { 1.0f, 0 };

    pDepthTexture = m_Device.CreateTexture(depthTextureDesc, &optimizedClearValue);

    log << "[Initialize] Depth texture created\n";
    log.flush();

    log << "[Initialize] Creating lighting EffectPSO\n";
    log.flush();

    m_LightingPSO =
        std::make_shared<EffectPSO>(
            m_Device,
            true,   // enable lighting
            false   // disable decal
        );

    log << "[Initialize] Lighting EffectPSO created\n";
    log.flush();

    log << "[Initialize] Flushing command queue\n";
    log.flush();

    commandQueue.Flush();

    log << "[Initialize] Finished successfully\n";
    log.flush();

    return TRUE;
}