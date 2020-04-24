
cbuffer ConstantBuffer
{
	float4x4 matView;
}

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VOut main(float4 pos : POSITION, float4 color : COLOR)
{
	VOut output;
	output.position = mul(matView, float4(pos.x, pos.y, pos.z, 1.0f));
	output.color = color;

	return output;
}