
sampler Texture;

float4 main(float4 position : POSITION, 
			float4 color : COLOR, 
			float4 texcoord : TEXCOORD0, 
			float4 colorkey : TEXCOORD1,
			float4 horzcoord : TEXCOORD2,
			float4 vertcoord : TEXCOORD3
			) : COLOR
{
	float4 Center = tex2D(Texture, texcoord.xy);
	float4 Left =	tex2D(Texture, horzcoord.xy);
	float4 Right =	tex2D(Texture, horzcoord.zw);
	float4 Up =		tex2D(Texture, vertcoord.xy);
	float4 Bottom = tex2D(Texture, vertcoord.zw);

	float2 PixelSize = frac(texcoord.xy * texcoord.zw);
	if (PixelSize.y >= 0.5f)
	{
		float4 swap = Up;
		Up = Bottom;
		Bottom = swap;
	}

	if (PixelSize.x >= 0.5f)
	{
		float4 swap = Left;
		Left = Right;
		Right = swap;
	}

	bool Match1 = (Up.r == Left.r && Up.g == Left.g && Up.b == Left.b && Up.a == Left.a);
	bool Match2 = (Up.r != Right.r || Up.g != Right.g || Up.b != Right.g || Up.a != Right.a);
	bool Match3 = (Left.r != Bottom.r || Left.g != Bottom.g || Left.b != Bottom.b || Left.a != Bottom.a);
	if (Match1 && Match2 && Match3)
	{
		Center = Left;
	}

	// Color-keying
	clip(
		((Center.a == colorkey.a) &&
		 (Center.r == colorkey.r) && 
		 (Center.g == colorkey.g) && 
		 (Center.b == colorkey.b)) ? -1 : 1);

	return color * Center;
}