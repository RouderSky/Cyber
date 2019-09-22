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

	D3DXCreateSphere(pD3DDevice, 0.02, 10, 20, &m_pSphereMesh, NULL);	//为什么不能放到构造函数中？
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