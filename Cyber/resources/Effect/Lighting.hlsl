matrix matW;
matrix matVP;

float4 lightColor;
float3 lightPos;
texture texDiffuse;		

sampler DiffuseSampler = sampler_state
{
	Texture = (texDiffuse);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap; AddressW = Wrap;
	MaxAnisotropy = 16;
};

struct VS_INPUT
{
	float4 position : POSITION0;
	float3 normal : NORMAL;
	float2 tex0 : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : POSITION0;
	float2 tex0 : TEXCOORD0;
	float4 shade : COLOR0;		//原本使用的TEXCOORD0是不对的吧？
};

VS_OUTPUT vs(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4 posWorld = mul(IN.position, matW);
	float4 normal = normalize(mul(IN.normal, matW));	//这个计算成立的前提是，matW没有非统一缩放等破坏空间比例的元素

	OUT.position = mul(posWorld, matVP);
	OUT.tex0 = IN.tex0;
	OUT.shade = max(dot(normal, normalize(lightPos - posWorld)), 0.2f) * lightColor;

	return OUT;
}

float4 ps(VS_OUTPUT IN) : COLOR0
{
	float4 diffuseColor = tex2D(DiffuseSampler, IN.tex0);
	return IN.shade * diffuseColor;
}

technique NormalLighting
{
	pass P0
	{
		Lighting = false;
		VertexShader = compile vs_2_0 vs();
		PixelShader = compile ps_2_0 ps();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

extern float4x4 MatrixPaletts[35];		//这里必须用extern？ConvertToIndexedBlendedMesh哪里设置的是30，这两应该要保持同步吧？
extern int numBoneInfluences = 2;

struct VS_INPUT_SKIN
{
	float4 position : POSITION0;
};

technique skinningandlighting
{
	pass p0
	{

	};
};