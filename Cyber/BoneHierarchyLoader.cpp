#include "BoneHierarchyLoader.h"
#include "Bone.h"
#include "BoneMesh.h"

HRESULT BoneHierarchyLoader::CreateFrame(LPCSTR Name, LPD3DXFRAME *ppNewFrame)
{
	Bone *newBone = new Bone;
	memset(newBone, 0, sizeof(Bone));

	if (Name != NULL)
	{
		newBone->Name = new char[strlen(Name) + 1];
		strcpy(newBone->Name, Name);
	}

	//D3DXMatrixIdentity(&newBone->TransformationMatrix);
	D3DXMatrixIdentity(&newBone->matrixOfbone2Model);

	*ppNewFrame = (D3DXFRAME*)newBone;

	return S_OK;
}

HRESULT BoneHierarchyLoader::CreateMeshContainer(
	LPCTSTR Name,
	CONST D3DXMESHDATA *pMeshData,
	CONST D3DXMATERIAL *pMaterials,
	CONST D3DXEFFECTINSTANCE *pEffectInstances,
	DWORD NumMaterials,
	CONST DWORD *pAdjacency,
	LPD3DXSKININFO pSkinInfo,
	LPD3DXMESHCONTAINER *ppNewMeshContainer)
{
	*ppNewMeshContainer = new BoneMesh;
	memset(*ppNewMeshContainer, 0, sizeof(BoneMesh));
	return S_OK;
}

HRESULT BoneHierarchyLoader::DestroyFrame(LPD3DXFRAME pFrameToFree)
{
	if (pFrameToFree)
	{
		if (pFrameToFree->Name != NULL)
			delete[]pFrameToFree->Name;

		delete pFrameToFree;
	}

	return S_OK;
}

HRESULT BoneHierarchyLoader::DestroyMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase)
{
	if (pMeshContainerBase != NULL)
		delete pMeshContainerBase;

	return S_OK;
}