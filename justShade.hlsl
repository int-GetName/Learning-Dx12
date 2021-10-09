cbuffer constBufferObj:register(b0) {
	float4x4 projMatrix;
};
/*
const D3D12_INPUT_ELEMENT_DESC cube::inputLayout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,sizeof(cube::vertex::pos),D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
};
*/
struct VIn {
	float3 PosV:POSITION;
	float4 colorV:COLOR;
};
struct VOut {
	float4 posH:SV_POSITION;
	float4 colorV: COLOR;
};
float biFunc(float x, float y);
VOut VS(VIn vin) {
	VOut passToPS;
	vin.PosV.z = biFunc(vin.PosV.x*2, vin.PosV.y*2);
	passToPS.posH = mul(float4(vin.PosV, 1.0f), projMatrix);
	//passToPS.posH = mul(projMatrix, float4(vin.PosV, 1.0f));
	passToPS.colorV = vin.colorV;
	return passToPS;
}
float4 PS(VOut pin) :SV_Target{
	return pin.colorV;
}
float biFunc(float x, float y) { return log(x * y + 0.0000008451329846523); }