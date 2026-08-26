#include "pch.h"
#include "EffectPSO.h"

#include <CommandList.h>
#include <Device.h>
#include <Helpers.h>
#include <Material.h>
#include <Light.h>
#include <PipelineStateObject.h>
#include <RootSignature.h>
#include <VertexTypes.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <wrl/client.h>
#include <filesystem>
#include <Windows.h>

using namespace Microsoft::WRL;
using namespace dx12lib;

namespace
{
    std::filesystem::path GetExecutableDirectory()
    {
        wchar_t path[MAX_PATH];

        GetModuleFileNameW(
            nullptr,
            path,
            MAX_PATH
        );

        return std::filesystem::path(path).parent_path();
    }
}

EffectPSO::EffectPSO(Device& device, bool enableLighting, bool enableDecal)
    : m_Device(device)
    , m_DirtyFlags(DF_All)
    , m_pPreviousCommandList(nullptr)
    , m_EnableLighting(enableLighting)
    , m_EnableDecal(enableDecal)
{
    m_pAlignedMVP = (MVP*)_aligned_malloc(sizeof(MVP), 16);

    // Setup the root signature
    // Load the vertex shader.
    const auto executableDirectory =
        GetExecutableDirectory();

    const auto vertexShaderPath =
        executableDirectory / L"Basic_VS.cso";

    ComPtr<ID3DBlob> vertexShaderBlob;

    ThrowIfFailed(
        D3DReadFileToBlob(
            vertexShaderPath.c_str(),
            &vertexShaderBlob
        )
    );

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob;

    if (enableLighting)
    {
        if (enableDecal)
        {
            const auto pixelShaderPath =
                executableDirectory / L"Decal_PS.cso";

            ThrowIfFailed(
                D3DReadFileToBlob(
                    pixelShaderPath.c_str(),
                    &pixelShaderBlob
                )
            );
        }
        else
        {
            const auto pixelShaderPath =
                executableDirectory / L"Lighting_PS.cso";

            ThrowIfFailed(
                D3DReadFileToBlob(
                    pixelShaderPath.c_str(),
                    &pixelShaderBlob
                )
            );
        }
    }
    else
    {
        const auto pixelShaderPath =
            executableDirectory / L"Unlit_PS.cso";

        ThrowIfFailed(
            D3DReadFileToBlob(
                pixelShaderPath.c_str(),
                &pixelShaderBlob
            )
        );
    }

    // Create a root signature.
    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // Descriptor range for the textures.
    CD3DX12_DESCRIPTOR_RANGE1 descriptorRage(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 3);

    // clang-format off
    CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
    rootParameters[RootParameters::MatricesCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[RootParameters::MaterialCB].InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::LightPropertiesCB].InitAsConstants(sizeof(LightProperties) / 4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::PointLights].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::SpotLights].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::DirectionalLights].InitAsShaderResourceView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::Textures].InitAsDescriptorTable(1, &descriptorRage, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler(0, D3D12_FILTER_ANISOTROPIC);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(RootParameters::NumRootParameters, rootParameters, 1, &anisotropicSampler, rootSignatureFlags);
    // clang-format on

    m_RootSignature = m_Device.CreateRootSignature(rootSignatureDescription.Desc_1_1);

    // Setup the pipeline state.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
    } pipelineStateStream;

    // Match the swap-chain render target format.
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    // Match the swap chain sample description.
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = backBufferFormat;

    CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);
    rasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
    { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.RasterizerState = rasterizerState;
    //pipelineStateStream.InputLayout = VertexPositionNormalTangentBitangentTexture::InputLayout;
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.DSVFormat = depthBufferFormat;
    pipelineStateStream.RTVFormats = rtvFormats;
    pipelineStateStream.SampleDesc = sampleDesc;

    m_PipelineStateObject = m_Device.CreatePipelineStateObject(pipelineStateStream);

    // Create an SRV that can be used to pad unused texture slots.
    D3D12_SHADER_RESOURCE_VIEW_DESC defaultSRV;
    defaultSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    defaultSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    defaultSRV.Texture2D.MostDetailedMip = 0;
    defaultSRV.Texture2D.MipLevels = 1;
    defaultSRV.Texture2D.PlaneSlice = 0;
    defaultSRV.Texture2D.ResourceMinLODClamp = 0;
    defaultSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_DefaultSRV = m_Device.CreateShaderResourceView(nullptr, &defaultSRV);
}

EffectPSO::~EffectPSO()
{
    _aligned_free(m_pAlignedMVP);
}

inline void EffectPSO::BindTexture(CommandList& commandList, uint32_t offset, const std::shared_ptr<Texture>& texture)
{
    if (texture)
    {
        commandList.SetShaderResourceView(RootParameters::Textures, offset, texture,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
    else
    {
        commandList.SetShaderResourceView(RootParameters::Textures, offset, m_DefaultSRV,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}

void EffectPSO::Apply(CommandList& commandList)
{
    if (m_pPreviousCommandList != &commandList)
    {
        m_DirtyFlags |= DF_All;
        m_pPreviousCommandList = &commandList;
    }

    commandList.SetPipelineState(m_PipelineStateObject);
    commandList.SetGraphicsRootSignature(m_RootSignature);

    if (m_DirtyFlags & DF_Matrices)
    {
        Matrices m;
        m.ModelMatrix = m_pAlignedMVP->World;
        m.ModelViewMatrix = m_pAlignedMVP->World * m_pAlignedMVP->View;
        m.ModelViewProjectionMatrix = m.ModelViewMatrix * m_pAlignedMVP->Projection;
        m.InverseTransposeModelViewMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, m.ModelViewMatrix));

        commandList.SetGraphicsDynamicConstantBuffer(RootParameters::MatricesCB, m);
    }

    if (m_DirtyFlags & DF_Material)
    {
        if (m_Material)
        {
            const auto& materialProps = m_Material->GetMaterialProperties();

            commandList.SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, materialProps);

            using TextureType = Material::TextureType;

            BindTexture(commandList, 0, m_Material->GetTexture(TextureType::Ambient));
            BindTexture(commandList, 1, m_Material->GetTexture(TextureType::Emissive));
            BindTexture(commandList, 2, m_Material->GetTexture(TextureType::Diffuse));
            BindTexture(commandList, 3, m_Material->GetTexture(TextureType::Specular));
            BindTexture(commandList, 4, m_Material->GetTexture(TextureType::SpecularPower));
            BindTexture(commandList, 5, m_Material->GetTexture(TextureType::Normal));
            BindTexture(commandList, 6, m_Material->GetTexture(TextureType::Bump));
            BindTexture(commandList, 7, m_Material->GetTexture(TextureType::Opacity));
        }
    }

    if (m_DirtyFlags & DF_PointLights)
    {
        std::vector<GPUPointLight> gpuPointLights;

        gpuPointLights.reserve(m_PointLights.size());

        for (const auto& light : m_PointLights)
        {
            GPUPointLight gpuLight{};

            gpuLight.PositionWS = light->PositionWS;
            gpuLight.PositionVS = light->PositionVS;
            gpuLight.Color = light->color;

            gpuLight.Ambient = light->ambient;
            gpuLight.DiffuseIntensity = light->diffuseIntensity;
            gpuLight.SpecularIntensity = light->specularIntensity;

            gpuLight.ConstantAttenuation = light->ConstantAttenuation;
            gpuLight.LinearAttenuation = light->LinearAttenuation;
            gpuLight.QuadraticAttenuation = light->QuadraticAttenuation;

            gpuPointLights.push_back(gpuLight);
        }

        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::PointLights, gpuPointLights);
    }

    if (m_DirtyFlags & DF_SpotLights)
    {
        std::vector<GPUSpotLight> gpuSpotLights;

        gpuSpotLights.reserve(m_SpotLights.size());

        for (const auto& light : m_SpotLights)
        {
            GPUSpotLight gpuLight{};

            gpuLight.PositionWS = light->PositionWS;
            gpuLight.PositionVS = light->PositionVS;

            gpuLight.DirectionWS = light->DirectionWS;
            gpuLight.DirectionVS = light->DirectionVS;

            gpuLight.Color = light->color;

            gpuLight.Ambient = light->ambient;
            gpuLight.DiffuseIntensity = light->diffuseIntensity;
            gpuLight.SpecularIntensity = light->specularIntensity;
            gpuLight.SpotAngle = light->SpotAngle;

            gpuLight.ConstantAttenuation = light->ConstantAttenuation;
            gpuLight.LinearAttenuation = light->LinearAttenuation;
            gpuLight.QuadraticAttenuation = light->QuadraticAttenuation;

            gpuSpotLights.push_back(gpuLight);
        }

        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::SpotLights, gpuSpotLights);
    }

    if (m_DirtyFlags & DF_DirectionalLights)
    {
        std::vector<GPUDirectionalLight> gpuDirectionalLights;

        gpuDirectionalLights.reserve(m_DirectionalLights.size());

        for (const auto& light : m_DirectionalLights)
        {
            GPUDirectionalLight gpuLight{};

            gpuLight.DirectionWS = light->DirectionWS;

            XMVECTOR directionWS =
                XMLoadFloat4(&light->DirectionWS);

            XMVECTOR directionVS =
                XMVector3TransformNormal(
                    directionWS,
                    m_pAlignedMVP->View
                );

            directionVS =
                XMVector3Normalize(directionVS);

            XMStoreFloat4(
                &gpuLight.DirectionVS,
                directionVS
            );

            gpuLight.Color = light->color;

            gpuLight.Ambient = light->ambient;
            gpuLight.DiffuseIntensity = light->diffuseIntensity;
            gpuLight.SpecularIntensity = light->specularIntensity;

            gpuDirectionalLights.push_back(gpuLight);
        }

        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::DirectionalLights, gpuDirectionalLights);
    }

    if (m_DirtyFlags & (DF_PointLights | DF_SpotLights | DF_DirectionalLights))
    {
        LightProperties lightProps{};
        lightProps.NumPointLights = static_cast<uint32_t>(m_PointLights.size());
        lightProps.NumSpotLights = static_cast<uint32_t>(m_SpotLights.size());
        lightProps.NumDirectionalLights = static_cast<uint32_t>(m_DirectionalLights.size());
        lightProps.Padding = 0;
        lightProps.AmbientLightColor = m_AmbientLightColor;

        commandList.SetGraphics32BitConstants(
            RootParameters::LightPropertiesCB,
            lightProps
        );
    }

    // Clear the dirty flags to avoid setting any states the next time the effect is applied.
    m_DirtyFlags = DF_None;
}
