#include "mesh.h"

Mesh::Mesh()
{
	m_pMesh = NULL;
}

Mesh::Mesh(const char fName[], const char lightingEffectFileName[], const char shadowEffectFileName[])
{
	m_pMesh = NULL;
	Load(fName, lightingEffectFileName, shadowEffectFileName);
}

Mesh::~Mesh()
{
	Release();
}

HRESULT Mesh::Load(const char fName[], const char lightingEffectFileName[], const char shadowEffectFileName[])
{
	Release();

	//加载shader文件
	ID3DXBuffer *pErrorMsgs = NULL;
	char *effectFileName = global::CombineStr(ROOT_PATH_TO_EFFECT, lightingEffectFileName);
	HRESULT hRes = D3DXCreateEffectFromFile(
		pD3DDevice,
		effectFileName,
		NULL,
		NULL,
		D3DXSHADER_DEBUG,
		NULL,
		&pLightingEffect,
		&pErrorMsgs);
	delete[]effectFileName;
	if (FAILED(hRes) && (pErrorMsgs != NULL))
	{
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Lighting Effect Error", MB_OK);		//MB_OK是啥？
		return E_FAIL;
	}

	effectFileName = global::CombineStr(ROOT_PATH_TO_EFFECT, shadowEffectFileName);
	hRes = D3DXCreateEffectFromFile(
		pD3DDevice,
		effectFileName,
		NULL,
		NULL,
		D3DXSHADER_DEBUG,
		NULL,
		&pShadowEffect,
		&pErrorMsgs);
	delete[]effectFileName;
	if (FAILED(hRes) && (pErrorMsgs != NULL))
	{
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Shadow Effect Error", MB_OK);		//MB_OK是啥？
		return E_FAIL;
	}

	//加载
	ID3DXBuffer * adjacencyBfr = NULL;
	ID3DXBuffer * materialBfr = NULL;
	DWORD noMaterials = NULL;
	if (FAILED(D3DXLoadMeshFromX(fName, D3DXMESH_MANAGED, pD3DDevice, &adjacencyBfr, &materialBfr, NULL, &noMaterials, &m_pMesh)))
	{
		MessageBox(NULL, "Error", "Load Mesh Error", MB_OK);
		return E_FAIL;
	}
	
	//存储数据
	D3DXMATERIAL *mtrls = (D3DXMATERIAL*)materialBfr->GetBufferPointer();
	for (int i = 0; i < (int)noMaterials; ++i)
	{
		//材质
		m_materials.push_back(mtrls[i].MatD3D);
		//纹理
		if (mtrls[i].pTextureFilename != NULL)
		{
			char *textureFileName = global::CombineStr(ROOT_PATH_TO_MESH_TEXTURES, mtrls[i].pTextureFilename);
			IDirect3DTexture9 *newTexture = NULL;
			D3DXCreateTextureFromFile(pD3DDevice, textureFileName, &newTexture);
			m_textures.push_back(newTexture);
		}
		else
			m_textures.push_back(NULL);
	}

	//优化网格
	m_pMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT | D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjacencyBfr->GetBufferPointer(), NULL, NULL, NULL);

	//释放无用资源
	adjacencyBfr->Release();
	materialBfr->Release();

	return S_OK;
}

void Mesh::Render(D3DXMATRIX *world, D3DXMATRIX *view, D3DXMATRIX *proj, D3DXVECTOR4 *lightPos, D3DXVECTOR4 *lightColor, D3DXMATRIX *shadow)
{
	pLightingEffect->SetMatrix("matW", world);
	pLightingEffect->SetMatrix("matVP", &((*view) * (*proj)));
	pLightingEffect->SetVector("lightPos", lightPos);
	pLightingEffect->SetVector("lightColor", lightColor);

	D3DXHANDLE hTech = pLightingEffect->GetTechniqueByName("NormalLighting");
	pLightingEffect->SetTechnique(hTech);
	UINT passCont;
	pLightingEffect->Begin(&passCont, NULL);
	for (UINT i = 0; i < passCont; i++)
	{
		pLightingEffect->BeginPass(i);
		DrawMesh();
		pLightingEffect->EndPass();
	}
	pLightingEffect->End();

	pShadowEffect->SetMatrix("matShadow", shadow);
	pShadowEffect->SetMatrix("matVP", &((*view) * (*proj)));
	hTech = pShadowEffect->GetTechniqueByName("Shadow");
	pShadowEffect->SetTechnique(hTech);
	pShadowEffect->Begin(&passCont, NULL);
	for (UINT i = 0; i < passCont; i++)
	{
		pShadowEffect->BeginPass(i);
		DrawMesh();
		pShadowEffect->EndPass();
	}
	pShadowEffect->End();
}

void Mesh::DrawMesh()
{
	int numMaterials = (int)m_materials.size();

	for (int i = 0; i < numMaterials; ++i)
	{
		if (m_textures[i] != NULL)
			pD3DDevice->SetMaterial(&global::mtrlOfWhite);
		else
			pD3DDevice->SetMaterial(&m_materials[i]);		//没有纹理时，直接限时材质颜色就好

		pD3DDevice->SetTexture(0, m_textures[i]);		//竟然可以是空的？

		m_pMesh->DrawSubset(i);
	}
}

void Mesh::Release()
{
	if (m_pMesh != NULL)
	{
		m_pMesh->Release();
		m_pMesh = NULL;
	}

	int numTextures = (int)m_textures.size();
	for (int i=0;i<numTextures;++i)
	{
		if (m_textures[i] != NULL)
			m_textures[i]->Release();
	}

	m_textures.clear();
	m_materials.clear();

	if (pLightingEffect != NULL)
		pLightingEffect->Release();
	if (pShadowEffect != NULL)
		pShadowEffect->Release();
}