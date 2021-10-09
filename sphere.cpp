#include"sphere.h"

void sphere::onWnd(HINSTANCE theInst, HINSTANCE thePrvAppInst, PWSTR thePCmdLine, int theNCmdShow, WNDPROC theProc) {
	WNDCLASS wndDesc;
	wchar_t clsName[] = L"sphere";

	wndDesc.lpfnWndProc = theProc;
	wndDesc.lpszClassName = clsName;
	wndDesc.hInstance = theInst;
	wndDesc.cbClsExtra = wndDesc.cbWndExtra = 0;
	wndDesc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndDesc.hCursor = LoadCursor(0, IDC_ARROW);
	wndDesc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndDesc.lpszMenuName = 0;
	wndDesc.style = CS_HREDRAW | CS_VREDRAW;

	RECT wndRect = scissorRect;
	AdjustWindowRect(&wndRect, WS_OVERLAPPEDWINDOW, false);
	try {
		if (!RegisterClass(&wndDesc))
			throw NormalStrExcpt;

		if (!(renderTargetWnd = CreateWindow(clsName, L"Port to dream", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, 0, 0, theInst, 0)))
			throw NormalStrExcpt;
	}
	catch (std::string errorMsg) {
		std::wstring wstr;
		for (const auto& oh : errorMsg)
			wstr.push_back(oh);
		MessageBox(0, wstr.c_str(), 0, 0);
		exit(0);
	}
	ShowWindow(renderTargetWnd, theNCmdShow);
	UpdateWindow(renderTargetWnd);
}
sphere::sphere(const std::vector<vertex>& newVer, const std::vector<ITYPE>& newInd, const LONG& width, const LONG& height) :
	DX12app(width, height), vertices(newVer), indices(newInd), dirty(true),
	eyeRadius(5.0f), eyeTheta(0), eyePhi(DirectX::XM_PIDIV4),
	CBPort(nullptr),lastX(0),lastY(0)
{
	assert((!vertices.empty()) && (!indices.empty()));
}
sphere::sphere(const char* geoFile, const LONG& width, const LONG& height) :
	DX12app(width, height), vertices(), indices(), dirty(true),
	eyeRadius(5.0f), eyeTheta(0), eyePhi(DirectX::XM_PIDIV4),
	CBPort(nullptr), lastX(0), lastY(0)
{
	std::ifstream geoIn;
	geoIn.open(geoFile, std::ifstream::in | std::ifstream::binary);
	uint64_t vCount=0, iCount=0;
	geoIn.read(reinterpret_cast<char*>(&iCount), sizeof(iCount));
	geoIn.read(reinterpret_cast<char*>(&vCount), sizeof(vCount));

	uint64_t he;
	for (he = 0;he < iCount;++he) {
		indices.emplace_back();
		geoIn.read(reinterpret_cast<char*>(&(indices.back())), sizeof(uint16_t));
	}
	for (he = 0;he < vCount;++he) {
		vertices.emplace_back();
		geoIn.read(reinterpret_cast<char*>(&(vertices.back())), sizeof(vertex));
	}
	geoIn.close();
}
void sphere::onInit(void) {
	DX12app::onInit();
	myCmdList->Reset(myCmdAlloc.Get(), nullptr);
	CreateDefaultBufferAndLoad(
		myDevice, myCmdList, indices.data(), indices.size(), sizeof(ITYPE), indexUploader, indexBuffer);
	CreateDefaultBufferAndLoad(
		myDevice, myCmdList, vertices.data(), vertices.size(), sizeof(vertex), vertexUploader, vertexBuffer);
	createConstBufferDescriptorHeapAndView<projMatrix>(myDevice, CBUploader, CBHeap, CBPort);
	compileShader(L"justShade.hlsl", "VS", compiledVS, "vs_5_0");
	compileShader(L"justShade.hlsl", "PS", compiledPS, "ps_5_0");

	CD3DX12_ROOT_PARAMETER rootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE range;
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	(rootParameter[0]).InitAsDescriptorTable(1, &range);

	Microsoft::WRL::ComPtr<ID3DBlob> rootSigErrorblob;
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, rootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSigBlob.GetAddressOf(), rootSigErrorblob.GetAddressOf());
	myDevice->CreateRootSignature(0, serializedRootSigBlob->GetBufferPointer(), serializedRootSigBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC whatIsThePipline;
	ZeroMemory(&whatIsThePipline, sizeof(whatIsThePipline));
	whatIsThePipline.InputLayout = { inputFormat, 2 };
	whatIsThePipline.pRootSignature = rootSignature.Get();
	whatIsThePipline.VS = {
		reinterpret_cast<BYTE*>(compiledVS->GetBufferPointer()),
		compiledVS->GetBufferSize()
	};
	whatIsThePipline.PS = {
	reinterpret_cast<BYTE*>(compiledPS->GetBufferPointer()),
	compiledPS->GetBufferSize()
	};
	whatIsThePipline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	whatIsThePipline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	whatIsThePipline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	whatIsThePipline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	whatIsThePipline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	whatIsThePipline.SampleMask = UINT_MAX;
	whatIsThePipline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	whatIsThePipline.NumRenderTargets = 1;
	whatIsThePipline.RTVFormats[0] = mySwapCBufferFormat;
	whatIsThePipline.SampleDesc.Count = my4xMSAAStat ? 4 : 1;
	whatIsThePipline.SampleDesc.Quality = my4xMSAAStat ? (my4xMSAAQulity - 1) : 0;
	whatIsThePipline.DSVFormat = myDepthStencilFormat;
	myDevice->CreateGraphicsPipelineState(&whatIsThePipline, IID_PPV_ARGS(pipStates.GetAddressOf()));
	myCmdList->Close();
	ID3D12CommandList* cmdLists[] = { myCmdList.Get() };
	myCmdQueue->ExecuteCommandLists(1, cmdLists);
	waitForExecution();
}
HRESULT sphere::CreateDefaultBufferAndLoad(
	Microsoft::WRL::ComPtr<ID3D12Device> yourDevice,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> yourCmdList,
	const void* initData,
	UINT64 count,
	UINT64 byteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
	Microsoft::WRL::ComPtr<ID3D12Resource>& defaultBuffer
) {
	D3D12_HEAP_PROPERTIES heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(count * byteSize);
	HRESULT condCode = S_OK;
	if (FAILED(condCode = yourDevice->CreateCommittedResource(
		&heapDesc,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())
	)))return condCode;
	heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	if (FAILED(condCode = yourDevice->CreateCommittedResource(
		&heapDesc,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	)))return condCode;

	D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	yourCmdList->ResourceBarrier(1, &transition);

	D3D12_SUBRESOURCE_DATA subResource = {};
	subResource.pData = initData;
	subResource.RowPitch = byteSize;
	subResource.SlicePitch = subResource.RowPitch;
	UpdateSubresources<1>(
		yourCmdList.Get(),
		defaultBuffer.Get(),
		uploadBuffer.Get(),
		0, 0, 1, &subResource
		);

	transition = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	yourCmdList->ResourceBarrier(1, &transition);

	return condCode;
}
void sphere::changeGeo(const vertex* vIn, const ITYPE* iIn, const size_t& vCount, const size_t& iCount) {
	std::vector<vertex> vTemp;
	std::vector<ITYPE> iTemp;

	for (size_t he = 0;he < vCount;++he)
		vTemp.push_back(vIn[he]);
	for (size_t she = 0;she < iCount;++she)
		iTemp.push_back(iIn[she]);

	vertices = std::move(vTemp);
	indices = std::move(iTemp);
}
template<typename ty>
HRESULT sphere::createConstBufferDescriptorHeapAndView(
	Microsoft::WRL::ComPtr<ID3D12Device>& yourDevice,
	Microsoft::WRL::ComPtr<ID3D12Resource>& yourCB,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& CBVHeap,
	BYTE*& mappedPort
) {
	HRESULT condCode = S_OK;
	D3D12_HEAP_PROPERTIES heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(ConstBufferSize(sizeof(ty)));
	if (FAILED(
		condCode = yourDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(yourCB.GetAddressOf())
		)
	))return condCode;
	if (FAILED(
		condCode = yourCB->Map(0, nullptr, reinterpret_cast<void**>(&mappedPort))
	))return condCode;

	D3D12_DESCRIPTOR_HEAP_DESC CBVHdesc = {};
	CBVHdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CBVHdesc.NodeMask = 0;
	CBVHdesc.NumDescriptors = 1;
	CBVHdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (FAILED(
		condCode = yourDevice->CreateDescriptorHeap(&CBVHdesc, IID_PPV_ARGS(CBVHeap.GetAddressOf()))
	))return condCode;

	D3D12_CONSTANT_BUFFER_VIEW_DESC CBV;
	CBV.BufferLocation = yourCB->GetGPUVirtualAddress();
	CBV.SizeInBytes = ConstBufferSize(sizeof(ty));
	yourDevice->CreateConstantBufferView(&CBV, CBVHeap->GetCPUDescriptorHandleForHeapStart());
	return condCode;
}
void sphere::compileShader(
	wchar_t const* const fileP, char const* const entryP,
	Microsoft::WRL::ComPtr<ID3DBlob>& shaderBytes,
	char const* const target
) {
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	D3DCompileFromFile(
		fileP, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryP, target, flags, 0, shaderBytes.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob.Get()) {
		MessageBox(0, (wchar_t*)errorBlob->GetBufferPointer(), 0, 0);
		exit(0);
	}
}
void sphere::onDraw(void) {
	if (FAILED(myCmdAlloc->Reset())) {
		MessageBox(0, L"Failed to reset allocator", 0, 0);
		exit(0);
	}
	if(FAILED(myCmdList->Reset(myCmdAlloc.Get(),pipStates.Get()))){
		MessageBox(0, L"Failed to reset the list", 0, 0);
		exit(0);
	}
	myCmdList->RSSetScissorRects(1, &scissorRect);
	myCmdList->RSSetViewports(1, &viewPort);

	D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(
		mySwapChainBuffers[myCurrentBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	D3D12_CPU_DESCRIPTOR_HANDLE rtHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		myRtvHeap->GetCPUDescriptorHandleForHeapStart(), myCurrentBackBuffer, rtvIncreSize);
	myCmdList->ResourceBarrier(1, &transition);
	myCmdList->ClearRenderTargetView(
		rtHandle,
		DirectX::Colors::BlueViolet,
		0,
		nullptr
	);
	D3D12_CPU_DESCRIPTOR_HANDLE dsHandle = myDsvHeap->GetCPUDescriptorHandleForHeapStart();
	myCmdList->ClearDepthStencilView(
		dsHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0,
		nullptr
	);
	myCmdList->OMSetRenderTargets(
		1,
		&rtHandle,
		true,
		&dsHandle
	);
	ID3D12DescriptorHeap* cbHeaps[] = { CBHeap.Get() };
	myCmdList->SetDescriptorHeaps(_countof(cbHeaps), cbHeaps);
	myCmdList->SetGraphicsRootSignature(rootSignature.Get());

	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbv.SizeInBytes = vertices.size() * sizeof(vertex);
	vbv.StrideInBytes = sizeof(vertex);
	myCmdList->IASetVertexBuffers(0, 1, &vbv);

	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibv.SizeInBytes = indices.size() * sizeof(ITYPE);
#ifdef ITYPE
	ibv.Format = DXGI_FORMAT_R16_UINT;
#else
	ibv.Format =
#endif
	myCmdList->IASetIndexBuffer(&ibv);

	myCmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_GPU_DESCRIPTOR_HANDLE cbvHandle(CBHeap->GetGPUDescriptorHandleForHeapStart());
	myCmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

	myCmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
	transition = CD3DX12_RESOURCE_BARRIER::Transition(
		mySwapChainBuffers[myCurrentBackBuffer].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	myCmdList->ResourceBarrier(1, &transition);
	myCmdList->Close();

	ID3D12CommandList* lists[] = { myCmdList.Get() };
	myCmdQueue->ExecuteCommandLists(1, lists);
	mySwapChain->Present(0, 0);
	myCurrentBackBuffer = (++myCurrentBackBuffer) % mySwapChainBufferCount;
	waitForExecution();
}
void sphere::onMouseDown(WPARAM wParam, int x, int y) {
	lastX = x;
	lastY = y;
	SetCapture(renderTargetWnd);
}
void sphere::onMouseUp(WPARAM wParam, int x, int y) {
	ReleaseCapture();
}
void sphere::onMouseMove(WPARAM wParam, int x, int y) {
	if (WM_LBUTTONDOWN & wParam) {
		eyePhi -= DirectX::XMConvertToRadians(static_cast<float>(y - lastY))/3.0;
		eyeTheta -= DirectX::XMConvertToRadians(static_cast<float>(x - lastX))/3.0;
		dirty = true;
	}
	lastX = x, lastY = y;
}
void sphere::onNextFrame(void) {
	DirectX::XMStoreFloat4x4(
		&(projection.m),
		DirectX::XMMatrixTranspose(
			DirectX::XMMatrixLookAtRH(
				DirectX::XMVectorSet(
					eyeRadius * sinf(eyePhi) * cosf(eyeTheta), 
					eyeRadius * sinf(eyePhi) * sinf(eyeTheta), 
					eyeRadius * cosf(eyePhi), 
					1.0f
				),
				DirectX::XMVectorZero(),
				DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)
			) *
			DirectX::XMMatrixPerspectiveFovRH(
				0.25f * DirectX::XM_PI,
				static_cast<float>(clientWidth) / clientHeight,
				1.0f, 1000.0f
			)
		)
	);
	std::memcpy(CBPort, &projection, sizeof(projection));
	//The new frame could be drawn thereafter.
}
void sphere::onMouseWheel(WPARAM wParam, int x, int y) {
	eyeRadius -= static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / 200;
	eyeRadius = clamp(eyeRadius, 2.0f, 9.0f);
	dirty = true;
}
int sphere::run(void) {
	MSG sphereMSG = { 0 };
	while (sphereMSG.message != WM_QUIT) {
		if (PeekMessage(&sphereMSG, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&sphereMSG);
			DispatchMessage(&sphereMSG);
		}
		else {
			if (!paused) {
				if (dirty) {
					onNextFrame();
					onDraw();
					dirty = false;
				}
			}
			else {
				Sleep(100);
			}
		}
	}
	return (int)sphereMSG.wParam;
}
LRESULT sphere::yourWndProc(HWND yourHwnd, UINT yourMsg, WPARAM wParam, LPARAM lParam) {
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
	case WM_COMMAND: {
		//Sent when the user selects a command item from a menu, 
		//when a control sends a notification message to its parent window, 
		//or when an accelerator keystroke is translated.
		return menuProc(yourHwnd, yourMsg, wParam, lParam);
	}
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
				else if (resizing) {

				}
				else {
					onResize();
				}
			}
			dirty = true;
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
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 300;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 300;
		break;
	case WM_MOUSEWHEEL:
		onMouseWheel(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		onMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		onMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSEMOVE:
		onMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(yourHwnd, yourMsg, wParam, lParam);
	}
	return 0;
}
void finerTriangle(std::vector<ITYPE>& theyList, std::vector<vertex>& theyVertex, std::vector<std::vector<ITYPE>>& theyAdj) {
	//Read the first three indices, which forms the first triangle.
	ITYPE i0 = theyList[0],
		i1 = theyList[1],
		i2 = theyList[2];

	/*
	getMidPoint should search the mid point between given line segment.
	If there is no such point, getMidPoint will calculate it secretly,
	  and pretend as if the point was already there.
	The newly calculated point will be added to the vertices. The new
	  adjacency will be recorded too.
	In a word, getMidPoint will find indices mid points without showing any bother.
	*/
	ITYPE m01 = getMidPoint(i0, i1, theyVertex, theyAdj),
		m02 = getMidPoint(i0, i2, theyVertex, theyAdj),
		m12 = getMidPoint(i1, i2, theyVertex, theyAdj);
	/*
	The new triangle list will be:
	m01,m12,m02
	i0,m01,m02
	m01,i1,m12
	m02,m12,i2
	I should use them to replace the original i0,i1,i2. So i0,i1,i2 should be removed, and the new list be appended.
	*/

	//erase(a,b) will remove elements in [a,b).
	theyList.erase(theyList.begin(), theyList.begin() + 3);
	theyList.insert(theyList.end(), {
		m01, m12, m02,
		i0, m01, m02,
		m01, i1, m12,
		m02, m12, i2
		});
}
ITYPE getMidPoint(ITYPE a, ITYPE b, std::vector<vertex>& theyVertex, std::vector<std::vector<ITYPE>>& theyAdj) {
	assert((a < theyVertex.size()) && (b < theyVertex.size()));
	assert(a != b);
	if (b < a)std::swap(a, b);
	//a is less than b anyway.

	ITYPE& midIndex = theyAdj[a][b - a - 1/*A trick to save space*/];
	if (midIndex == -1) {
		midIndex = makeMidPoint(a, b, theyVertex);
		for (auto& row : theyAdj)
			row.push_back(-1);
		theyAdj.emplace_back();
	}
	return theyAdj[a][b - a - 1/*A trick to save space*/];

}
ITYPE makeMidPoint(const ITYPE& a, const ITYPE& b, std::vector<vertex>& theyVertex) {
	DirectX::XMVECTOR va = XMLoadFloat3(&(theyVertex[a].pos)), vb = XMLoadFloat3(&(theyVertex[b].pos)), vc, vd;
	vc = DirectX::operator+(va, vb);

	va = XMLoadFloat4(&(theyVertex[a].color));
	vb = XMLoadFloat4(&(theyVertex[b].color));
	vd = DirectX::operator+(DirectX::operator*(0.5, va), DirectX::operator*(0.5, vb));

	theyVertex.emplace_back();
	XMStoreFloat3(&(theyVertex[theyVertex.size() - 1].pos), DirectX::XMVector3Normalize(vc));
	XMStoreFloat4(&(theyVertex[theyVertex.size() - 1].color), vd);

	return theyVertex.size() - 1;
}