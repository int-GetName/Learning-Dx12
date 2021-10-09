#ifndef __DX12APP_H__
#define __DX12APP_H__
#include"DX12Basic.h"
#define clamp(x,lower,higher) ((x)<(lower)?(lower):((x)>(higher)?(higher):(x)))
#define ConstBufferSize(x) ((x)+255)&~255

struct DX12app :DX12Basic {
	DX12app(const LONG& width, const LONG& height);
	DX12app(const DX12app&) = delete;
	DX12app(DX12app&&) = delete;
	DX12app& operator=(const DX12app&) = delete;
	DX12app& operator=(DX12app&&) = delete;
	virtual ~DX12app();
	virtual void onInit(void);
	int run(void);
	virtual LRESULT yourWndProc(HWND yourHwnd, UINT yourMsg, WPARAM wParam, LPARAM lParam);
	HWND renderTargetWnd;
protected:
	static const UINT mySwapChainBufferCount = 2;

	Microsoft::WRL::ComPtr<IDXGIFactory4> myFactory;
	Microsoft::WRL::ComPtr<ID3D12Device> myDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> myCmdAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> myCmdList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> myCmdQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mySwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> myDepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myRtvHeap, myDsvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> mySwapChainBuffers[mySwapChainBufferCount];

	bool my4xMSAAStat, mined, maxed, paused, resizing;
	DXGI_FORMAT mySwapCBufferFormat, myDepthStencilFormat;

	UINT myCurrentBackBuffer;
	UINT my4xMSAAQulity;
	UINT rtvIncreSize, dsvIncreSize;
	UINT64 myCurrentFence;	

	void waitForExecution(void);
	virtual LRESULT menuProc(HWND mHwnd, UINT mMsg, WPARAM wParam, LPARAM lParam);
	virtual HRESULT onResize(void);
};
#endif // !__DX12APP_H__
