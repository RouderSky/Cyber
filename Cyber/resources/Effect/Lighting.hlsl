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
	float4 shade : COLOR0;		//ԭ��ʹ�õ�TEXCOORD0�ǲ��Եİɣ�
};

VS_OUTPUT vs_NormalLighting(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4 posWorld = mul(IN.position, matW);
	float4 normal = normalize(mul(IN.normal, matW));	//������������ǰ���ǣ�matWû�з�ͳһ���ŵ��ƻ��ռ������Ԫ��

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

extern float4x4 MatrixPalette[35];		//���������extern��ConvertToIndexedBlendedMesh�������õ���30������Ӧ��Ҫ����ͬ���ɣ�
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
	//�Ե������������Ƥ
	IN.normal = normalize(IN.normal);
	int lastIndex = numBoneInfluences - 1;
	float hadUsedWeight = 0.0f;
	for (int i = 0; i < lastIndex; ++i)	//��ȥ�����һ��
	{
		float weight = IN.weights[i];
		int index = IN.boneIndices[i];
		worldPos += weight * mul(IN.position, MatrixPalette[index]);
		normal += weight * mul(IN.normal, MatrixPalette[index]);		//��������ֱ�
		hadUsedWeight += weight;
	}

	//������ô����ǰ����ÿ���������ƤȨ��֮��Ϊ1��ֻҪ�����Ѿ���λ����blend��ķ���Ҳ�ǵ�λ����
	//�Ҿ��ò��Ǻ�׼ȷ��Ӧ��ֱ��blend���еľ���Ȼ���ٽ����ߵ�λ��
	float leftWeight = 1.0 - hadUsedWeight;
	worldPos += leftWeight * mul(IN.position, MatrixPalette[IN.boneIndices[lastIndex]]);
	worldPos.w = 1.0f;
	normal += leftWeight * mul(IN.normal, MatrixPalette[IN.boneIndices[lastIndex]]);
	////////////////////////////////////////////////////////////////

	//����ռ�任
	OUT.position = mul(worldPos, matVP);

	//����������
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