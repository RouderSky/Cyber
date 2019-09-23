#include "BoneHierarchyLoader.h"
#include "Bone.h"
#include "BoneMesh.h"
#include "global.h"

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
	BoneMesh *boneMesh = new BoneMesh;
	memset(boneMesh, 0, sizeof(BoneMesh));

	pMeshData->pMesh->AddRef();
	boneMesh->originalMesh = pMeshData->pMesh;
	boneMesh->MeshData.Type = pMeshData->Type;

	for (int i = 0; i < (int)NumMaterials; i++)
	{
		//保存材质
		D3DXMATERIAL mtrl;
		memcpy(&mtrl, &pMaterials, sizeof(D3DXMATERIAL));
		boneMesh->materials.push_back(mtrl.MatD3D);

		//保存纹理
		IDirect3DTexture9 *newTexture = NULL;
		if (mtrl.pTextureFilename != NULL)
		{
			char *textureFileName = global::CombineStr(ROOT_PATH_TO_MESH_TEXTURES, mtrl.pTextureFilename);
			D3DXCreateTextureFromFile(pD3DDevice, textureFileName, &newTexture);
			delete[]textureFileName;
		}
		boneMesh->textures.push_back(newTexture);
	}

	if (pSkinInfo != NULL)
	{
		//保存蒙皮信息
		boneMesh->pSkinInfo = pSkinInfo;
		pSkinInfo->AddRef();			//通过一级指针获得的内容由D3D管理，通过二级指针获得的内容自己管理？内容由D3D管理时想要长期时候就要AddRef？

		//复制一份网格数据
		pMeshData->pMesh->CloneMeshFVF(D3DXMESH_MANAGED, pMeshData->pMesh->GetFVF(),
			pD3DDevice, &boneMesh->MeshData.pMesh);

		//保存属性表  改过
		pMeshData->pMesh->GetAttributeTable(NULL, &boneMesh->numAttributeGroups);
		boneMesh->attributeTable = new D3DXATTRIBUTERANGE[boneMesh->numAttributeGroups];
		pMeshData->pMesh->GetAttributeTable(boneMesh->attributeTable, NULL);

		//保存绑定姿势逆矩阵
		int numBones = pSkinInfo->GetNumBones();
		boneMesh->matrixsOfModel2Bone = new D3DXMATRIX[numBones];
		for (int i = 0; i < numBones; i++)
			boneMesh->matrixsOfModel2Bone[i] = *(boneMesh->pSkinInfo->GetBoneOffsetMatrix(i));

		//初始化蒙皮矩阵
		boneMesh->matrixPalette = new D3DXMATRIX[numBones];
	}
	else
	{
		boneMesh->MeshData.pMesh = pMeshData->pMesh;
	}

	*ppNewMeshContainer = boneMesh;

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
	BoneMesh *boneMesh = (BoneMesh*)pMeshContainerBase;

	//释放纹理
	int numTextures = (int)boneMesh->textures.size();
	for (int i = 0; i < numTextures; i++)
		if (boneMesh->textures[i] != NULL)
			boneMesh->textures[i]->Release();

	//释放网格和蒙皮信息	改过
	if (boneMesh->originalMesh)
		boneMesh->originalMesh->Release();
	if (boneMesh->MeshData.pMesh)
		boneMesh->MeshData.pMesh->Release();
	if (boneMesh->pSkinInfo)
		boneMesh->pSkinInfo->Release();

	return S_OK;
}