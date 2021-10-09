#ifndef __SPHERE_H__
#define __SPHERE_H__
#include"DX12app.h"

#ifdef EIS
#define 32ITYPE UINT32
#else
#define ITYPE INT16
#endif

struct vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};
struct projMatrix {
	DirectX::XMFLOAT4X4 m;
};
const static D3D12_INPUT_ELEMENT_DESC inputFormat[2] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,sizeof(vertex::pos),D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
};
struct sphere :DX12app {
	sphere(const std::vector<vertex>& newVer,const std::vector<ITYPE>& newInd,const LONG& width = defWid, const LONG& height = defHgt);
	sphere(const char* geoFile, const LONG& width = defWid, const LONG& height = defHgt);
	~sphere() {};
	virtual void onWnd(HINSTANCE, HINSTANCE, PWSTR, int, WNDPROC);
	virtual void onInit(void)override;
	virtual int run(void)override;
	virtual LRESULT yourWndProc(HWND yourHwnd, UINT yourMsg, WPARAM wParam, LPARAM lParam) override;
	void changeGeo(const vertex* vIn, const ITYPE* iIn, const size_t& vCount, const size_t& iCount);
	std::vector<vertex> vertices;
	std::vector<ITYPE> indices;
protected:
	projMatrix projection;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipStates;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSigBlob;

	Microsoft::WRL::ComPtr<ID3DBlob> compiledVS, compiledPS;

	Microsoft::WRL::ComPtr<ID3D12Resource>
		vertexBuffer, vertexUploader,
		indexBuffer, indexUploader;

	FLOAT eyeRadius, eyeTheta, eyePhi;

	BYTE* CBPort;
	Microsoft::WRL::ComPtr<ID3D12Resource> CBUploader;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CBHeap;

	INT lastX, lastY;
	bool dirty;

	HRESULT CreateDefaultBufferAndLoad(
		Microsoft::WRL::ComPtr<ID3D12Device> yourDevice,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
		const void* initData,
		UINT64 count,
		UINT64 byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
		Microsoft::WRL::ComPtr<ID3D12Resource>& defaultBuffer
	);
	template<typename ty>
	HRESULT createConstBufferDescriptorHeapAndView(
		Microsoft::WRL::ComPtr<ID3D12Device>& yourDevice,
		Microsoft::WRL::ComPtr<ID3D12Resource>& constantB,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& CBVHeap,
		BYTE*& mappedData
	);
	void compileShader(
		wchar_t const* const fileP, char const* const entryP,
		Microsoft::WRL::ComPtr<ID3DBlob>& shaderBytes,
		char const* const target
	);
	virtual void onMouseDown(WPARAM wParam, int x, int y);
	virtual void onMouseUp(WPARAM wParam, int x, int y);
	virtual void onMouseMove(WPARAM wParam, int x, int y);
	virtual void onDraw(void);
	virtual void onNextFrame(void);
	virtual void onMouseWheel(WPARAM wParam, int x, int y);
};

void finerTriangle(std::vector<INT16>& theyList, std::vector<vertex>& theyVertex, std::vector<std::vector<INT16>>& theyAdj);
INT16 getMidPoint(INT16 a, INT16 b, std::vector<vertex>& theyVertex, std::vector<std::vector<INT16>>& theyAdj);
INT16 makeMidPoint(const INT16& a, const INT16& b, std::vector<vertex>& theyVertex);

void fromGravCenter(std::queue<UINT32>& triangles, std::vector<vertex>& endpoints);
UINT32 getGravCenter(const UINT32& a, const UINT32& b, const UINT32& c, std::vector<vertex>& endpoints);
using DirectX::operator+;
using DirectX::operator/;
#endif // !__SPHERE_H__
