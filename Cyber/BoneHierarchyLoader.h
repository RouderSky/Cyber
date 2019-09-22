#ifndef BONE_HIERARCHY_LOADER
#define BONE_HIERARCHY_LOADER

#include <d3dx9.h>

//层级加载器
class BoneHierarchyLoader :public ID3DXAllocateHierarchy
{
public:
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);		//什么时候要加THIS_？一定要用STDMETHOD？
	STDMETHOD(CreateMeshContainer) (
		THIS_ LPCTSTR Name,
		CONST D3DXMESHDATA *pMeshData,
		CONST D3DXMATERIAL *pMaterials,
		CONST D3DXEFFECTINSTANCE *pEffectInstances,
		DWORD NumMaterials,
		CONST DWORD *pAdjacency,
		LPD3DXSKININFO pSkinInfo,
		LPD3DXMESHCONTAINER *ppNewMeshContainer);		//为什么用CONST而不用const？
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
};

#endif
