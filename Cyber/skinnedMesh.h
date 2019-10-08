#ifndef SKINNED_MESH
#define SKINNED_MESH

#include "global.h"
#include <d3dx9.h>
#include "Bone.h"
#include "BoneMesh.h"

class SkinnedMesh
{
public:
	SkinnedMesh();
	~SkinnedMesh();
	HRESULT Load(const char fileName[], const char lightingEffectFileName[], const char shadowEffectFileName[]);
	void SoftRender(D3DXMATRIX *view, D3DXMATRIX *proj, D3DXVECTOR4 *lightPos, D3DXVECTOR4 *lightColor, D3DXMATRIX *shadow);
	void RealSoftRender(Bone *curBone);
	void HardRender(D3DXMATRIX *view, D3DXMATRIX *proj, D3DXVECTOR4 *lightPos, D3DXVECTOR4 *lightColor, D3DXMATRIX *shadow);
	void RealHardRender(Bone *curBone);
	void RenderSkeleton(D3DXMATRIX *view, D3DXMATRIX *proj);
	void RealRenderSkeleton(Bone* curBone, Bone* parent = NULL);

	void GetAllAnimSetName(vector<string> &animSetNames);
	void ActiveAnimSet(string name);
	//¸üÐÂPose
	void UpdateMatrixOfBone2Model();
	ID3DXAnimationController* GetControllerCopy();

	void OnLostDevice();
	void OnResetDevice();

	D3DXMATRIX world;
private:
	void SaveMatrixsOfBone2Model2Container(Bone *bone);
	void UpdateMatrixOfBone2Model(Bone* bone, D3DXMATRIX* parentMatrix);

	Bone *m_pRootBone;
	ID3DXAnimationController *m_pAnimControl;
	LPD3DXMESH m_pSphereMesh;
	ID3DXEffect *pLightingEffect;
	ID3DXEffect *pShadowEffect;
};

#endif
