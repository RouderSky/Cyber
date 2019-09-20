#include "mesh.h"

#pragma warning(disable:4996)

Mesh::Mesh()
{
	m_pMesh = NULL;
}

Mesh::Mesh(const char fName[])
{
	m_pMesh = NULL;
	Load(fName);
}

Mesh::~Mesh()
{
	Release();
}

HRESULT Mesh::Load(const char fName[])
{
	Release();

	//加载
	ID3DXBuffer * adjacencyBfr = NULL;
	ID3DXBuffer * materialBfr = NULL;
	DWORD noMaterials = NULL;
	if (FAILED(D3DXLoadMeshFromX(fName, D3DXMESH_MANAGED, global::pDevice, &adjacencyBfr, &materialBfr, NULL, &noMaterials, &m_pMesh)))
		return E_FAIL;
	
	//存储数据
	D3DXMATERIAL *mtrls = (D3DXMATERIAL*)materialBfr->GetBufferPointer();
	for (int i = 0; i < (int)noMaterials; ++i)
	{
		//材质
		m_materials.push_back(mtrls[i].MatD3D);
		//纹理
		if (mtrls[i].pTextureFilename != NULL)
		{
			char textureFileName[90];
			strcpy(textureFileName, ROOT_PATH_TO_TEXTURES);
			strcat(textureFileName, mtrls[i].pTextureFilename);
			IDirect3DTexture9 *newTexture = NULL;
			D3DXCreateTextureFromFile(global::pDevice, textureFileName, &newTexture);
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

void Mesh::Render()
{
	int numMaterials = (int)m_materials.size();

	for (int i=0;i<numMaterials;++i)
	{
		if (m_textures[i] != NULL)
			global::pDevice->SetMaterial(&global::mtrlOfWhite);
		else
			global::pDevice->SetMaterial(&m_materials[i]);		//没有纹理时，直接限时材质颜色就好

		global::pDevice->SetTexture(0, m_textures[i]);		//竟然可以是空的？
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
	m_materials.clear;
}