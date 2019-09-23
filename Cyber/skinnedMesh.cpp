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
}

void SkinnedMesh::Load(const char fileName[])
{
	BoneHierarchyLoader boneHierarchyLoader;
	D3DXLoadMeshHierarchyFromX(fileName, D3DXMESH_MANAGED, pD3DDevice, &boneHierarchyLoader,
								NULL, (LPD3DXFRAME*)&m_pRootBone, NULL);

	//将根骨骼设置到模型空间原点
	D3DXMATRIX i;
	D3DXMatrixIdentity(&i);
	UpdateMatrixOfBone2Model(m_pRootBone, &i);

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

void SkinnedMesh::Render(Bone *curBone)
{
	if (curBone == NULL)		//第一层调用会
		curBone = m_pRootBone;

	if (curBone->pMeshContainer != NULL)
	{
		BoneMesh *boneMesh = (BoneMesh*)curBone->pMeshContainer;
		if (boneMesh->pSkinInfo != NULL)		//如果有顶点蒙皮信息
		{
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			//计算矩阵调色板
			for (int i = 0; i < numBones; i++)
				D3DXMatrixMultiply(
					&boneMesh->matrixPalette[i],
					&boneMesh->matrixsOfModel2Bone[i],
					boneMesh->matrixsOfBone2Model[i]);

			//蒙皮
			BYTE *src = NULL, *dest = NULL;
			boneMesh->originalMesh->LockVertexBuffer(D3DLOCK_READONLY, (VOID**)&src);
			boneMesh->originalMesh->UnlockVertexBuffer();
			boneMesh->MeshData.pMesh->LockVertexBuffer(0, (VOID**)&dest);
			boneMesh->MeshData.pMesh->UnlockVertexBuffer();
			boneMesh->pSkinInfo->UpdateSkinnedMesh(boneMesh->matrixPalette, NULL, src, dest);
			
			//绘制网格
			for (int i = 0; i < (int)boneMesh->numAttributeGroups; i++)
			{
				int attribId = boneMesh->attributeTable[i].AttribId;
				pD3DDevice->SetMaterial(&(boneMesh->materials[attribId]));
				pD3DDevice->SetTexture(0, boneMesh->textures[attribId]);
				boneMesh->MeshData.pMesh->DrawSubset(attribId);
			}
		}
	}

	if (curBone->pFrameSibling != NULL)
		Render((Bone*)curBone->pFrameSibling);
	if (curBone->pFrameFirstChild != NULL)
		Render((Bone*)curBone->pFrameFirstChild);
}

 void SkinnedMesh::RenderSkeleton(D3DXMATRIX world, Bone* curBone, Bone* parentBone)
{
	//第一次调用时，curBone和parent都是NULL
	if (curBone == NULL)
		curBone = m_pRootBone;

	if (parentBone != NULL)
	{
		if (curBone->Name && parentBone->Name)
		{
			//在当前骨骼位置绘制一个球
			pD3DDevice->SetRenderState(D3DRS_LIGHTING, true);
			pD3DDevice->SetTransform(D3DTS_WORLD, &(curBone->matrixOfbone2Model * world));
			m_pSphereMesh->DrawSubset(0);

			//绘制骨骼间的连线
			D3DXMATRIX w1 = curBone->matrixOfbone2Model;
			D3DXMATRIX w2 = parentBone->matrixOfbone2Model;

			D3DXVECTOR3 curBonePos = D3DXVECTOR3(w1(3, 0), w1(3, 1), w1(3, 2));
			D3DXVECTOR3 parentBonePos = D3DXVECTOR3(w2(3, 0), w2(3, 1), w2(3, 2));

			if (D3DXVec3Length(&(curBonePos - parentBonePos)) < 2.0f)
			{
				VERTEX vert[] = { VERTEX(parentBonePos, 0xffff0000), VERTEX(curBonePos, 0xff00ff00) };
				pD3DDevice->SetTransform(D3DTS_WORLD, &world);
				pD3DDevice->SetRenderState(D3DRS_LIGHTING, false);
				pD3DDevice->SetFVF(VERTEX::FVF);
				pD3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, vert, sizeof(VERTEX));
			}
		}
	}

	if (curBone->pFrameSibling)
		RenderSkeleton(world, (Bone*)curBone->pFrameSibling, parentBone);
	if (curBone->pFrameFirstChild)
		RenderSkeleton(world, (Bone*)curBone->pFrameFirstChild, curBone);
}