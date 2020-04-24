
float4x4 matView : register(c0);
float4x4 matWorld;

struct VOut
{
	float4 position : POSITION;
	float4 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float4 colorkey : TEXCOORD1;
};

VOut main(
	float4 pos : POSITION, 
	float4 color : COLOR0, 
	float2 texcoord : TEXCOORD0, 
	float4 colorkey : TEXCOORD1)
{
	VOut output;
	
	float4x4 mvp = mul(matView, matWorld);
	output.position = mul(mvp, float4(pos.x, pos.y, 0.0f, 1.0f));

	output.color = color;
	output.texcoord = texcoord.xy;
	output.colorkey = colorkey;

	return output;
}