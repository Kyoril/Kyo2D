
float4x4 matView : register(c0);

struct VOut
{
	float4 position : POSITION;
	float4 color : COLOR;
};

VOut main( float4 pos : POSITION, float4 color : COLOR )
{
	VOut output;
	output.position = mul(matView, float4(pos.x, pos.y, 0.0f, 1.0f));
	output.color = color;

	return output;
}