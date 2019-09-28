#ifndef BONE_MESH
#define BONE_MESH

#include <d3dx9.h>
#include <vector>

using namespace std;

struct BoneMesh : public D3DXMESHCONTAINER
{
	//�̳�������MeshData��Ϊ�洢��Ƥ���������
	ID3DXMesh *originalMesh;				//ԭ��������

	//������������Ҳ��һһ��Ӧ��
	vector<D3DMATERIAL9> materials;
	vector<IDirect3DTexture9*> textures;	//��materialsһһ��Ӧ
	DWORD numAttributeGroups;				//�������ж��ٸ�������
	D3DXATTRIBUTERANGE* attributeTable;		//���Ա�

	//�������������Ԫ����һһ��Ӧ��
	D3DXMATRIX **matrixsOfBone2Model;		//Ԫ�������õ�matrixOfbone2Model��ָ��
	D3DXMATRIX *matrixsOfModel2Bone;		//����������󣬹̶����䣻��Ҳ������Ǵ�ģ�Ϳռ�䵽�����ռ仹�Ǵ�container�ռ�䵽�����ռ䣬�������Ա䵽�����ռ�		boneOffsetMatrices
	D3DXMATRIX *matrixPalette;				//��Ƥ������Ⱦ֮ǰ����һ��	currentBoneMatrices
};

#endif