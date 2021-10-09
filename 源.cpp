#include"sphere.h"
#include<fstream>
#define DEBUG_DX12

static sphere chp4("Grid5.geo");
LRESULT CALLBACK youDriveMeInsane(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { return chp4.yourWndProc(hWnd, message, wParam, lParam); }
int WINAPI wWinMain(HINSTANCE appInst, HINSTANCE prvAppInst, PWSTR pCmdLine, int nCmdShow) {
	
	chp4.onWnd(appInst, prvAppInst, pCmdLine, nCmdShow, static_cast<WNDPROC>(youDriveMeInsane));
	chp4.onInit();
	return chp4.run();
}
/*
	std::ifstream iIn, vIn;
	iIn.open("indexOut", std::ifstream::in | std::ifstream::binary);
	vIn.open("vertexOut", std::ifstream::in | std::ifstream::binary);

	unsigned int vCount, iCount;
	iIn.read(reinterpret_cast<char*>(&iCount), sizeof(iCount));
	vIn.read(reinterpret_cast<char*>(&vCount), sizeof(vCount));

	vertex* thePoints = new vertex[vCount];
	ITYPE* myEdges = new ITYPE[iCount];
	iIn.read(reinterpret_cast<char*>(myEdges), sizeof(ITYPE) * iCount);
	vIn.read(reinterpret_cast<char*>(thePoints), sizeof(vertex) * vCount);

	iIn.close(), vIn.close();
	chp4.changeGeo(thePoints, myEdges, vCount, iCount);
	*/