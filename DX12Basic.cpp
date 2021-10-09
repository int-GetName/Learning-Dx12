#include"DX12Basic.h"
DX12Basic::DX12Basic(const LONG& w, const LONG& h) :clientWidth(w), clientHeight(h),
scissorRect({ 0,0,w,h }), viewPort({ 0.0f,0.0f,static_cast<float>(w),static_cast<float>(h),0.0f,1.0f }) {}
DX12Basic::~DX12Basic() {}
HRESULT DX12Basic::createCmdObjs(
	Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>& yourQue,
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& yourAlloc,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& yourList
) {
	HRESULT hResult;
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	if (FAILED(hResult = yourDev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(yourQue.GetAddressOf()))))
		return hResult;
	if (FAILED(hResult = yourDev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(yourAlloc.GetAddressOf()))))
		return hResult;
	if (FAILED(hResult = yourDev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, yourAlloc.Get(), nullptr, IID_PPV_ARGS(yourList.GetAddressOf()))))
		return hResult;
	yourList->Close();
	return S_OK;
}
HRESULT DX12Basic::createSwapChain(
	Microsoft::WRL::ComPtr<IDXGIFactory4>& yourFac,
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>& yourCmdQue,
	Microsoft::WRL::ComPtr<IDXGISwapChain>& yourSwapC,
	const UINT& bufferNum,
	const HWND& outputWnd,
	const UINT& bufferWid,
	const UINT& bufferHigt,
	const DXGI_FORMAT& bufferFormat,
	const bool& MSAAFlag,
	const UINT& MSAAQuality
) {
	HRESULT hResult;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferCount = bufferNum;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = outputWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	swapChainDesc.BufferDesc.Width = bufferWid;
	swapChainDesc.BufferDesc.Height = bufferHigt;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = bufferFormat;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = MSAAFlag ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = MSAAFlag ? (MSAAQuality - 1) : 0;

	if (FAILED(hResult = yourFac->CreateSwapChain(yourCmdQue.Get(), &swapChainDesc, yourSwapC.GetAddressOf())))
		return hResult;
	return S_OK;
}
HRESULT DX12Basic::createDescriptorHeaps(
	Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& yourRtvHeap,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& yourDsvHeap,
	const UINT& yourRtvhSize,
	const UINT& yourDsvhSize
) {
	HRESULT hResult;
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	HeapDesc.NumDescriptors = yourRtvhSize;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HeapDesc.NodeMask = 0;
	if (FAILED(hResult = yourDev->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(yourRtvHeap.GetAddressOf()))))
		return hResult;

	HeapDesc.NumDescriptors = yourDsvhSize;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HeapDesc.NodeMask = 0;
	if (FAILED(hResult = yourDev->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(yourDsvHeap.GetAddressOf()))))
		return hResult;
	return S_OK;
}
HRESULT DX12Basic::createDsBuffer(
	Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
	Microsoft::WRL::ComPtr<ID3D12Resource>& yourDSBuffer,
	const UINT64& bufferWid,
	const UINT& bufferHigt,
	const DXGI_FORMAT& bufferFormat,
	const bool& MSAAFlag,
	const UINT& MSAAQuality
) {
	HRESULT hResult;

	D3D12_RESOURCE_DESC DsDesc;
	DsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DsDesc.Alignment = 0;
	DsDesc.Width = bufferWid;
	DsDesc.Height = bufferHigt;
	DsDesc.DepthOrArraySize = 1;
	DsDesc.MipLevels = 1;
	DsDesc.Format = bufferFormat;
	DsDesc.SampleDesc.Count = MSAAFlag ? 4 : 1;
	DsDesc.SampleDesc.Quality = MSAAFlag ? (MSAAQuality - 1) : 0;
	DsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = bufferFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	auto alrightThen = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if (FAILED(hResult = yourDev->CreateCommittedResource(
		&alrightThen,
		D3D12_HEAP_FLAG_NONE,
		&DsDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(yourDSBuffer.GetAddressOf())
	)))
		return hResult;
	return S_OK;
}
HRESULT DX12Basic::createSwapCBufferAndView(
	Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& yourRtvHeap,
	Microsoft::WRL::ComPtr<IDXGISwapChain>& yourSwapC,
	Microsoft::WRL::ComPtr<ID3D12Resource>* yourBuffers,
	const UINT& bufferCount,
	const UINT& viewSize
) {
	HRESULT hResult;
	CD3DX12_CPU_DESCRIPTOR_HANDLE the_handle(yourRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0;i < bufferCount;++i) {
		if (FAILED(hResult = yourSwapC->GetBuffer(i, IID_PPV_ARGS((yourBuffers[i]).GetAddressOf())))) {
			return hResult;
		}
		yourDev->CreateRenderTargetView(yourBuffers[i].Get(), nullptr, the_handle);
		the_handle.Offset(1, viewSize);
	}
	return S_OK;
}
void DX12Basic::createDsv(
	Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
	Microsoft::WRL::ComPtr<ID3D12Resource>& yourBuffer,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& yourHeap,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& yourList
) {
	yourDev->CreateDepthStencilView(
		yourBuffer.Get(),
		nullptr,
		yourHeap->GetCPUDescriptorHandleForHeapStart()
	);
	auto alright = CD3DX12_RESOURCE_BARRIER::Transition(
		yourBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	yourList->ResourceBarrier(1, &alright);
}
void DX12Basic::setViewPort(const FLOAT& height, const FLOAT& width, const FLOAT& X, const FLOAT& Y, const FLOAT& minD, const FLOAT& maxD) {
	viewPort.TopLeftX = X;
	viewPort.TopLeftY = Y;
	viewPort.Height = static_cast<float>(height);
	viewPort.Width = static_cast<float>(width);
	viewPort.MaxDepth = maxD;
	viewPort.MinDepth = minD;
}
void DX12Basic::setScissorRect(LONG yourTop, LONG yourLeft, LONG yourBottom, LONG yourRight) {
	scissorRect.left = yourLeft;
	scissorRect.top = yourTop;
	scissorRect.right = yourRight;
	scissorRect.bottom = yourBottom;
}