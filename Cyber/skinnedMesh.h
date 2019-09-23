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
	void Load(const char fileName[]);
	void Render(Bone *curBone = NULL);
	void RenderSkeleton(D3DXMATRIX world, Bone* curBone = NULL, Bone* parent = NULL);

private:
	void SaveMatrixsOfBone2Model2Container(Bone *bone);
	void UpdateMatrixOfBone2Model(Bone* bone, D3DXMATRIX* parentMatrix);

	Bone *m_pRootBone;
	LPD3DXMESH m_pSphereMesh;
};

#endif
