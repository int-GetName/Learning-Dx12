#include"DX12app.h"
DX12app::DX12app(const LONG& width, const LONG& height) :
	DX12Basic(width, height),
	my4xMSAAStat(false),
	mined(false), maxed(false), paused(false), resizing(false),
	mySwapCBufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM),
	myDepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT),
	myCurrentBackBuffer(0), my4xMSAAQulity(0), myCurrentFence(0)
{}
DX12app::~DX12app() {}

int DX12app::run(void) {
	MSG currMsg = { 0 };
	while (currMsg.message != WM_QUIT) {
		if (PeekMessage(&currMsg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&currMsg);
			DispatchMessage(&currMsg);
		}
		else {
			//timer.tick()
			if (!paused) {
				//Do frame statistics
			}
			else {
				Sleep(100);
			}
		}
	}
	return (int)(currMsg.wParam);
}
void DX12app::onInit(void) {
	try {
#ifdef DEBUG_DX12
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> debugCtlr;
			if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugCtlr)))) {
				throw NormalStrExcpt;
			}
			debugCtlr->EnableDebugLayer();
		}
#endif
		if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(myFactory.GetAddressOf()))))
			throw NormalStrExcpt;
		if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(myDevice.GetAddressOf()))))
			throw NormalStrExcpt;
		if (FAILED(myDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(myFence.GetAddressOf()))))
			throw NormalStrExcpt;
		rtvIncreSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		dsvIncreSize = myDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS checkDesc;
		checkDesc.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		checkDesc.Format = mySwapCBufferFormat;
		checkDesc.NumQualityLevels = 0;
		checkDesc.SampleCount = 4;

		if (FAILED(myDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &checkDesc, sizeof(checkDesc))))
			throw NormalStrExcpt;

		my4xMSAAQulity = checkDesc.NumQualityLevels;
		assert(my4xMSAAQulity > 0);

		if (FAILED(createCmdObjs(myDevice, myCmdQueue, myCmdAlloc, myCmdList)))
			throw NormalStrExcpt;
		assert(myFactory);
		assert(myCmdQueue);
		assert(renderTargetWnd);
		if (FAILED(createSwapChain(
			myFactory,
			myCmdQueue,
			mySwapChain,
			mySwapChainBufferCount,
			renderTargetWnd, clientWidth, clientHeight,
			mySwapCBufferFormat, my4xMSAAStat, my4xMSAAQulity)
		))
			throw NormalStrExcpt;
		if (FAILED(createDsBuffer(
			myDevice,
			myDepthStencilBuffer,
			clientWidth, clientHeight,
			myDepthStencilFormat,
			my4xMSAAStat, my4xMSAAQulity)))
			throw NormalStrExcpt;
		if (FAILED(createDescriptorHeaps(myDevice, myRtvHeap, myDsvHeap, mySwapChainBufferCount, 1)))
			throw NormalStrExcpt;

		createDsv(myDevice, myDepthStencilBuffer, myDsvHeap, myCmdList);
		if (FAILED(createSwapCBufferAndView(myDevice, myRtvHeap, mySwapChain, mySwapChainBuffers, mySwapChainBufferCount, rtvIncreSize)))
			throw NormalStrExcpt;
	}
	catch (std::string errorMsg) {
		std::wstring wstr;
		for (const auto& oh : errorMsg)
			wstr.push_back(oh);
		MessageBox(0, wstr.c_str(), 0, 0);
		exit(0);
	}
}
LRESULT DX12app::menuProc(HWND mHwnd, UINT mMsg, WPARAM wParam, LPARAM lParam) {
	switch (LOWORD(wParam))
	{
	case MID_START_D3D12:
		MessageBox(0, L"MID_START_D3D12", 0, 0);
		break;
	case MID_EXIT:
		PostQuitMessage(0);
		break;
	case MID_TELL_DIM:
		MessageBox(0, (L"Width: "+std::to_wstring(clientWidth)+L"Height: "+std::to_wstring(clientHeight)).c_str(), 0, 0);
		break;
	case MID_D3D_STATE:
		MessageBox(0, L"MID_D3D_STATE", 0, 0);
		break;
	default:
		return DefWindowProc(mHwnd, mMsg, wParam, lParam);
	}
	return 0;
}
HRESULT DX12app::onResize(void) {
	assert(myDevice);
	assert(mySwapChain);
	assert(myCmdAlloc);
	HRESULT condCode = S_OK;
	waitForExecution();
	//Resize Swap chain buffers and recreate DS buffer.
	if (FAILED(condCode = myCmdList->Reset(myCmdAlloc.Get(), nullptr)))
		return condCode;
	for (int i = 0;i < mySwapChainBufferCount;++i)
		mySwapChainBuffers[i].Reset();
	myDepthStencilBuffer.Reset();

	if (FAILED(condCode = mySwapChain->ResizeBuffers(
		mySwapChainBufferCount,
		clientWidth, clientHeight,
		mySwapCBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)
	))
		return condCode;
	myCurrentBackBuffer = 0;
	if (FAILED(condCode = createSwapCBufferAndView(myDevice, myRtvHeap, mySwapChain, mySwapChainBuffers, mySwapChainBufferCount, rtvIncreSize)))
		return condCode;
	if (FAILED(condCode = createDsBuffer(myDevice, myDepthStencilBuffer, clientWidth, clientHeight, myDepthStencilFormat, my4xMSAAStat, my4xMSAAQulity)))
		return condCode;
	createDsv(myDevice,myDepthStencilBuffer,myDsvHeap,myCmdList);
	if (FAILED(condCode = myCmdList->Close()))
		return condCode;
	ID3D12CommandList* cmdList[] = { myCmdList.Get() };
	myCmdQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
	waitForExecution();
	setViewPort(static_cast<float>(clientHeight), static_cast<float>(clientWidth));
	setScissorRect(0, 0, clientHeight, clientWidth);
	return condCode;
}
void DX12app::waitForExecution(void) {
	++myCurrentFence;
	if (FAILED(myCmdQueue->Signal(myFence.Get(), myCurrentFence))) {
		MessageBox(0, L"Failed to signal", L"Wrong", 0);
		exit(0);
	}
	if (myFence->GetCompletedValue() < myCurrentFence) {
		HANDLE eventHandle = CreateEventExA(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		if (FAILED(myFence->SetEventOnCompletion(myCurrentFence,eventHandle))) {
			MessageBox(0, L"Failed to set event", L"Wrong", 0);
			exit(0);
		}
		if (eventHandle) {
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		else {
			MessageBox(0, L"Failed to wait for some event", L"Wrong", 0);
			exit(0);
		}
	}
}

LRESULT DX12app::yourWndProc(HWND yourHwnd, UINT yourMsg, WPARAM wParam, LPARAM lParam) {
	switch (yourMsg) {
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			paused = true;
			//timer.stop()
		}
		else {
			paused = false;
			//timer.start()
		}
		break;
	case WM_COMMAND:
		return menuProc(yourHwnd, yourMsg, wParam, lParam);

	case WM_SIZE:
		clientHeight = HIWORD(lParam);
		clientWidth = LOWORD(lParam);
		if (myDevice) {
			if (wParam == SIZE_MINIMIZED) {
				paused = true;
				mined = true;
				maxed = false;
			}
			else if (wParam == SIZE_MAXIMIZED) {
				paused = false;
				mined = false;
				maxed = true;
				onResize();
			}
			else if (wParam == SIZE_RESTORED) {
				if (mined) {
					paused = false;
					mined = false;
					onResize();
				}
				else if (maxed) {
					paused = false;
					maxed = false;
					onResize();
				}
				else if (resizing) {}
				else {
					onResize();
				}
			}
		}
		break;
	case WM_ENTERSIZEMOVE:
		paused = true;
		resizing = true;
		//timer.stop()
		break;
	case WM_EXITSIZEMOVE:
		paused = false;
		resizing = false;
		//timer.start()
		onResize();
		break;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 400;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 400;
		break;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(yourHwnd, yourMsg, wParam, lParam);
	}
	return 0;
}