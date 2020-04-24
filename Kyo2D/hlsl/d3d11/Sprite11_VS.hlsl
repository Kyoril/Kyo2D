
cbuffer ConstantBuffer
{
	float4x4 matView;
}

cbuffer PerObject
{
	float4x4 matWorld;
	float targetWidth;
	float targetHeight;
	float rawWidth;
	float rawHeight;
};

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD0;
	float4 colorkey : TEXCOORD1;
};

VOut main(
	float4 pos : POSITION, 
	float4 color : COLOR, 
	float2 texcoord : TEXCOORD0, 
	float4 colorkey : TEXCOORD1)
{
	VOut output;
	
	float4x4 mvp = mul(matView, matWorld);
	output.position = mul(mvp, float4(pos.x, pos.y, pos.z, 1.0f));

	output.color = color;
	output.texcoord = texcoord.xy;
	output.colorkey = colorkey;

	return output;
}