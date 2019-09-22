#ifndef BONE_HIERARCHY_LOADER
#define BONE_HIERARCHY_LOADER

#include <d3dx9.h>

//�㼶������
class BoneHierarchyLoader :public ID3DXAllocateHierarchy
{
public:
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);		//ʲôʱ��Ҫ��THIS_��һ��Ҫ��STDMETHOD��
	STDMETHOD(CreateMeshContainer) (
		THIS_ LPCTSTR Name,
		CONST D3DXMESHDATA *pMeshData,
		CONST D3DXMATERIAL *pMaterials,
		CONST D3DXEFFECTINSTANCE *pEffectInstances,
		DWORD NumMaterials,
		CONST DWORD *pAdjacency,
		LPD3DXSKININFO pSkinInfo,
		LPD3DXMESHCONTAINER *ppNewMeshContainer);		//Ϊʲô��CONST������const��
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
};

#endif
