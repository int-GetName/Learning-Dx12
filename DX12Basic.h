#ifndef __APPFRAME_H__
#define __APPFRAME_H__
#include"headers.h"
#define NormalStrExcpt std::string("Failed in file: ") + __FILE__ + ("\nAt line: " + std::to_string(__LINE__))
#define defWid 800
#define defHgt 600
struct DX12Basic {
	DX12Basic(const LONG& w = 800, const LONG& h = 600);
	DX12Basic(const DX12Basic&) = delete;
	DX12Basic(DX12Basic&&) = delete;
	DX12Basic& operator=(const DX12Basic&) = delete;
	DX12Basic& operator=(DX12Basic&&) = delete;
	virtual ~DX12Basic();
protected:
	HRESULT createCmdObjs(
		Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>& yourQue,
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& yourAlloc,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& yourList
	);
	HRESULT createSwapChain(
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
	);
	HRESULT createDsBuffer(
		Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
		Microsoft::WRL::ComPtr<ID3D12Resource>& yourDSBuffer,
		const UINT64& bufferWid,
		const UINT& bufferHigt,
		const DXGI_FORMAT& bufferFormat,
		const bool& MSAAFlag,
		const UINT& MSAAQuality
	);
	HRESULT createDescriptorHeaps(
		Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& yourRtvHeap,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& yourDsvHeap,
		const UINT& yourRtvhSize,
		const UINT& yourDsvhSize
	);
	void createDsv(
		Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
		Microsoft::WRL::ComPtr<ID3D12Resource>& yourBuffer,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& yourHeap,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& yourList
	);
	HRESULT createSwapCBufferAndView(
		Microsoft::WRL::ComPtr<ID3D12Device>& yourDev,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& yourRtvHeap,
		Microsoft::WRL::ComPtr<IDXGISwapChain>& yourSwapC,
		Microsoft::WRL::ComPtr<ID3D12Resource>* yourBuffers,
		const UINT& bufferCount,
		const UINT& viewSize
	);
	void setViewPort(
		const FLOAT& height = 600, 
		const FLOAT& width = 800, 
		const FLOAT& X = 0.0f, 
		const FLOAT& Y = 0.0f, 
		const FLOAT& minD = 0.0f, 
		const FLOAT& maxD = 1.0f
	);
	void setScissorRect(LONG yourTop, LONG yourLeft, LONG yourBottom, LONG yourRight);
	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;
	LONG clientWidth, clientHeight;
private:
};
#endif // !__APPFRAME_H__