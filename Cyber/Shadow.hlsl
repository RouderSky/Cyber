matrix matShader;
matrix matVP;

struct VS_INPUT
{
	float4 position : POSITION;
};

float4 vs(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	return mul(mul(IN.position, matShader), matVP);
}

float4 ps(VS_OUTPUT IN) : COLOR0
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
	};
};