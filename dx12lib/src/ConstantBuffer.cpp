#include "DX12LibPCH.h"

#include <ConstantBuffer.h>
#include <Device.h>
#include <d3dx12.h>

using namespace dx12lib;

ConstantBuffer::ConstantBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource )
: Buffer( device, resource )
{
    m_SizeInBytes = GetD3D12ResourceDesc().Width;
}

ConstantBuffer::~ConstantBuffer() {}
