
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
	float4 texcoord : TEXCOORD0;	// xy, rw,rh
	float4 colorkey : TEXCOORD1;
	float4 horzcoord : TEXCOORD2;	// left - right
	float4 vertcoord : TEXCOORD3;	// top - down
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
	output.texcoord = float4(texcoord.xy, rawWidth, rawHeight);
	output.colorkey = colorkey;

	float pixw = 1.0f / rawWidth;
	float pixh = 1.0f / rawHeight;

	output.horzcoord = float4(texcoord.x - pixw, texcoord.y, texcoord.x + pixw, texcoord.y);
	output.vertcoord = float4(texcoord.x, texcoord.y - pixh, texcoord.x, texcoord.y + pixh);

	return output;
}