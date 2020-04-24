
sampler state;

float4 main(float4 position : POSITION, 
			float4 color : COLOR0, 
			float2 texcoord : TEXCOORD0,
			float4 colorkey : TEXCOORD1
			) : COLOR0
{
	float4 Center = tex2D(state, texcoord.xy);

	// Color-keying
	clip(
		((Center.a == colorkey.a) &&
		 (Center.r == colorkey.r) && 
		 (Center.g == colorkey.g) && 
		 (Center.b == colorkey.b)) ? -1 : 1);

	return color * Center;
}