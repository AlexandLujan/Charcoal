#include "DX12LibPCH.h"

#include <PipelineStateObject.h>
#include <Device.h>
#include <fstream>
#include <iomanip>

using namespace dx12lib;

PipelineStateObject::PipelineStateObject(Device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc)
    : m_Device(device)
{
    std::ofstream log("sandbox_debug_log.txt", std::ios::app);

    log << "[PipelineStateObject] Constructor entered\n";
    log << "[PipelineStateObject] Stream size = " << desc.SizeInBytes << "\n";
    log << "[PipelineStateObject] Stream pointer = " << desc.pPipelineStateSubobjectStream << "\n";
    log.flush();

    auto d3d12Device = device.GetD3D12Device();

    log << "[PipelineStateObject] Got D3D12 device\n";
    log << "[PipelineStateObject] About to call CreatePipelineState\n";
    log.flush();

    HRESULT hr = d3d12Device->CreatePipelineState(
        &desc,
        IID_PPV_ARGS(&m_d3d12PipelineState)
    );

    log << "[PipelineStateObject] CreatePipelineState returned HRESULT = 0x"
        << std::hex << hr << "\n";
    log.flush();

    if (FAILED(hr))
    {
        log << "[PipelineStateObject] CreatePipelineState FAILED\n";
        log.flush();

        ThrowIfFailed(hr);
    }

    log << "[PipelineStateObject] Constructor finished successfully\n";
    log.flush();
}