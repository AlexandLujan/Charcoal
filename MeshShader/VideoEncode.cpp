#include "stdafx.h"
#include <cstdint>
#include <dxgiformat.h>
#include <iostream>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <string>
#include <windows.h>
#include <wrl/client.h>
#include "Const.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "DXImage.h"
#include "DXImageHelper.h"
#include "MeshRender.h"
#include "Utility.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace std;
using namespace cv;


int DXImage::StartVideoOutput(int w, int h, double fps, string out_file)
{
	bool result = video.open(out_file,
		//		cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
				cv::VideoWriter::fourcc('x', '2', '6', '4'),
				fps,
				cv::Size(w, h), true);
	if (result == false)
	{
		std::cerr << "Could not open the video writer!" << std::endl;
		return -1;
	}
	if (!video.isOpened())
	{
		std::cerr << "Could not open the video writer!" << std::endl;
		return -1;
	}

	return 0;
}

// Per-frame (after DX12 readback to uint8_t* bgra_data, linesize=w*4)

void DXImage::EncodeFrame(uint8_t* bgra_data, int linesize)
{
	cv::Mat frame(SCREEN_HEIGHT, SCREEN_WIDTH, CV_8UC4, bgra_data, linesize);
	video << frame;
}

// End:
void DXImage::FinishVideo()
{
	video.release();
}

void MeshRender::VideoEncoder(ID3D12Resource* outputBuffer)
{
	HRESULT hr;
	// puts buffer at 256 boundary	
	uint32_t rowStride = ((SCREEN_WIDTH + 63) & 0xFFFFFFC0) * sizeof(uint32_t);
	// size of the captured frame in bytes 
	const UINT64 screenBufferSize = rowStride * SCREEN_HEIGHT;

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[1];
	ComPtr<ID3D12PipelineState> m_pipelineState;
//	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
//	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[0]));
	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	D3D12_RESOURCE_BARRIER b = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
								{},
								{outputBuffer, //->Get(),
								0,
								D3D12_RESOURCE_STATE_PRESENT, //D3D12_RESOURCE_STATE_PRESENT,
								D3D12_RESOURCE_STATE_COPY_SOURCE} };

	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = outputBuffer; // .Get(); // Current Swapchain buffer
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = 0;

	D3D12_HEAP_PROPERTIES readbackHeapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK) };
	D3D12_RESOURCE_DESC readbackBufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(screenBufferSize) };
	ComPtr<ID3D12Resource> readbackBuffer;
	hr = m_device->CreateCommittedResource(
		&readbackHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&readbackBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&readbackBuffer));
	if (FAILED(hr)) { /* log and return */ } // D3D12_RESOURCE_DIMENSION_TEXTURE2

	readbackBuffer->SetName(L"Read back buffer");
	outputBuffer->SetName(L"Back buffer");

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = readbackBuffer.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dst.PlacedFootprint.Footprint.Width = SCREEN_WIDTH;
	dst.PlacedFootprint.Footprint.Height = SCREEN_HEIGHT;
	dst.PlacedFootprint.Footprint.Depth = 1;
	dst.PlacedFootprint.Footprint.RowPitch = rowStride;

	m_commandList->ResourceBarrier(1, &b);

	m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	b = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
			{},
			{outputBuffer, //.Get(),
			0,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_PRESENT} }; // D3D12_RESOURCE_STATE_PRESENT
													//D3D12_RESOURCE_STATE_COMMON
	m_commandList->ResourceBarrier(1, &b);

	// execute and signal fence, wait CPU until fence reached
	ThrowIfFailed(m_commandList->Close());
	m_commandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(m_commandList.GetAddressOf()));

	UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));



	// Map and copy out, taking rowPitch into account
	uint8_t* mapped = nullptr;
	D3D12_RANGE readRange{ 0, (SIZE_T)screenBufferSize };
	readbackBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mapped));
	for (UINT i = 0; i < screenBufferSize; i += sizeof(UINT))
	{
		uint32_t* p = reinterpret_cast<uint32_t*>(mapped + i);
		uint32_t tem = *p;
		*p = colorswap(tem);
	}
	WaitForGpu();
	EncodeFrame(mapped, rowStride);

	WaitForGpu();
	D3D12_RANGE emptyRange{ 0, 0 };
	readbackBuffer->Unmap
	(
		0,
		&emptyRange
	);
// safe to release resources now
	readbackBuffer.Reset();

}

