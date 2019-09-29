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
	BoneMesh *newBoneMesh = new BoneMesh;
	memset(newBoneMesh, 0, sizeof(BoneMesh));

	//����ԭ������Ϣ
	pMeshData->pMesh->AddRef();
	newBoneMesh->originalMesh = pMeshData->pMesh;
	newBoneMesh->MeshData.Type = pMeshData->Type;

	for (int i = 0; i < (int)NumMaterials; i++)
	{
		//�������
		D3DXMATERIAL mtrl;
		memcpy(&mtrl, &pMaterials[i], sizeof(D3DXMATERIAL));
		newBoneMesh->materials.push_back(mtrl.MatD3D);

		//��������
		IDirect3DTexture9 *newTexture = NULL;
		if (mtrl.pTextureFilename != NULL)
		{
			char *textureFileName = global::CombineStr(ROOT_PATH_TO_MESH_TEXTURES, mtrl.pTextureFilename);
			D3DXCreateTextureFromFile(pD3DDevice, textureFileName, &newTexture);
			delete[]textureFileName;
		}
		newBoneMesh->textures.push_back(newTexture);
	}

	if (pSkinInfo != NULL)
	{
		//������Ƥ��Ϣ
		newBoneMesh->pSkinInfo = pSkinInfo;
		pSkinInfo->AddRef();			//ͨ��һ��ָ���õ�������D3D����ͨ������ָ���õ������Լ�����������D3D����ʱ��Ҫ����ʱ���ҪAddRef��

#if SOFTWARE_SKINNED
		//����һ����������
		pMeshData->pMesh->CloneMeshFVF(D3DXMESH_MANAGED, pMeshData->pMesh->GetFVF(),
			pD3DDevice, &newBoneMesh->MeshData.pMesh);
#endif
#if HARDWARE_SKINNED
		DWORD maxVertInfluences = 0;
		DWORD numBoneComboEntries = 0;
		ID3DXBuffer* boneComboTable = 0;
		//����Ƥ��Ϣ���浽����
		pSkinInfo->ConvertToIndexedBlendedMesh(
			pMeshData->pMesh,
			D3DXMESH_MANAGED | D3DXMESH_WRITEONLY,
			35,
			NULL,
			NULL,
			NULL,
			NULL,
			&maxVertInfluences,
			&numBoneComboEntries,
			&boneComboTable,
			&newBoneMesh->MeshData.pMesh);			//��Щ������ɶ��˼��boneComboTable����Ƥ����ɣ�
		if (boneComboTable != NULL)
			boneComboTable->Release();
#endif
		//�������Ա�
		newBoneMesh->MeshData.pMesh->GetAttributeTable(NULL, &newBoneMesh->numAttributeGroups);
		newBoneMesh->attributeTable = new D3DXATTRIBUTERANGE[newBoneMesh->numAttributeGroups];
		newBoneMesh->MeshData.pMesh->GetAttributeTable(newBoneMesh->attributeTable, NULL);

		//��������������
		int numBones = pSkinInfo->GetNumBones();
		newBoneMesh->matrixsOfModel2Bone = new D3DXMATRIX[numBones];
		for (int i = 0; i < numBones; i++)
			newBoneMesh->matrixsOfModel2Bone[i] = *(newBoneMesh->pSkinInfo->GetBoneOffsetMatrix(i));

		//��ʼ����Ƥ����
		newBoneMesh->matrixPalette = new D3DXMATRIX[numBones];
	}
	else
	{
		//newBoneMesh->MeshData.pMesh = pMeshData->pMesh;		//û�õİɣ�
	}

	*ppNewMeshContainer = newBoneMesh;

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

	//�ͷ�����
	int numTextures = (int)boneMesh->textures.size();
	for (int i = 0; i < numTextures; i++)
		if (boneMesh->textures[i] != NULL)
			boneMesh->textures[i]->Release();

	//�ͷ��������Ƥ��Ϣ	�Ĺ�
	if (boneMesh->originalMesh)
		boneMesh->originalMesh->Release();
	if (boneMesh->MeshData.pMesh)
		boneMesh->MeshData.pMesh->Release();
	if (boneMesh->pSkinInfo)
		boneMesh->pSkinInfo->Release();

	return S_OK;
}