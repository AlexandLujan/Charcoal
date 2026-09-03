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

void CShaderAppDlg::ClearDebugLogs()
{
    const char* logFiles[] =
    {
        "bloom_debug_log.txt",
        "sandbox_debug_log.txt"
    };

    for (const char* file : logFiles)
    {
        std::ofstream(file, std::ios::trunc).close();
    }
}

BOOL CShaderAppDlg::Initialize()
{
    ClearDebugLogs();
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
    // REGULAR TOP GEOMETRY
    //

    log << "[Initialize] Copying regular top vertex buffer\n";
    log.flush();

    pRegularTopVertexBuffer = commandList->CopyVertexBuffer(
        scene.RegularTop.Vertices.size(),
        sizeof(SceneVertex),
        scene.RegularTop.Vertices.data()
    );

    log << "[Initialize] Regular top vertex buffer copied\n";
    log.flush();

    log << "[Initialize] Copying regular top index buffer\n";
    log.flush();

    pRegularTopIndexBuffer = commandList->CopyIndexBuffer(
        scene.RegularTop.Indices.size(),
        DXGI_FORMAT_R32_UINT,
        scene.RegularTop.Indices.data()
    );

    log << "[Initialize] Regular top index buffer copied\n";
    log.flush();


    //
    // REGULAR SIDES GEOMETRY
    //

    log << "[Initialize] Copying regular sides vertex buffer\n";
    log.flush();

    pRegularSidesVertexBuffer = commandList->CopyVertexBuffer(
        scene.RegularSides.Vertices.size(),
        sizeof(SceneVertex),
        scene.RegularSides.Vertices.data()
    );

    log << "[Initialize] Regular sides vertex buffer copied\n";
    log.flush();

    log << "[Initialize] Copying regular sides index buffer\n";
    log.flush();

    pRegularSidesIndexBuffer = commandList->CopyIndexBuffer(
        scene.RegularSides.Indices.size(),
        DXGI_FORMAT_R32_UINT,
        scene.RegularSides.Indices.data()
    );

    log << "[Initialize] Regular sides index buffer copied\n";
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

//
// REGULAR TOP MATERIAL
//

    log << "[Initialize] Creating regular top material\n";
    log.flush();

    pRegularTopMaterial = std::make_shared<Material>();

    pRegularTopMaterial->SetDiffuseColor(
        { 1.0f, 1.0f, 1.0f, 1.0f });

    pRegularTopMaterial->SetAmbientColor(
        { 0.22f, 0.22f, 0.22f, 1.0f });

    pRegularTopMaterial->SetSpecularColor(
        { 0.15f, 0.15f, 0.15f, 1.0f });

    pRegularTopMaterial->SetEmissiveColor(
        { 0.0f, 0.0f, 0.0f, 1.0f });

    pRegularTopMaterial->SetBumpIntensity(
        { 1.25f }
    );

    log << "[Initialize] Regular top material created\n";
    log.flush();


    //
    // REGULAR SIDES MATERIAL
    //

    log << "[Initialize] Creating regular sides material\n";
    log.flush();

    pRegularSidesMaterial = std::make_shared<Material>();

    pRegularSidesMaterial->SetDiffuseColor(
        { 1.0f, 1.0f, 1.0f, 1.0f });

    pRegularSidesMaterial->SetAmbientColor(
        { 0.22f, 0.22f, 0.22f, 1.0f });

    pRegularSidesMaterial->SetSpecularColor(
        { 0.15f, 0.15f, 0.15f, 1.0f });

    pRegularSidesMaterial->SetEmissiveColor(
        { 0.0f, 0.0f, 0.0f, 1.0f });

    pRegularSidesMaterial->SetBumpIntensity(
        { 0.25f }
    );

    log << "[Initialize] Regular sides material created\n";
    log.flush();


    //
    // EMISSIVE MATERIAL
    //

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
        { 1.0f, 1.0f, 1.0f, 1.0f });

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

    const auto textureDirectory =
        GetExecutableDirectory()
        / L"Textures"
        / L"RoughRock";

    const auto albedoTexturePath =
        textureDirectory / L"roughrockface2_Base_Color.png";

    const auto normalTexturePath =
        textureDirectory / L"roughrockface2_Normal.png";

    const auto heightTexturePath =
        textureDirectory / L"roughrockface2_Height.png";

    const auto roughnessTexturePath =
        textureDirectory / L"roughrockface2_Roughness.png";

    const auto ambientOcclusionTexturePath =
        textureDirectory / L"roughrockface2_Ambient_Occlusion.png";

    auto albedoTexture =
        textureCommandList->LoadTextureFromFile(
            albedoTexturePath.wstring(),
            true
        );

    
    auto normalTexture =
        textureCommandList->LoadTextureFromFile(
            normalTexturePath.wstring(),
            false
        );
    

    auto heightTexture =
        textureCommandList->LoadTextureFromFile(
            heightTexturePath.wstring(),
            false
        );

    auto roughnessTexture =
        textureCommandList->LoadTextureFromFile(
            roughnessTexturePath.wstring(),
            false
        );

    auto ambientOcclusionTexture =
        textureCommandList->LoadTextureFromFile(
            ambientOcclusionTexturePath.wstring(),
            false
        );

    pRegularTopMaterial->SetTexture(
        Material::TextureType::Diffuse,
        albedoTexture
    );

    
    pRegularTopMaterial->SetTexture(
        Material::TextureType::Normal,
        normalTexture
    );
    

    pRegularTopMaterial->SetTexture(
        Material::TextureType::Bump,
        heightTexture
    );

    pRegularTopMaterial->SetTexture(
        Material::TextureType::Roughness,
        roughnessTexture
    );

    pRegularSidesMaterial->SetTexture(
        Material::TextureType::Diffuse,
        albedoTexture
    );

    pRegularTopMaterial->SetTexture(
        Material::TextureType::AmbientOcclusion,
        ambientOcclusionTexture
    );

    /*
    pRegularSidesMaterial->SetTexture(
        Material::TextureType::Normal,
        normalTexture
    );

    pRegularSidesMaterial->SetTexture(
        Material::TextureType::Bump,
        heightTexture
    );
    */

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

    m_SceneRenderTarget.AttachTexture(
        AttachmentPoint::Color0,
        pSceneRenderTarget
    );

    m_SceneRenderTarget.AttachTexture(
        AttachmentPoint::DepthStencil,
        pDepthTexture
    );

    /// BRIGHT PASS RENDER TARGET

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

    ///
    /// HORIZONTAL BLUR RENDER TARGET
    ///

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

    ///
    /// VERTICAL BLUR RENDER TARGET
    ///

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

    /// COMPOSITE ROOT SIGNATURE

    CD3DX12_DESCRIPTOR_RANGE1 compositeTextureRange(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        2,  // one texture
        0   // t0
    );

    CD3DX12_ROOT_PARAMETER1 compositeRootParameters[1];

    compositeRootParameters[0].InitAsDescriptorTable(
        1,
        &compositeTextureRange,
        D3D12_SHADER_VISIBILITY_PIXEL
    );

    CD3DX12_STATIC_SAMPLER_DESC compositeSampler(
        0,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR
    );

    D3D12_ROOT_SIGNATURE_FLAGS compositeRootFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC compositeRootDesc;

    compositeRootDesc.Init_1_1(
        _countof(compositeRootParameters),
        compositeRootParameters,
        1,
        &compositeSampler,
        compositeRootFlags
    );

    pCompositeRootSignature =
        m_Device.CreateRootSignature(
            compositeRootDesc.Desc_1_1
        );

    log << "[Initialize] HDR scene target and composite root signature created\n";
    log.flush();

    log << "[Initialize] Creating lighting EffectPSO\n";
    log.flush();

    m_LightingPSO =
        std::make_shared<EffectPSO>(
            m_Device,
            true,   // enable lighting
            false,   // disable decal
            DXGI_FORMAT_R16G16B16A16_FLOAT
        );

    log << "[Initialize] Lighting EffectPSO created\n";
    log.flush();

    for (size_t i = 0;
        i < scene.EmissiveLightPositions.size();
        ++i)
    {
        const auto& position =
            scene.EmissiveLightPositions[i];

        auto pointLight =
            std::make_shared<PointLight>(
                position,
                position,
                "RED",
                0.0f,   // Ambient
                3.0f,   // Diffuse
                0.0f,   // Specular
                1.0f,   // Constant
                0.06f,  // Linear
                0.03f   // Quadratic
            );

        pointLight->SetColor(
            XMFLOAT4{
                1.00f,
                0.16f,
                0.02f,
                1.00f
            }
        );

        pointLight->TurnOn();

        RegisterLight(
            "Hex Point Light " +
            std::to_string(i),
            pointLight
        );
    }

    //
    // COMPOSITE SHADERS
    //

    const auto fullscreenVertexShaderPath =
        executableDirectory / L"Fullscreen_VS.cso";

    const auto compositePixelShaderPath =
        executableDirectory / L"Composite_PS.cso";

    Microsoft::WRL::ComPtr<ID3DBlob> fullscreenVertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> compositePixelShaderBlob;

    ThrowIfFailed(
        D3DReadFileToBlob(
            fullscreenVertexShaderPath.c_str(),
            &fullscreenVertexShaderBlob
        )
    );

    ThrowIfFailed(
        D3DReadFileToBlob(
            compositePixelShaderPath.c_str(),
            &compositePixelShaderBlob
        )
    );

    const auto brightPassPixelShaderPath =
        executableDirectory / L"BrightPass_PS.cso";

    Microsoft::WRL::ComPtr<ID3DBlob> brightPassPixelShaderBlob;

    ThrowIfFailed(
        D3DReadFileToBlob(
            brightPassPixelShaderPath.c_str(),
            &brightPassPixelShaderBlob
        )
    );

    const auto blurHorizontalPixelShaderPath =
        executableDirectory / L"BlurHorizontal_PS.cso";

    const auto blurVerticalPixelShaderPath =
        executableDirectory / L"BlurVertical_PS.cso";

    Microsoft::WRL::ComPtr<ID3DBlob> blurHorizontalPixelShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> blurVerticalPixelShaderBlob;

    ThrowIfFailed(
        D3DReadFileToBlob(
            blurHorizontalPixelShaderPath.c_str(),
            &blurHorizontalPixelShaderBlob
        )
    );

    ThrowIfFailed(
        D3DReadFileToBlob(
            blurVerticalPixelShaderPath.c_str(),
            &blurVerticalPixelShaderBlob
        )
    );

    //
    // COMPOSITE PIPELINE STATE
    //

    struct CompositePipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendState;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencilState;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK           SampleMask;
    } compositePipelineStateStream = {};

    compositePipelineStateStream.pRootSignature =
        pCompositeRootSignature
        ->GetD3D12RootSignature()
        .Get();

    compositePipelineStateStream.VS =
        CD3DX12_SHADER_BYTECODE(
            fullscreenVertexShaderBlob.Get()
        );

    compositePipelineStateStream.PS =
        CD3DX12_SHADER_BYTECODE(
            compositePixelShaderBlob.Get()
        );

    CD3DX12_RASTERIZER_DESC compositeRasterizer(
        D3D12_DEFAULT
    );

    compositeRasterizer.CullMode =
        D3D12_CULL_MODE_NONE;

    compositePipelineStateStream.RasterizerState =
        compositeRasterizer;

    compositePipelineStateStream.BlendState =
        CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    compositePipelineStateStream.SampleMask =
        UINT_MAX;

    compositePipelineStateStream.InputLayout =
    {
        nullptr,
        0
    };

    compositePipelineStateStream.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    CD3DX12_DEPTH_STENCIL_DESC compositeDepthState(
        D3D12_DEFAULT
    );

    compositeDepthState.DepthEnable = FALSE;
    compositeDepthState.StencilEnable = FALSE;

    compositePipelineStateStream.DepthStencilState =
        compositeDepthState;

    D3D12_RT_FORMAT_ARRAY compositeRTVFormats = {};
    compositeRTVFormats.NumRenderTargets = 1;

    compositeRTVFormats.RTFormats[0] =
        DXGI_FORMAT_R8G8B8A8_UNORM;

    compositePipelineStateStream.RTVFormats =
        compositeRTVFormats;

    DXGI_SAMPLE_DESC compositeSampleDesc = {};
    compositeSampleDesc.Count = 1;
    compositeSampleDesc.Quality = 0;

    compositePipelineStateStream.SampleDesc =
        compositeSampleDesc;

    pCompositePSO =
        m_Device.CreatePipelineStateObject(
            compositePipelineStateStream
        );

    /// BRIGHT PASS PIPELINE STATE

    auto brightPassPipelineStateStream =
        compositePipelineStateStream;

    brightPassPipelineStateStream.PS =
        CD3DX12_SHADER_BYTECODE(
            brightPassPixelShaderBlob.Get()
        );

    D3D12_RT_FORMAT_ARRAY brightPassRTVFormats = {};
    brightPassRTVFormats.NumRenderTargets = 1;
    brightPassRTVFormats.RTFormats[0] =
        DXGI_FORMAT_R16G16B16A16_FLOAT;

    brightPassPipelineStateStream.RTVFormats =
        brightPassRTVFormats;

    pBrightPassPSO =
        m_Device.CreatePipelineStateObject(
            brightPassPipelineStateStream
        );

    //
    // HORIZONTAL BLUR PIPELINE STATE
    //

    auto blurHorizontalPipelineStateStream =
        compositePipelineStateStream;

    blurHorizontalPipelineStateStream.PS =
        CD3DX12_SHADER_BYTECODE(
            blurHorizontalPixelShaderBlob.Get()
        );

    D3D12_RT_FORMAT_ARRAY blurHorizontalRTVFormats = {};
    blurHorizontalRTVFormats.NumRenderTargets = 1;
    blurHorizontalRTVFormats.RTFormats[0] =
        DXGI_FORMAT_R16G16B16A16_FLOAT;

    blurHorizontalPipelineStateStream.RTVFormats =
        blurHorizontalRTVFormats;

    pBlurHorizontalPSO =
        m_Device.CreatePipelineStateObject(
            blurHorizontalPipelineStateStream
        );

    //
    // VERTICAL BLUR PIPELINE STATE
    //

    auto blurVerticalPipelineStateStream =
        compositePipelineStateStream;

    blurVerticalPipelineStateStream.PS =
        CD3DX12_SHADER_BYTECODE(
            blurVerticalPixelShaderBlob.Get()
        );

    D3D12_RT_FORMAT_ARRAY blurVerticalRTVFormats = {};
    blurVerticalRTVFormats.NumRenderTargets = 1;
    blurVerticalRTVFormats.RTFormats[0] =
        DXGI_FORMAT_R16G16B16A16_FLOAT;

    blurVerticalPipelineStateStream.RTVFormats =
        blurVerticalRTVFormats;

    pBlurVerticalPSO =
        m_Device.CreatePipelineStateObject(
            blurVerticalPipelineStateStream
        );

    log << "[Initialize] Flushing command queue\n";
    log.flush();

    commandQueue.Flush();

    log << "[Initialize] Finished successfully\n";
    log.flush();

    return TRUE;
}