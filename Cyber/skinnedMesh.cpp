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
	//����shader�ļ�
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
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Lighting Effect Error", MB_OK);		//MB_OK��ɶ��
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
		MessageBox(NULL, (char*)pErrorMsgs->GetBufferPointer(), "Load Shadow Effect Error", MB_OK);		//MB_OK��ɶ��
		return E_FAIL;
	}

	//��������
	BoneHierarchyLoader boneHierarchyLoader;
	D3DXLoadMeshHierarchyFromX(fileName, D3DXMESH_MANAGED, pD3DDevice, &boneHierarchyLoader,
								NULL, (LPD3DXFRAME*)&m_pRootBone, NULL);


	UpdateMatrixOfBone2Model(m_pRootBone);

	SaveMatrixsOfBone2Model2Container(m_pRootBone);			//�Ĺ�

	D3DXCreateSphere(pD3DDevice, 0.02, 10, 20, &m_pSphereMesh, NULL);	//Ϊʲô���ܷŵ����캯���У�
}

//Ϊɶ����CreateMeshContainer��ʵ�������߼�������Ϊһ��Container�е�����Bone��һ�������������
//�о����Դ������������Frame��Container��ģ�Ϳؼ�֮��Ĺ�ϵ
void SkinnedMesh::SaveMatrixsOfBone2Model2Container(Bone *bone)
{
	if (bone->pMeshContainer != NULL)
	{
		BoneMesh *boneMesh = (BoneMesh*)bone->pMeshContainer;
		if (boneMesh->pSkinInfo != NULL)	//����ж�����Ƥ��Ϣ
		{
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			boneMesh->matrixsOfBone2Model = new D3DXMATRIX*[numBones];
			for (int  i = 0; i < numBones; i++)
			{
				Bone *b = (Bone*)D3DXFrameFind(m_pRootBone, boneMesh->pSkinInfo->GetBoneName(i));
				if (b != NULL)
					boneMesh->matrixsOfBone2Model[i] = &b->matrixOfbone2Model;
				else			//��������������ɣ�
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
	//ģ�ͻ���
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

	//��Ӱ����
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
		if (boneMesh->pSkinInfo != NULL)		//����ж�����Ƥ��Ϣ
		{
			//��������ɫ��
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			for (int i = 0; i < numBones; i++)
				D3DXMatrixMultiply(
					&boneMesh->matrixPalette[i],
					&boneMesh->matrixsOfModel2Bone[i],
					boneMesh->matrixsOfBone2Model[i]);

			//Ӧ�þ����ɫ�岢��Ƥ
			BYTE *src = NULL, *dest = NULL;
			boneMesh->originalMesh->LockVertexBuffer(D3DLOCK_READONLY, (VOID**)&src);
			boneMesh->originalMesh->UnlockVertexBuffer();
			boneMesh->MeshData.pMesh->LockVertexBuffer(0, (VOID**)&dest);
			boneMesh->MeshData.pMesh->UnlockVertexBuffer();
			boneMesh->pSkinInfo->UpdateSkinnedMesh(boneMesh->matrixPalette, NULL, src, dest);
			
			//��������������
			for (int i = 0; i < (int)boneMesh->numAttributeGroups; i++)
			{
				int attribId = boneMesh->attributeTable[i].AttribId;		//��AttribId��̫׼ȷ�ɣ�
				pD3DDevice->SetMaterial(&(boneMesh->materials[attribId]));
				pD3DDevice->SetTexture(0, boneMesh->textures[attribId]);	//��������һ�£�shader�е�����������Ϳ���ʹ����������ˣ�Ӧ����������Ŷ�Ӧ���ˣ�effect Begin֮��ֻ����pD3DDevice->SetTexture��������pLightingEffect->SetTexture��
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
	if (curBone->pMeshContainer != NULL)		//����˵��D3DXMESHCONTAINER�Ĺ���������Ҫ��Ⱦ�ģ�
	{
		BoneMesh *boneMesh = (BoneMesh*)curBone->pMeshContainer;
		if (boneMesh->pSkinInfo != NULL)		//����ж�����Ƥ��Ϣ
		{
			//��������ɫ��
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			for (int i = 0; i < numBones; i++)
				D3DXMatrixMultiply(
					&boneMesh->matrixPalette[i],
					&boneMesh->matrixsOfModel2Bone[i],
					boneMesh->matrixsOfBone2Model[i]);

			//���ú�Shader
			pLightingEffect->SetMatrixArray("MatrixPalette", boneMesh->matrixPalette, boneMesh->pSkinInfo->GetNumBones());
			D3DXHANDLE hTech = pLightingEffect->GetTechniqueByName("SkinningAndLighting");		//����shader��Ӧ�þ����ɫ�岢��Ƥ
			pLightingEffect->SetTechnique(hTech);

			//��������������
			for (int i = 0; i < (int)boneMesh->numAttributeGroups; i++)
			{
				int attribId = boneMesh->attributeTable[i].AttribId;		//��AttribId��̫׼ȷ�ɣ�
				pD3DDevice->SetMaterial(&(boneMesh->materials[attribId]));
				pLightingEffect->SetTexture("texDiffuse", boneMesh->textures[attribId]);		//effect Begin֮ǰֻ����pLightingEffect->SetTexture��������pD3DDevice->SetTexture��

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
			//���ú�Shader
			pLightingEffect->SetMatrix("matW", &curBone->matrixOfbone2Model);	//������ƤҲ���������ֱ�Ӹ��˻�������ɣ�
																				//�����������ֻ�Ǳ任��ģ�Ϳռ䣬û��ȥ������ռ�ɣ�Ӧ�ó���world���������ý�ȥ�ɣ�
																				//û����Ƥ��Ϣ������Ķ��㲻�Ǳ�����ģ�Ϳռ��е�

			D3DXHANDLE hTech = pLightingEffect->GetTechniqueByName("NormalLighting");
			pLightingEffect->SetTechnique(hTech);

			//��������������
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
	//��һ�����ʱ��parent��NULL
	if (parentBone != NULL)
	{
		if (curBone->Name && parentBone->Name)
		{
			//�ڵ�ǰ����λ�û���һ����
			pD3DDevice->SetRenderState(D3DRS_LIGHTING, true);
			pD3DDevice->SetTransform(D3DTS_WORLD, &(curBone->matrixOfbone2Model * (*world)));
			m_pSphereMesh->DrawSubset(0);

			//���ƹ����������
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