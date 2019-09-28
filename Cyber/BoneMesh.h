#ifndef BONE_MESH
#define BONE_MESH

#include <d3dx9.h>
#include <vector>

using namespace std;

struct BoneMesh : public D3DXMESHCONTAINER
{
	//继承下来的MeshData作为存储蒙皮结果的容器
	ID3DXMesh *originalMesh;				//原网格数据

	//以下三个数据也是一一对应的
	vector<D3DMATERIAL9> materials;
	vector<IDirect3DTexture9*> textures;	//与materials一一对应
	DWORD numAttributeGroups;				//代表着有多少个子网格
	D3DXATTRIBUTERANGE* attributeTable;		//属性表

	//以下三个矩阵的元素是一一对应的
	D3DXMATRIX **matrixsOfBone2Model;		//元素是引用到matrixOfbone2Model的指针
	D3DXMATRIX *matrixsOfModel2Bone;		//绑定姿势逆矩阵，固定不变；我也不清楚是从模型空间变到骨骼空间还是从container空间变到骨骼空间，反正可以变到骨骼空间		boneOffsetMatrices
	D3DXMATRIX *matrixPalette;				//蒙皮矩阵，渲染之前更新一下	currentBoneMatrices
};

#endif