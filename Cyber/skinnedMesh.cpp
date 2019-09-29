#include "skinnedMesh.h"
#include "BoneHierarchyLoader.h"

SkinnedMesh::SkinnedMesh()
{
	m_pRootBone = NULL;
	m_pSphereMesh = NULL;
}

SkinnedMesh::~SkinnedMesh()
{
	BoneHierarchyLoader boneHierarchy;
	boneHierarchy.DestroyFrame(m_pRootBone);
	if (pLightingEffect != NULL)
		pLightingEffect->Release();
	if (pShadowEffect != NULL)
		pShadowEffect->Release();
}

HRESULT SkinnedMesh::Load(const char fileName[], const char lightingEffectFileName[], const char shadowEffectFileName[])
{
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

	//加载网格
	BoneHierarchyLoader boneHierarchyLoader;
	D3DXLoadMeshHierarchyFromX(fileName, D3DXMESH_MANAGED, pD3DDevice, &boneHierarchyLoader,
								NULL, (LPD3DXFRAME*)&m_pRootBone, NULL);


	UpdateMatrixOfBone2Model(m_pRootBone);

	SaveMatrixsOfBone2Model2Container(m_pRootBone);			//改过

	D3DXCreateSphere(pD3DDevice, 0.02, 10, 20, &m_pSphereMesh, NULL);	//为什么不能放到构造函数中？
}

//为啥不在CreateMeshContainer中实现以下逻辑，是因为一个Container中的所有Bone不一定都创建好了嘛？
//感觉可以从这个函数看出Frame、Container、模型控件之间的关系
void SkinnedMesh::SaveMatrixsOfBone2Model2Container(Bone *bone)
{
	if (bone->pMeshContainer != NULL)
	{
		BoneMesh *boneMesh = (BoneMesh*)bone->pMeshContainer;
		if (boneMesh->pSkinInfo != NULL)	//如果有顶点蒙皮信息
		{
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			boneMesh->matrixsOfBone2Model = new D3DXMATRIX*[numBones];
			for (int  i = 0; i < numBones; i++)
			{
				Bone *b = (Bone*)D3DXFrameFind(m_pRootBone, boneMesh->pSkinInfo->GetBoneName(i));
				if (b != NULL)
					boneMesh->matrixsOfBone2Model[i] = &b->matrixOfbone2Model;
				else			//不会有这种情况吧？
					boneMesh->matrixsOfBone2Model[i] = NULL;
			}
		}
	}

	if (bone->pFrameSibling != NULL)
		SaveMatrixsOfBone2Model2Container((Bone*)bone->pFrameSibling);
	if (bone->pFrameFirstChild != NULL)
		SaveMatrixsOfBone2Model2Container((Bone*)bone->pFrameFirstChild);

}

void SkinnedMesh::UpdateMatrixOfBone2Model(Bone* bone, D3DXMATRIX *parentMatrix)
{
	if (bone == NULL)
		return;

	if (parentMatrix == NULL)
	{
		D3DXMATRIX i;
		D3DXMatrixIdentity(&i);
		parentMatrix = &i;
	}

	D3DXMatrixMultiply(
		&bone->matrixOfbone2Model,
		&bone->TransformationMatrix,
		parentMatrix);

	if (bone->pFrameSibling)
		UpdateMatrixOfBone2Model((Bone*)bone->pFrameSibling, parentMatrix);

	if(bone->pFrameFirstChild)
		UpdateMatrixOfBone2Model((Bone*)bone->pFrameFirstChild, &bone->matrixOfbone2Model);
}

struct VERTEX
{
	VERTEX();
	VERTEX(D3DXVECTOR3 position, D3DCOLOR color)
	{
		this->position = position;
		this->color = color;
	}

	D3DXVECTOR3 position;
	D3DCOLOR color;

	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

void SkinnedMesh::SoftRender(D3DXMATRIX *world, D3DXMATRIX *view, D3DXMATRIX *proj, D3DXVECTOR4 *lightPos, D3DXVECTOR4 *lightColor, D3DXMATRIX *shadow)
{
	//模型绘制
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
		RealSoftRender(m_pRootBone);
		pLightingEffect->EndPass();
	}
	pLightingEffect->End();

	//阴影绘制
	pShadowEffect->SetMatrix("matShadow", shadow);
	pShadowEffect->SetMatrix("matVP", &((*view) * (*proj)));
	hTech = pShadowEffect->GetTechniqueByName("Shadow");
	pShadowEffect->SetTechnique(hTech);
	pShadowEffect->Begin(&passCont, NULL);
	for (UINT i = 0; i < passCont; i++)
	{
		pShadowEffect->BeginPass(i);
		RealSoftRender(m_pRootBone);
		pShadowEffect->EndPass();
	}
	pShadowEffect->End();
}

void SkinnedMesh::RealSoftRender(Bone *curBone)
{
	if (curBone->pMeshContainer != NULL)
	{
		BoneMesh *boneMesh = (BoneMesh*)curBone->pMeshContainer;
		if (boneMesh->pSkinInfo != NULL)		//如果有顶点蒙皮信息
		{
			//计算矩阵调色板
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			for (int i = 0; i < numBones; i++)
				D3DXMatrixMultiply(
					&boneMesh->matrixPalette[i],
					&boneMesh->matrixsOfModel2Bone[i],
					boneMesh->matrixsOfBone2Model[i]);

			//应用矩阵调色板并蒙皮
			BYTE *src = NULL, *dest = NULL;
			boneMesh->originalMesh->LockVertexBuffer(D3DLOCK_READONLY, (VOID**)&src);
			boneMesh->originalMesh->UnlockVertexBuffer();
			boneMesh->MeshData.pMesh->LockVertexBuffer(0, (VOID**)&dest);
			boneMesh->MeshData.pMesh->UnlockVertexBuffer();
			boneMesh->pSkinInfo->UpdateSkinnedMesh(boneMesh->matrixPalette, NULL, src, dest);
			
			//绘制所有子网格
			for (int i = 0; i < (int)boneMesh->numAttributeGroups; i++)
			{
				int attribId = boneMesh->attributeTable[i].AttribId;		//用AttribId不太准确吧？
				pD3DDevice->SetMaterial(&(boneMesh->materials[attribId]));
				pD3DDevice->SetTexture(0, boneMesh->textures[attribId]);	//这样设置一下，shader中的纹理采样器就可以使用这个纹理了，应该是纹理序号对应上了？effect Begin之后只能用pD3DDevice->SetTexture，不可用pLightingEffect->SetTexture？
				boneMesh->MeshData.pMesh->DrawSubset(attribId);
			}
		}
	}

	if (curBone->pFrameSibling != NULL)
		RealSoftRender((Bone*)curBone->pFrameSibling);
	if (curBone->pFrameFirstChild != NULL)
		RealSoftRender((Bone*)curBone->pFrameFirstChild);
}

void SkinnedMesh::HardRender(D3DXMATRIX *world, D3DXMATRIX *view, D3DXMATRIX *proj, D3DXVECTOR4 *lightPos, D3DXVECTOR4 *lightColor, D3DXMATRIX *shadow)
{
	pLightingEffect->SetMatrix("matW", world);
	pLightingEffect->SetMatrix("matVP", &((*view) * (*proj)));
	pLightingEffect->SetVector("lightPos", lightPos);
	pLightingEffect->SetVector("lightColor", lightColor);
	RealHardRender(m_pRootBone);
}

void SkinnedMesh::RealHardRender(Bone *curBone)
{
	if (curBone->pMeshContainer != NULL)		//就是说有D3DXMESHCONTAINER的骨骼才是需要渲染的？
	{
		BoneMesh *boneMesh = (BoneMesh*)curBone->pMeshContainer;
		if (boneMesh->pSkinInfo != NULL)		//如果有顶点蒙皮信息
		{
			//计算矩阵调色板
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			for (int i = 0; i < numBones; i++)
				D3DXMatrixMultiply(
					&boneMesh->matrixPalette[i],
					&boneMesh->matrixsOfModel2Bone[i],
					boneMesh->matrixsOfBone2Model[i]);

			//设置好Shader
			pLightingEffect->SetMatrixArray("MatrixPalette", boneMesh->matrixPalette, boneMesh->pSkinInfo->GetNumBones());
			D3DXHANDLE hTech = pLightingEffect->GetTechniqueByName("SkinningAndLighting");		//交给shader来应用矩阵调色板并蒙皮
			pLightingEffect->SetTechnique(hTech);

			//绘制所有子网格
			for (int i = 0; i < (int)boneMesh->numAttributeGroups; i++)
			{
				int attribId = boneMesh->attributeTable[i].AttribId;		//用AttribId不太准确吧？
				pD3DDevice->SetMaterial(&(boneMesh->materials[attribId]));
				pLightingEffect->SetTexture("texDiffuse", boneMesh->textures[attribId]);		//effect Begin之前只能用pLightingEffect->SetTexture，不可用pD3DDevice->SetTexture？

				UINT passCount;
				pLightingEffect->Begin(&passCount, NULL);
				for (UINT i = 0; i < passCount; i++)
				{
					pLightingEffect->BeginPass(i);
					boneMesh->MeshData.pMesh->DrawSubset(attribId);
					pLightingEffect->EndPass();
				}
				pLightingEffect->End();
			}
		}
		else
		{
			//设置好Shader
			pLightingEffect->SetMatrix("matW", &curBone->matrixOfbone2Model);	//顶点蒙皮也用这个矩阵，直接改了会有问题吧？
																				//而且这个矩阵只是变换到模型空间，没有去到世界空间吧，应该乘上world矩阵再设置进去吧？
																				//没有蒙皮信息的网格的顶点不是保存在模型空间中的

			D3DXHANDLE hTech = pLightingEffect->GetTechniqueByName("NormalLighting");
			pLightingEffect->SetTechnique(hTech);

			//绘制所有子网格
			for (int i = 0; i < (int)boneMesh->materials.size(); i++)
			{
				pD3DDevice->SetMaterial(&boneMesh->materials[i]);
				pLightingEffect->SetTexture("texDiffuse", boneMesh->textures[i]);

				UINT passCount;
				pLightingEffect->Begin(&passCount, NULL);
				for (UINT i = 0; i < passCount; i++)
				{
					pLightingEffect->BeginPass(i);
					boneMesh->originalMesh->DrawSubset(i);
					pLightingEffect->EndPass();
				}
				pLightingEffect->End();
			}
		}
	}

	if (curBone->pFrameSibling != NULL)
		RealHardRender((Bone*)curBone->pFrameSibling);
	if (curBone->pFrameFirstChild != NULL)
		RealHardRender((Bone*)curBone->pFrameFirstChild);
}

void SkinnedMesh::RenderSkeleton(D3DXMATRIX *world, D3DXMATRIX *view, D3DXMATRIX *proj)
{
	pD3DDevice->SetTransform(D3DTS_VIEW, view);
	pD3DDevice->SetTransform(D3DTS_PROJECTION, proj);
	RealRenderSkeleton(world, m_pRootBone);
}

 void SkinnedMesh::RealRenderSkeleton(D3DXMATRIX *world, Bone* curBone, Bone* parentBone)
{
	//第一层调用时，parent是NULL
	if (parentBone != NULL)
	{
		if (curBone->Name && parentBone->Name)
		{
			//在当前骨骼位置绘制一个球
			pD3DDevice->SetRenderState(D3DRS_LIGHTING, true);
			pD3DDevice->SetTransform(D3DTS_WORLD, &(curBone->matrixOfbone2Model * (*world)));
			m_pSphereMesh->DrawSubset(0);

			//绘制骨骼间的连线
			D3DXMATRIX w1 = curBone->matrixOfbone2Model;
			D3DXMATRIX w2 = parentBone->matrixOfbone2Model;

			D3DXVECTOR3 curBonePos = D3DXVECTOR3(w1(3, 0), w1(3, 1), w1(3, 2));
			D3DXVECTOR3 parentBonePos = D3DXVECTOR3(w2(3, 0), w2(3, 1), w2(3, 2));

			if (D3DXVec3Length(&(curBonePos - parentBonePos)) < 2.0f)
			{
				VERTEX vert[] = { VERTEX(parentBonePos, 0xffff0000), VERTEX(curBonePos, 0xff00ff00) };
				pD3DDevice->SetTransform(D3DTS_WORLD, world);
				pD3DDevice->SetRenderState(D3DRS_LIGHTING, false);
				pD3DDevice->SetFVF(VERTEX::FVF);
				pD3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, vert, sizeof(VERTEX));
			}
		}
	}

	if (curBone->pFrameSibling)
		RealRenderSkeleton(world, (Bone*)curBone->pFrameSibling, parentBone);
	if (curBone->pFrameFirstChild)
		RealRenderSkeleton(world, (Bone*)curBone->pFrameFirstChild, curBone);
}

 void SkinnedMesh::OnLostDevice()
 {
	 pLightingEffect->OnLostDevice();
	 pShadowEffect->OnLostDevice();
 }

 void SkinnedMesh::OnResetDevice()
 {
	 pLightingEffect->OnResetDevice();
	 pShadowEffect->OnResetDevice();
 }