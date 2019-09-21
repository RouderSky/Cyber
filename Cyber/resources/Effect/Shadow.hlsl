matrix matShader;
matrix matVP;

float4 vs(float4 position : POSITION0) : POSITION0
{
	return mul(mul(position, matShader), matVP);
}

float4 ps(float4 position : POSITION0) : COLOR0
{
	return float4(0.3f, 0.3f, 0.3f, 1.0f);
}

technique Shadow
{
	pass P0
	{
		Lighting = false;

		VertexShader = compile vs_2_0 vs();
		PixelShader = compile ps_2_0 ps();
	}
}