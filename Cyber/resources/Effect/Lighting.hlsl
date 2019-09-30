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

VS_OUTPUT vs_NormalLighting(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4 posWorld = mul(IN.position, matW);
	float4 normal = normalize(mul(IN.normal, matW));	//这个计算成立的前提是，matW没有非统一缩放等破坏空间比例的元素

	OUT.position = mul(posWorld, matVP);
	OUT.tex0 = IN.tex0;
	OUT.shade = max(dot(normal, normalize(lightPos - posWorld)), 0.2f) * lightColor;

	return OUT;
}

float4 ps_lighting(VS_OUTPUT IN) : COLOR0
{
	float4 diffuseColor = tex2D(DiffuseSampler, IN.tex0);
	return IN.shade * diffuseColor;
}

technique NormalLighting
{
	pass P0
	{
		Lighting = false;
		VertexShader = compile vs_2_0 vs_NormalLighting();
		PixelShader = compile ps_2_0 ps_lighting();
	}
}

///////////////////////////////////////////////////////////////////////////////////////

extern float4x4 MatrixPalette[35];		//这里必须用extern？ConvertToIndexedBlendedMesh哪里设置的是30，这两应该要保持同步吧？
extern int numBoneInfluences = 2;

struct VS_INPUT_SKIN
{
	float4 position : POSITION0;
	float3 normal : NORMAL;
	float2 tex0 : TEXCOORD0;
	float4 weights : BLENDWEIGHT0;
	int4 boneIndices : BLENDINDICES0;
};

VS_OUTPUT vs_SkinningAndLighting(VS_INPUT_SKIN IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4 worldPos = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float3 normal = float3(0.0f, 0.0f, 0.0f);

	////////////////////////////////////////////////////////////////
	//对单个顶点进行蒙皮
	IN.normal = normalize(IN.normal);
	int lastIndex = numBoneInfluences - 1;
	float hadUsedWeight = 0.0f;
	for (int i = 0; i < lastIndex; ++i)	//略去了最后一个
	{
		float weight = IN.weights[i];
		int index = IN.boneIndices[i];
		worldPos += weight * mul(IN.position, MatrixPalette[index]);
		normal += weight * mul(IN.normal, MatrixPalette[index]);		//处理法线真粗暴
		hadUsedWeight += weight;
	}

	//可以这么做的前提是每个顶点的蒙皮权重之和为1，只要法线已经单位化，blend后的法线也是单位化的
	//我觉得不是很准确，应该直接blend所有的矩阵，然后再将法线单位化
	float leftWeight = 1.0 - hadUsedWeight;
	worldPos += leftWeight * mul(IN.position, MatrixPalette[IN.boneIndices[lastIndex]]);
	worldPos.w = 1.0f;
	normal += leftWeight * mul(IN.normal, MatrixPalette[IN.boneIndices[lastIndex]]);
	////////////////////////////////////////////////////////////////

	//顶点空间变换
	OUT.position = mul(worldPos, matVP);

	//计算漫反射
	normal = normalize(normal);
	OUT.shade = max(dot(normal, normalize(lightPos - worldPos)), 0.2f) * lightColor;

	OUT.tex0 = IN.tex0;
	return OUT;
}

technique SkinningAndLighting
{
	pass p0
	{
		Lighting = false;
		VertexShader = compile vs_2_0 vs_SkinningAndLighting();
		PixelShader = compile ps_2_0 ps_lighting();
	}
}