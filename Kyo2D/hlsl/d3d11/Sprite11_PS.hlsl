
Texture2D Texture;
SamplerState ss;

float4 main(float4 position : SV_POSITION, 
			float4 color : COLOR, 
			float2 texcoord : TEXCOORD0, 
			float4 colorkey : TEXCOORD1
			) : SV_TARGET
{
	float4 Center = Texture.Sample(ss, texcoord.xy);

	// Color-keying
	clip(
		((Center.a == colorkey.a) &&
		 (Center.r == colorkey.r) && 
		 (Center.g == colorkey.g) && 
		 (Center.b == colorkey.b)) ? -1 : 1);

	return color * Center;
}